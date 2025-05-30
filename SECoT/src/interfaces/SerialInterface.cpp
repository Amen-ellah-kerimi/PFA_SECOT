/*
 * SerialInterface.cpp
 *
 * Implementation of the SerialInterface class
 * Supports both interactive mode and CLI tool communication
 */

#include "SerialInterface.h"

SerialInterface::SerialInterface() {
  bufferIndex = 0;
  memset(commandBuffer, 0, SERIAL_COMMAND_BUFFER);
  interfaceMode = SERIAL_MODE_INTERACTIVE;
  cliToolConnected = false;
  lastCliToolPing = 0;
}

void SerialInterface::begin(AttackManager* attackManager, NetworkScanner* networkScanner) {
  this->attackManager = attackManager;
  this->networkScanner = networkScanner;

  // Print initial prompt
  printPrompt();
}

bool SerialInterface::isConnectedToCLITool() {
  // Check if we've received a ping in the last 10 seconds
  if (cliToolConnected && millis() - lastCliToolPing > 10000) {
    cliToolConnected = false;
  }

  return cliToolConnected;
}

void SerialInterface::setMode(uint8_t mode) {
  interfaceMode = mode;

  if (mode == SERIAL_MODE_INTERACTIVE) {
    printPrompt();
  }
}

void SerialInterface::update() {
  // Check for incoming serial data
  while (Serial.available() > 0) {
    char c = Serial.read();

    // Handle backspace (only in interactive mode)
    if (interfaceMode == SERIAL_MODE_INTERACTIVE && (c == 8 || c == 127)) {
      if (bufferIndex > 0) {
        bufferIndex--;
        commandBuffer[bufferIndex] = '\0';
        Serial.print("\b \b"); // Erase character from terminal
      }
      continue;
    }

    // Echo character (only in interactive mode)
    if (interfaceMode == SERIAL_MODE_INTERACTIVE) {
      Serial.print(c);
    }

    // Handle newline
    if (c == '\n' || c == '\r') {
      if (interfaceMode == SERIAL_MODE_INTERACTIVE) {
        Serial.println();
      }

      // Process command if buffer is not empty
      if (bufferIndex > 0) {
        commandBuffer[bufferIndex] = '\0';
        String command = String(commandBuffer);

        // Process command based on interface mode
        if (interfaceMode == SERIAL_MODE_CLI_TOOL) {
          handleCliToolCommand(command);
        } else {
          processCommand(command);
        }
      }

      // Reset buffer
      bufferIndex = 0;
      memset(commandBuffer, 0, SERIAL_COMMAND_BUFFER);

      // Print prompt (only in interactive mode)
      if (interfaceMode == SERIAL_MODE_INTERACTIVE) {
        printPrompt();
      }

      continue;
    }

    // Add character to buffer if there's space
    if (bufferIndex < SERIAL_COMMAND_BUFFER - 1) {
      commandBuffer[bufferIndex++] = c;
    }
  }
}

void SerialInterface::processCommand(const String& command) {
  // Parse command and arguments
  String args[SERIAL_MAX_ARGS];
  int argCount = parseArgs(command, args, SERIAL_MAX_ARGS);

  if (argCount == 0) {
    return;
  }

  String cmd = args[0].toLowerCase();

  // Handle commands
  if (cmd == "help" || cmd == "?") {
    handleHelp();
  }
  else if (cmd == "scan") {
    handleScan(args, argCount);
  }
  else if (cmd == "attack") {
    handleAttack(args, argCount);
  }
  else if (cmd == "stop") {
    handleStop(args, argCount);
  }
  else if (cmd == "status") {
    handleStatus();
  }
  else if (cmd == "set") {
    handleSet(args, argCount);
  }
  else if (cmd == "get") {
    handleGet(args, argCount);
  }
  else if (cmd == "clear") {
    handleClear();
  }
  else {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
}

int SerialInterface::parseArgs(const String& command, String args[], int maxArgs) {
  int argCount = 0;
  int lastPos = 0;
  bool inQuotes = false;

  for (int i = 0; i < command.length(); i++) {
    char c = command.charAt(i);

    // Handle quotes
    if (c == '"') {
      inQuotes = !inQuotes;
      continue;
    }

    // Handle spaces (argument separator)
    if (c == ' ' && !inQuotes) {
      if (i > lastPos) {
        if (argCount < maxArgs) {
          args[argCount++] = command.substring(lastPos, i);
        }
      }
      lastPos = i + 1;
    }
  }

  // Add the last argument
  if (lastPos < command.length()) {
    if (argCount < maxArgs) {
      args[argCount++] = command.substring(lastPos);
    }
  }

  return argCount;
}

void SerialInterface::handleHelp() {
  Serial.println("SECoT - Security Evaluation & Compromise Toolkit");
  Serial.println("Available commands:");
  Serial.println("  help                - Show this help message");
  Serial.println("  scan [type]         - Scan for networks (wifi, mqtt, ble)");
  Serial.println("  attack <type> [dur] - Start an attack (optional duration in ms)");
  Serial.println("  stop [type]         - Stop an attack (or all if no type specified)");
  Serial.println("  status              - Show status of all attacks");
  Serial.println("  set <attack> <param> <value> - Set attack parameter");
  Serial.println("  get <attack> <param> - Get attack parameter");
  Serial.println("  clear               - Clear the screen");
  Serial.println();
  Serial.println("Available attack types:");
  printAttackList();
}

void SerialInterface::handleScan(const String args[], int argCount) {
  if (argCount < 2) {
    printScanOptions();
    return;
  }

  String scanType = args[1].toLowerCase();

  if (scanType == "wifi") {
    Serial.println("Starting WiFi scan...");
    networkScanner->scanWiFiNetworks();
  }
  else if (scanType == "mqtt") {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Error: Not connected to WiFi. Connect first to scan for MQTT brokers.");
      return;
    }

    String prefix = "";
    if (argCount >= 3) {
      prefix = args[2];
    }

    Serial.println("Starting MQTT broker scan...");
    networkScanner->scanMQTTBrokers(prefix);
  }
  #ifdef ENABLE_BLUETOOTH
  else if (scanType == "ble") {
    Serial.println("Starting BLE scan...");
    networkScanner->scanBLEDevices();
  }
  #endif
  else {
    Serial.println("Unknown scan type. Available types:");
    printScanOptions();
  }
}

void SerialInterface::handleAttack(const String args[], int argCount) {
  if (argCount < 2) {
    Serial.println("Usage: attack <type> [duration]");
    Serial.println("Available attack types:");
    printAttackList();
    return;
  }

  String attackType = args[1].toLowerCase();
  unsigned long duration = DEFAULT_ATTACK_DURATION;

  // Parse duration if provided
  if (argCount >= 3) {
    duration = args[2].toInt();
  }

  // Start the attack
  uint8_t type = 0;

  if (attackType == "deauth") {
    type = ATTACK_TYPE_DEAUTH;
  }
  else if (attackType == "beacon") {
    type = ATTACK_TYPE_BEACON_FLOOD;
  }
  else if (attackType == "probe") {
    type = ATTACK_TYPE_PROBE_SPAM;
  }
  else if (attackType == "arp") {
    type = ATTACK_TYPE_ARP_SPOOF;
  }
  else if (attackType == "mqtt") {
    type = ATTACK_TYPE_MQTT_SPOOF;
  }
  else if (attackType == "eviltwin") {
    type = ATTACK_TYPE_EVIL_TWIN;
  }
  else if (attackType == "sniff") {
    type = ATTACK_TYPE_PASSIVE_SNIFF;
  }
  #ifdef ENABLE_BLUETOOTH
  else if (attackType == "bluetooth") {
    type = ATTACK_TYPE_BLUETOOTH_SCAN;
  }
  #endif
  #ifdef ENABLE_JAMMING
  else if (attackType == "jam") {
    type = ATTACK_TYPE_JAMMING;
  }
  #endif
  else {
    Serial.println("Unknown attack type. Available types:");
    printAttackList();
    return;
  }

  if (attackManager->startAttack(type, duration)) {
    Serial.print("Started ");
    Serial.print(attackType);
    Serial.print(" attack");
    if (duration > 0) {
      Serial.print(" (duration: ");
      Serial.print(duration);
      Serial.print(" ms)");
    }
    Serial.println();
  } else {
    Attack* attack = attackManager->getAttack(type);
    if (attack) {
      Serial.print("Failed to start attack: ");
      Serial.println(attack->getErrorMessage());
    } else {
      Serial.println("Failed to start attack: Unknown error");
    }
  }
}

void SerialInterface::handleStop(const String args[], int argCount) {
  if (argCount < 2) {
    // Stop all attacks
    attackManager->stopAllAttacks();
    Serial.println("Stopped all attacks");
    return;
  }

  String attackType = args[1].toLowerCase();
  uint8_t type = 0;

  if (attackType == "deauth") {
    type = ATTACK_TYPE_DEAUTH;
  }
  else if (attackType == "beacon") {
    type = ATTACK_TYPE_BEACON_FLOOD;
  }
  else if (attackType == "probe") {
    type = ATTACK_TYPE_PROBE_SPAM;
  }
  else if (attackType == "arp") {
    type = ATTACK_TYPE_ARP_SPOOF;
  }
  else if (attackType == "mqtt") {
    type = ATTACK_TYPE_MQTT_SPOOF;
  }
  else if (attackType == "eviltwin") {
    type = ATTACK_TYPE_EVIL_TWIN;
  }
  else if (attackType == "sniff") {
    type = ATTACK_TYPE_PASSIVE_SNIFF;
  }
  #ifdef ENABLE_BLUETOOTH
  else if (attackType == "bluetooth") {
    type = ATTACK_TYPE_BLUETOOTH_SCAN;
  }
  #endif
  #ifdef ENABLE_JAMMING
  else if (attackType == "jam") {
    type = ATTACK_TYPE_JAMMING;
  }
  #endif
  else {
    Serial.println("Unknown attack type. Available types:");
    printAttackList();
    return;
  }

  if (attackManager->stopAttack(type)) {
    Serial.print("Stopped ");
    Serial.print(attackType);
    Serial.println(" attack");
  } else {
    Serial.print("Failed to stop ");
    Serial.print(attackType);
    Serial.println(" attack");
  }
}

void SerialInterface::handleStatus() {
  Serial.println("Attack Status:");
  Serial.println("-------------");

  int count = attackManager->getAttackCount();
  for (int i = 0; i < count; i++) {
    Attack* attack = attackManager->getAttackByIndex(i);
    if (attack) {
      Serial.print(attack->getName());
      Serial.print(": ");
      Serial.println(attack->getStatusString());
    }
  }

  Serial.println();

  Serial.println("Network Scanner Status:");
  Serial.println("-----------------------");
  Serial.print("WiFi Networks: ");
  Serial.println(networkScanner->getWiFiNetworks().size());
  Serial.print("MQTT Brokers: ");
  Serial.println(networkScanner->getMQTTBrokers().size());
  #ifdef ENABLE_BLUETOOTH
  Serial.print("BLE Devices: ");
  Serial.println(networkScanner->getBLEDevices().size());
  #endif
}

void SerialInterface::handleSet(const String args[], int argCount) {
  if (argCount < 4) {
    Serial.println("Usage: set <attack> <parameter> <value>");
    return;
  }

  String attackType = args[1].toLowerCase();
  String parameter = args[2];
  String value = args[3];

  uint8_t type = 0;

  if (attackType == "deauth") {
    type = ATTACK_TYPE_DEAUTH;
  }
  else if (attackType == "beacon") {
    type = ATTACK_TYPE_BEACON_FLOOD;
  }
  else if (attackType == "probe") {
    type = ATTACK_TYPE_PROBE_SPAM;
  }
  else if (attackType == "arp") {
    type = ATTACK_TYPE_ARP_SPOOF;
  }
  else if (attackType == "mqtt") {
    type = ATTACK_TYPE_MQTT_SPOOF;
  }
  else if (attackType == "eviltwin") {
    type = ATTACK_TYPE_EVIL_TWIN;
  }
  else if (attackType == "sniff") {
    type = ATTACK_TYPE_PASSIVE_SNIFF;
  }
  #ifdef ENABLE_BLUETOOTH
  else if (attackType == "bluetooth") {
    type = ATTACK_TYPE_BLUETOOTH_SCAN;
  }
  #endif
  #ifdef ENABLE_JAMMING
  else if (attackType == "jam") {
    type = ATTACK_TYPE_JAMMING;
  }
  #endif
  else {
    Serial.println("Unknown attack type. Available types:");
    printAttackList();
    return;
  }

  Attack* attack = attackManager->getAttack(type);
  if (attack) {
    if (attack->setParameter(parameter, value)) {
      Serial.print("Set ");
      Serial.print(parameter);
      Serial.print(" = ");
      Serial.print(value);
      Serial.print(" for ");
      Serial.print(attack->getName());
      Serial.println(" attack");
    } else {
      Serial.print("Failed to set parameter: ");
      Serial.println(attack->getErrorMessage());
    }
  } else {
    Serial.println("Attack not found");
  }
}

void SerialInterface::handleGet(const String args[], int argCount) {
  if (argCount < 3) {
    Serial.println("Usage: get <attack> <parameter>");
    return;
  }

  String attackType = args[1].toLowerCase();
  String parameter = args[2];

  uint8_t type = 0;

  if (attackType == "deauth") {
    type = ATTACK_TYPE_DEAUTH;
  }
  else if (attackType == "beacon") {
    type = ATTACK_TYPE_BEACON_FLOOD;
  }
  else if (attackType == "probe") {
    type = ATTACK_TYPE_PROBE_SPAM;
  }
  else if (attackType == "arp") {
    type = ATTACK_TYPE_ARP_SPOOF;
  }
  else if (attackType == "mqtt") {
    type = ATTACK_TYPE_MQTT_SPOOF;
  }
  else if (attackType == "eviltwin") {
    type = ATTACK_TYPE_EVIL_TWIN;
  }
  else if (attackType == "sniff") {
    type = ATTACK_TYPE_PASSIVE_SNIFF;
  }
  #ifdef ENABLE_BLUETOOTH
  else if (attackType == "bluetooth") {
    type = ATTACK_TYPE_BLUETOOTH_SCAN;
  }
  #endif
  #ifdef ENABLE_JAMMING
  else if (attackType == "jam") {
    type = ATTACK_TYPE_JAMMING;
  }
  #endif
  else {
    Serial.println("Unknown attack type. Available types:");
    printAttackList();
    return;
  }

  Attack* attack = attackManager->getAttack(type);
  if (attack) {
    String value = attack->getParameter(parameter);
    if (value.length() > 0) {
      Serial.print(parameter);
      Serial.print(" = ");
      Serial.println(value);
    } else {
      Serial.print("Parameter not found: ");
      Serial.println(parameter);
    }
  } else {
    Serial.println("Attack not found");
  }
}

void SerialInterface::handleClear() {
  // Clear terminal screen (ANSI escape code)
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");
}

void SerialInterface::handlePing() {
  // Respond to ping with device identifier
  Serial.println("SECoT Device v1.0");

  // Set CLI tool connection status
  cliToolConnected = true;
  lastCliToolPing = millis();

  // Switch to CLI tool mode
  setMode(SERIAL_MODE_CLI_TOOL);
}

void SerialInterface::handleCliToolCommand(const String& command) {
  // Update ping time for any command
  lastCliToolPing = millis();

  // Check if it's a ping command
  if (command.startsWith("ping")) {
    handlePing();
    return;
  }

  // Parse command and arguments
  String args[SERIAL_MAX_ARGS];
  int argCount = parseArgs(command, args, SERIAL_MAX_ARGS);

  if (argCount == 0) {
    return;
  }

  String cmd = args[0].toLowerCase();

  // Create JSON document for response
  StaticJsonDocument<2048> doc;

  // Handle commands
  if (cmd == "scan") {
    if (argCount < 2) {
      doc["status"] = "error";
      doc["message"] = "Missing scan type";
    } else {
      String scanType = args[1].toLowerCase();

      if (scanType == "wifi") {
        doc["status"] = "success";
        doc["command"] = "scan_wifi";
        networkScanner->scanWiFiNetworks();
        generateWifiScanJson(doc);
      }
      else if (scanType == "mqtt") {
        if (WiFi.status() != WL_CONNECTED) {
          doc["status"] = "error";
          doc["message"] = "Not connected to WiFi";
        } else {
          String prefix = "";
          if (argCount >= 3) {
            prefix = args[2];
          }

          doc["status"] = "success";
          doc["command"] = "scan_mqtt";
          networkScanner->scanMQTTBrokers(prefix);
          generateMqttScanJson(doc);
        }
      }
      #ifdef ENABLE_BLUETOOTH
      else if (scanType == "ble") {
        doc["status"] = "success";
        doc["command"] = "scan_ble";
        networkScanner->scanBLEDevices();
        // BLE scan results would be added here
      }
      #endif
      else {
        doc["status"] = "error";
        doc["message"] = "Unknown scan type";
      }
    }
  }
  else if (cmd == "attack") {
    if (argCount < 2) {
      doc["status"] = "error";
      doc["message"] = "Missing attack type";
    } else {
      String attackType = args[1].toLowerCase();
      unsigned long duration = DEFAULT_ATTACK_DURATION;

      // Parse duration if provided
      if (argCount >= 3) {
        duration = args[2].toInt();
      }

      // Start the attack
      uint8_t type = 0;
      bool validType = true;

      if (attackType == "deauth") {
        type = ATTACK_TYPE_DEAUTH;
      }
      else if (attackType == "beacon") {
        type = ATTACK_TYPE_BEACON_FLOOD;
      }
      else if (attackType == "probe") {
        type = ATTACK_TYPE_PROBE_SPAM;
      }
      else if (attackType == "arp") {
        type = ATTACK_TYPE_ARP_SPOOF;
      }
      else if (attackType == "mqtt") {
        type = ATTACK_TYPE_MQTT_SPOOF;
      }
      else if (attackType == "eviltwin") {
        type = ATTACK_TYPE_EVIL_TWIN;
      }
      else if (attackType == "sniff") {
        type = ATTACK_TYPE_PASSIVE_SNIFF;
      }
      #ifdef ENABLE_BLUETOOTH
      else if (attackType == "bluetooth") {
        type = ATTACK_TYPE_BLUETOOTH_SCAN;
      }
      #endif
      #ifdef ENABLE_JAMMING
      else if (attackType == "jam") {
        type = ATTACK_TYPE_JAMMING;
      }
      #endif
      else {
        validType = false;
        doc["status"] = "error";
        doc["message"] = "Unknown attack type";
      }

      if (validType) {
        if (attackManager->startAttack(type, duration)) {
          doc["status"] = "success";
          doc["command"] = "attack";
          doc["type"] = attackType;
          if (duration > 0) {
            doc["duration"] = duration;
          }
        } else {
          Attack* attack = attackManager->getAttack(type);
          doc["status"] = "error";
          if (attack) {
            doc["message"] = attack->getErrorMessage();
          } else {
            doc["message"] = "Unknown error starting attack";
          }
        }
      }
    }
  }
  else if (cmd == "stop") {
    if (argCount < 2) {
      // Stop all attacks
      attackManager->stopAllAttacks();
      doc["status"] = "success";
      doc["command"] = "stop_all";
    } else {
      String attackType = args[1].toLowerCase();
      uint8_t type = 0;
      bool validType = true;

      if (attackType == "deauth") {
        type = ATTACK_TYPE_DEAUTH;
      }
      else if (attackType == "beacon") {
        type = ATTACK_TYPE_BEACON_FLOOD;
      }
      else if (attackType == "probe") {
        type = ATTACK_TYPE_PROBE_SPAM;
      }
      else if (attackType == "arp") {
        type = ATTACK_TYPE_ARP_SPOOF;
      }
      else if (attackType == "mqtt") {
        type = ATTACK_TYPE_MQTT_SPOOF;
      }
      else if (attackType == "eviltwin") {
        type = ATTACK_TYPE_EVIL_TWIN;
      }
      else if (attackType == "sniff") {
        type = ATTACK_TYPE_PASSIVE_SNIFF;
      }
      #ifdef ENABLE_BLUETOOTH
      else if (attackType == "bluetooth") {
        type = ATTACK_TYPE_BLUETOOTH_SCAN;
      }
      #endif
      #ifdef ENABLE_JAMMING
      else if (attackType == "jam") {
        type = ATTACK_TYPE_JAMMING;
      }
      #endif
      else {
        validType = false;
        doc["status"] = "error";
        doc["message"] = "Unknown attack type";
      }

      if (validType) {
        attackManager->stopAttack(type);
        doc["status"] = "success";
        doc["command"] = "stop";
        doc["type"] = attackType;
      }
    }
  }
  else if (cmd == "status") {
    doc["status"] = "success";
    doc["command"] = "status";
    generateStatusJson(doc);
  }
  else if (cmd == "set") {
    if (argCount < 4) {
      doc["status"] = "error";
      doc["message"] = "Usage: set <attack> <parameter> <value>";
    } else {
      String attackType = args[1].toLowerCase();
      String parameter = args[2];
      String value = args[3];

      uint8_t type = 0;
      bool validType = true;

      if (attackType == "deauth") {
        type = ATTACK_TYPE_DEAUTH;
      }
      else if (attackType == "beacon") {
        type = ATTACK_TYPE_BEACON_FLOOD;
      }
      else if (attackType == "probe") {
        type = ATTACK_TYPE_PROBE_SPAM;
      }
      else if (attackType == "arp") {
        type = ATTACK_TYPE_ARP_SPOOF;
      }
      else if (attackType == "mqtt") {
        type = ATTACK_TYPE_MQTT_SPOOF;
      }
      else if (attackType == "eviltwin") {
        type = ATTACK_TYPE_EVIL_TWIN;
      }
      else if (attackType == "sniff") {
        type = ATTACK_TYPE_PASSIVE_SNIFF;
      }
      #ifdef ENABLE_BLUETOOTH
      else if (attackType == "bluetooth") {
        type = ATTACK_TYPE_BLUETOOTH_SCAN;
      }
      #endif
      #ifdef ENABLE_JAMMING
      else if (attackType == "jam") {
        type = ATTACK_TYPE_JAMMING;
      }
      #endif
      else {
        validType = false;
        doc["status"] = "error";
        doc["message"] = "Unknown attack type";
      }

      if (validType) {
        Attack* attack = attackManager->getAttack(type);
        if (attack) {
          if (attack->setParameter(parameter, value)) {
            doc["status"] = "success";
            doc["command"] = "set";
            doc["attack"] = attackType;
            doc["parameter"] = parameter;
            doc["value"] = value;
          } else {
            doc["status"] = "error";
            doc["message"] = attack->getErrorMessage();
          }
        } else {
          doc["status"] = "error";
          doc["message"] = "Attack not found";
        }
      }
    }
  }
  else if (cmd == "get") {
    if (argCount < 3) {
      doc["status"] = "error";
      doc["message"] = "Usage: get <attack> <parameter>";
    } else {
      String attackType = args[1].toLowerCase();
      String parameter = args[2];

      uint8_t type = 0;
      bool validType = true;

      if (attackType == "deauth") {
        type = ATTACK_TYPE_DEAUTH;
      }
      else if (attackType == "beacon") {
        type = ATTACK_TYPE_BEACON_FLOOD;
      }
      else if (attackType == "probe") {
        type = ATTACK_TYPE_PROBE_SPAM;
      }
      else if (attackType == "arp") {
        type = ATTACK_TYPE_ARP_SPOOF;
      }
      else if (attackType == "mqtt") {
        type = ATTACK_TYPE_MQTT_SPOOF;
      }
      else if (attackType == "eviltwin") {
        type = ATTACK_TYPE_EVIL_TWIN;
      }
      else if (attackType == "sniff") {
        type = ATTACK_TYPE_PASSIVE_SNIFF;
      }
      #ifdef ENABLE_BLUETOOTH
      else if (attackType == "bluetooth") {
        type = ATTACK_TYPE_BLUETOOTH_SCAN;
      }
      #endif
      #ifdef ENABLE_JAMMING
      else if (attackType == "jam") {
        type = ATTACK_TYPE_JAMMING;
      }
      #endif
      else {
        validType = false;
        doc["status"] = "error";
        doc["message"] = "Unknown attack type";
      }

      if (validType) {
        Attack* attack = attackManager->getAttack(type);
        if (attack) {
          String value = attack->getParameter(parameter);
          if (value.length() > 0) {
            doc["status"] = "success";
            doc["command"] = "get";
            doc["attack"] = attackType;
            doc["parameter"] = parameter;
            doc["value"] = value;
          } else {
            doc["status"] = "error";
            doc["message"] = "Parameter not found";
          }
        } else {
          doc["status"] = "error";
          doc["message"] = "Attack not found";
        }
      }
    }
  }
  else if (cmd == "interactive") {
    // Switch to interactive mode
    setMode(SERIAL_MODE_INTERACTIVE);
    doc["status"] = "success";
    doc["command"] = "interactive";
    doc["message"] = "Switched to interactive mode";
  }
  else {
    doc["status"] = "error";
    doc["message"] = "Unknown command";
  }

  // Send JSON response
  sendJsonResponse(doc);
}

void SerialInterface::sendJsonResponse(const JsonDocument& doc) {
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
}

void SerialInterface::generateWifiScanJson(JsonDocument& doc) {
  JsonArray networks = doc.createNestedArray("networks");

  std::vector<WiFiNetwork>& wifiNetworks = networkScanner->getWiFiNetworks();
  for (const WiFiNetwork& network : wifiNetworks) {
    JsonObject netObj = networks.createNestedObject();
    netObj["ssid"] = network.ssid;

    char bssid[18];
    snprintf(bssid, sizeof(bssid), "%02X:%02X:%02X:%02X:%02X:%02X",
             network.bssid[0], network.bssid[1], network.bssid[2],
             network.bssid[3], network.bssid[4], network.bssid[5]);
    netObj["bssid"] = bssid;

    netObj["rssi"] = network.rssi;
    netObj["channel"] = network.channel;
    netObj["encryption"] = network.encryptionType;
    netObj["hidden"] = network.isHidden;
  }
}

void SerialInterface::generateMqttScanJson(JsonDocument& doc) {
  JsonArray brokers = doc.createNestedArray("brokers");

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

void SerialInterface::generateStatusJson(JsonDocument& doc) {
  // Attack status
  JsonArray attacks = doc.createNestedArray("attacks");

  int count = attackManager->getAttackCount();
  for (int i = 0; i < count; i++) {
    Attack* attack = attackManager->getAttackByIndex(i);
    if (attack) {
      JsonObject attackObj = attacks.createNestedObject();
      attackObj["name"] = attack->getName();
      attackObj["status"] = attack->getStatusString();
      attackObj["running"] = attack->isRunning();
    }
  }

  // Network scanner status
  JsonObject scanner = doc.createNestedObject("scanner");
  scanner["wifi_networks"] = networkScanner->getWiFiNetworks().size();
  scanner["mqtt_brokers"] = networkScanner->getMQTTBrokers().size();
  #ifdef ENABLE_BLUETOOTH
  scanner["ble_devices"] = networkScanner->getBLEDevices().size();
  #endif

  // Device status
  JsonObject device = doc.createNestedObject("device");
  device["free_heap"] = ESP.getFreeHeap();
  device["uptime"] = millis() / 1000;
  device["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  if (WiFi.status() == WL_CONNECTED) {
    device["ip_address"] = WiFi.localIP().toString();
  }
}

void SerialInterface::printAttackList() {
  Serial.println("  deauth    - WiFi Deauthentication Attack");
  Serial.println("  beacon    - Beacon Flood Attack");
  Serial.println("  probe     - Probe Request Spam Attack");
  Serial.println("  arp       - ARP Spoofing Attack");
  Serial.println("  mqtt      - MQTT Spoofing Attack");
  Serial.println("  eviltwin  - Evil Twin Attack");
  Serial.println("  sniff     - Passive Sniffing");
  #ifdef ENABLE_BLUETOOTH
  Serial.println("  bluetooth - Bluetooth Scanning Attack");
  #endif
  #ifdef ENABLE_JAMMING
  Serial.println("  jam       - 2.4GHz Jamming Attack");
  #endif
}

void SerialInterface::printScanOptions() {
  Serial.println("Usage: scan <type>");
  Serial.println("Available scan types:");
  Serial.println("  wifi      - Scan for WiFi networks");
  Serial.println("  mqtt      - Scan for MQTT brokers");
  #ifdef ENABLE_BLUETOOTH
  Serial.println("  ble       - Scan for Bluetooth LE devices");
  #endif
}

void SerialInterface::printPrompt() {
  Serial.print("SECoT> ");
}
