/*
 * WebInterface.cpp
 *
 * Implementation of the Web-based interface for SECoT
 */

#include "WebInterface.h"

WebInterface::WebInterface() {
  // Initialize variables
  serverStarted = false;
  lastUpdate = 0;

  // Default AP settings
  apMode = true;
  apSSID = AP_SSID;
  apPassword = AP_PASSWORD;
  apIP = IPAddress(192, 168, 4, 1);
}

void WebInterface::begin(AttackManager* attackManager, NetworkScanner* networkScanner) {
  this->attackManager = attackManager;
  this->networkScanner = networkScanner;

  // Initialize SPIFFS for serving web files
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Set up access point
  if (apMode) {
    setupAccessPoint();
  }

  // Set up web server
  setupWebServer();

  Serial.println("Web interface initialized");
}

void WebInterface::update() {
  unsigned long currentMillis = millis();

  // Update WebSocket clients every 1 second
  if (currentMillis - lastUpdate >= 1000) {
    notifyClients();
    lastUpdate = currentMillis;
  }
}

void WebInterface::setupAccessPoint() {
  // Set up WiFi access point
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  bool result = WiFi.softAP(apSSID.c_str(), apPassword.c_str());

  if (result) {
    Serial.print("Access Point started with SSID: ");
    Serial.println(apSSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start Access Point");
  }
}

void WebInterface::setupWebServer() {
  // Create web server
  server = new AsyncWebServer(WEB_SERVER_PORT);

  // Create WebSocket
  ws = new AsyncWebSocket("/ws");
  ws->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                    AwsEventType type, void* arg, uint8_t* data, size_t len) {
    this->handleWebSocketEvent(server, client, type, arg, data, len);
  });

  // Add WebSocket to server
  server->addHandler(ws);

  // API routes
  server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    this->handleAPIStatus(request);
  });

  server->on("/api/attacks", HTTP_GET, [this](AsyncWebServerRequest* request) {
    this->handleAPIAttacks(request);
  });

  server->on("/api/attack/start", HTTP_POST, [this](AsyncWebServerRequest* request) {
    this->handleAPIAttackStart(request);
  });

  server->on("/api/attack/stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
    this->handleAPIAttackStop(request);
  });

  server->on("/api/attack/params", HTTP_GET, [this](AsyncWebServerRequest* request) {
    this->handleAPIAttackParams(request);
  });

  server->on("/api/attack/setparam", HTTP_POST, [this](AsyncWebServerRequest* request) {
    this->handleAPIAttackSetParam(request);
  });

  server->on("/api/scan", HTTP_POST, [this](AsyncWebServerRequest* request) {
    this->handleAPIScan(request);
  });

  server->on("/api/scan/results", HTTP_GET, [this](AsyncWebServerRequest* request) {
    this->handleAPIScanResults(request);
  });

  // Serve static files from SPIFFS
  server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Handle not found
  server->onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  });

  // Start server
  server->begin();
  serverStarted = true;

  Serial.println("Web server started");
}

void WebInterface::handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      parseWebSocketMessage(data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void WebInterface::notifyClients() {
  if (ws->count() > 0) {
    // Send status update to all clients
    String status = getStatusJSON();
    ws->textAll(status);
  }
}

void WebInterface::parseWebSocketMessage(uint8_t* data, size_t len) {
  // Parse JSON message
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, data, len);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Process command
  String command = doc["command"];

  if (command == "start_attack") {
    uint8_t attackType = doc["type"];
    unsigned long duration = doc["duration"] | 0;

    Attack* attack = attackManager->getAttack(attackType);
    if (attack) {
      attack->start(duration);
    }
  }
  else if (command == "stop_attack") {
    uint8_t attackType = doc["type"];

    Attack* attack = attackManager->getAttack(attackType);
    if (attack) {
      attack->stop();
    }
  }
  else if (command == "set_param") {
    uint8_t attackType = doc["type"];
    String paramName = doc["name"];
    String paramValue = doc["value"];

    Attack* attack = attackManager->getAttack(attackType);
    if (attack) {
      attack->setParameter(paramName, paramValue);
    }
  }
  else if (command == "scan") {
    uint8_t scanType = doc["type"];

    if (scanType == NETWORK_TYPE_WIFI) {
      networkScanner->scanWiFiNetworks();
    }
    else if (scanType == NETWORK_TYPE_MQTT) {
      String prefix = doc["prefix"] | "";
      networkScanner->scanMQTTBrokers(prefix);
    }
    #ifdef ENABLE_BLUETOOTH
    else if (scanType == NETWORK_TYPE_BLE) {
      networkScanner->scanBLEDevices();
    }
    #endif
  }
}

void WebInterface::handleAPIStatus(AsyncWebServerRequest* request) {
  String status = getStatusJSON();
  request->send(200, "application/json", status);
}

void WebInterface::handleAPIAttacks(AsyncWebServerRequest* request) {
  String attacks = getAttacksJSON();
  request->send(200, "application/json", attacks);
}

void WebInterface::handleAPIAttackStart(AsyncWebServerRequest* request) {
  if (!request->hasParam("type")) {
    request->send(400, "text/plain", "Missing attack type");
    return;
  }

  uint8_t attackType = request->getParam("type")->value().toInt();
  unsigned long duration = 0;

  if (request->hasParam("duration")) {
    duration = request->getParam("duration")->value().toInt();
  }

  Attack* attack = attackManager->getAttack(attackType);
  if (!attack) {
    request->send(404, "text/plain", "Attack not found");
    return;
  }

  bool success = attack->start(duration);

  if (success) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json",
                 "{\"success\":false,\"error\":\"" + attack->getErrorMessage() + "\"}");
  }
}

void WebInterface::handleAPIAttackStop(AsyncWebServerRequest* request) {
  if (!request->hasParam("type")) {
    request->send(400, "text/plain", "Missing attack type");
    return;
  }

  uint8_t attackType = request->getParam("type")->value().toInt();

  Attack* attack = attackManager->getAttack(attackType);
  if (!attack) {
    request->send(404, "text/plain", "Attack not found");
    return;
  }

  bool success = attack->stop();

  if (success) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json",
                 "{\"success\":false,\"error\":\"" + attack->getErrorMessage() + "\"}");
  }
}

void WebInterface::handleAPIAttackParams(AsyncWebServerRequest* request) {
  if (!request->hasParam("type")) {
    request->send(400, "text/plain", "Missing attack type");
    return;
  }

  uint8_t attackType = request->getParam("type")->value().toInt();

  Attack* attack = attackManager->getAttack(attackType);
  if (!attack) {
    request->send(404, "text/plain", "Attack not found");
    return;
  }

  String params = getAttackParamsJSON(attackType);
  request->send(200, "application/json", params);
}

void WebInterface::handleAPIAttackSetParam(AsyncWebServerRequest* request) {
  if (!request->hasParam("type") || !request->hasParam("name") || !request->hasParam("value")) {
    request->send(400, "text/plain", "Missing required parameters");
    return;
  }

  uint8_t attackType = request->getParam("type")->value().toInt();
  String paramName = request->getParam("name")->value();
  String paramValue = request->getParam("value")->value();

  Attack* attack = attackManager->getAttack(attackType);
  if (!attack) {
    request->send(404, "text/plain", "Attack not found");
    return;
  }

  bool success = attack->setParameter(paramName, paramValue);

  if (success) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json",
                 "{\"success\":false,\"error\":\"" + attack->getErrorMessage() + "\"}");
  }
}

void WebInterface::handleAPIScan(AsyncWebServerRequest* request) {
  if (!request->hasParam("type")) {
    request->send(400, "text/plain", "Missing scan type");
    return;
  }

  uint8_t scanType = request->getParam("type")->value().toInt();
  bool success = false;

  if (scanType == NETWORK_TYPE_WIFI) {
    success = networkScanner->scanWiFiNetworks();
  }
  else if (scanType == NETWORK_TYPE_MQTT) {
    String prefix = "";
    if (request->hasParam("prefix")) {
      prefix = request->getParam("prefix")->value();
    }
    success = networkScanner->scanMQTTBrokers(prefix);
  }
  #ifdef ENABLE_BLUETOOTH
  else if (scanType == NETWORK_TYPE_BLE) {
    success = networkScanner->scanBLEDevices();
  }
  #endif

  if (success) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json", "{\"success\":false,\"error\":\"Scan failed\"}");
  }
}

void WebInterface::handleAPIScanResults(AsyncWebServerRequest* request) {
  if (!request->hasParam("type")) {
    request->send(400, "text/plain", "Missing scan type");
    return;
  }

  uint8_t scanType = request->getParam("type")->value().toInt();
  String results = getScanResultsJSON(scanType);

  request->send(200, "application/json", results);
}

String WebInterface::getStatusJSON() {
  DynamicJsonDocument doc(2048);

  // System status
  doc["system"]["uptime"] = millis() / 1000;
  doc["system"]["free_heap"] = ESP.getFreeHeap();

  // WiFi status
  doc["wifi"]["ap_mode"] = apMode;
  doc["wifi"]["ap_ssid"] = apSSID;
  doc["wifi"]["ap_ip"] = WiFi.softAPIP().toString();
  doc["wifi"]["sta_connected"] = (WiFi.status() == WL_CONNECTED);
  if (WiFi.status() == WL_CONNECTED) {
    doc["wifi"]["sta_ssid"] = WiFi.SSID();
    doc["wifi"]["sta_ip"] = WiFi.localIP().toString();
    doc["wifi"]["sta_rssi"] = WiFi.RSSI();
  }

  // Attack status
  JsonArray attacks = doc.createNestedArray("attacks");

  for (int i = 0; i < attackManager->getAttackCount(); i++) {
    Attack* attack = attackManager->getAttackByIndex(i);
    if (attack) {
      JsonObject attackObj = attacks.createNestedObject();
      attackObj["type"] = attack->getType();
      attackObj["name"] = attack->getName();
      attackObj["status"] = attack->getStatus();
      attackObj["status_text"] = attack->getStatusString();

      if (attack->getStatus() == ATTACK_STATUS_ERROR) {
        attackObj["error"] = attack->getErrorMessage();
      }
    }
  }

  // Scan status
  doc["scan"]["wifi_networks"] = networkScanner->getWiFiNetworks().size();
  doc["scan"]["mqtt_brokers"] = networkScanner->getMQTTBrokers().size();
  #ifdef ENABLE_BLUETOOTH
  doc["scan"]["ble_devices"] = networkScanner->getBLEDevices().size();
  #endif
  doc["scan"]["scanning"] = networkScanner->isScanning();

  String output;
  serializeJson(doc, output);
  return output;
}

String WebInterface::getAttacksJSON() {
  DynamicJsonDocument doc(1024);
  JsonArray attacks = doc.createNestedArray("attacks");

  for (int i = 0; i < attackManager->getAttackCount(); i++) {
    Attack* attack = attackManager->getAttackByIndex(i);
    if (attack) {
      JsonObject attackObj = attacks.createNestedObject();
      attackObj["type"] = attack->getType();
      attackObj["name"] = attack->getName();
      attackObj["status"] = attack->getStatus();
      attackObj["status_text"] = attack->getStatusString();
    }
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String WebInterface::getAttackParamsJSON(uint8_t attackType) {
  DynamicJsonDocument doc(1024);

  Attack* attack = attackManager->getAttack(attackType);
  if (attack) {
    doc["type"] = attack->getType();
    doc["name"] = attack->getName();

    // Parse the attack's JSON parameters
    String paramsStr = attack->getAllParameters();
    DynamicJsonDocument paramsDoc(1024);
    deserializeJson(paramsDoc, paramsStr);

    // Copy parameters to the response
    doc["params"] = paramsDoc;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String WebInterface::getScanResultsJSON(uint8_t scanType) {
  DynamicJsonDocument doc(4096);

  if (scanType == NETWORK_TYPE_WIFI) {
    JsonArray networks = doc.createNestedArray("wifi_networks");

    std::vector<WiFiNetwork>& wifiNetworks = networkScanner->getWiFiNetworks();
    for (const WiFiNetwork& network : wifiNetworks) {
      JsonObject netObj = networks.createNestedObject();
      netObj["ssid"] = network.ssid;

      char bssidStr[18];
      snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
               network.bssid[0], network.bssid[1], network.bssid[2],
               network.bssid[3], network.bssid[4], network.bssid[5]);
      netObj["bssid"] = bssidStr;

      netObj["rssi"] = network.rssi;
      netObj["channel"] = network.channel;
      netObj["encryption"] = network.encryptionType;
      netObj["hidden"] = network.isHidden;
    }
  }
  else if (scanType == NETWORK_TYPE_MQTT) {
    JsonArray brokers = doc.createNestedArray("mqtt_brokers");

    std::vector<MQTTBroker>& mqttBrokers = networkScanner->getMQTTBrokers();
    for (const MQTTBroker& broker : mqttBrokers) {
      JsonObject brokerObj = brokers.createNestedObject();
      brokerObj["ip"] = broker.ip;
      brokerObj["port"] = broker.port;
      brokerObj["requires_auth"] = broker.requiresAuth;
      brokerObj["supports_tls"] = broker.supportsTLS;
      brokerObj["accessible"] = broker.isAccessible;
    }
  }
  #ifdef ENABLE_BLUETOOTH
  else if (scanType == NETWORK_TYPE_BLE) {
    JsonArray devices = doc.createNestedArray("ble_devices");

    std::vector<BLEDevice>& bleDevices = networkScanner->getBLEDevices();
    for (const BLEDevice& device : bleDevices) {
      JsonObject deviceObj = devices.createNestedObject();
      deviceObj["address"] = device.address;
      deviceObj["name"] = device.name;
      deviceObj["rssi"] = device.rssi;
      deviceObj["connectable"] = device.isConnectable;
    }
  }
  #endif

  String output;
  serializeJson(doc, output);
  return output;
}
