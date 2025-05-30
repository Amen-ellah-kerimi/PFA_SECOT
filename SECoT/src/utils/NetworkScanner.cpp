/*
 * NetworkScanner.cpp
 * 
 * Implementation of the NetworkScanner class
 */

#include "NetworkScanner.h"
#include <WiFi.h>
#include <PubSubClient.h>

NetworkScanner::NetworkScanner() {
  scanning = false;
  scanStartTime = 0;
  scanTimeout = DEFAULT_SCAN_TIME;
}

void NetworkScanner::begin() {
  // Initialize WiFi in station mode for scanning
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

bool NetworkScanner::scanWiFiNetworks(unsigned long timeout) {
  // Check if already scanning
  if (scanning) {
    return false;
  }
  
  // Clear previous results
  wifiNetworks.clear();
  
  // Start scan
  scanning = true;
  scanStartTime = millis();
  scanTimeout = timeout;
  
  Serial.println("Starting WiFi scan...");
  int networksFound = WiFi.scanNetworks();
  
  // Process results
  if (networksFound > 0) {
    Serial.print(networksFound);
    Serial.println(" networks found");
    
    for (int i = 0; i < networksFound; i++) {
      WiFiNetwork network;
      network.ssid = WiFi.SSID(i);
      memcpy(network.bssid, WiFi.BSSID(i), 6);
      network.rssi = WiFi.RSSI(i);
      network.channel = WiFi.channel(i);
      network.encryptionType = WiFi.encryptionType(i);
      network.isHidden = WiFi.isHidden(i);
      
      wifiNetworks.push_back(network);
      
      // Print network info
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(network.ssid);
      Serial.print(" (");
      printMACAddress(network.bssid);
      Serial.print(") Ch:");
      Serial.print(network.channel);
      Serial.print(" RSSI:");
      Serial.print(network.rssi);
      Serial.print("dBm Encryption:");
      Serial.println(network.encryptionType);
    }
  } else {
    Serial.println("No networks found");
  }
  
  // Clean up
  WiFi.scanDelete();
  scanning = false;
  
  return (networksFound > 0);
}

bool NetworkScanner::scanMQTTBrokers(const String& networkPrefix, uint16_t startPort, uint16_t endPort) {
  // Check if already scanning
  if (scanning) {
    return false;
  }
  
  // Check if connected to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot scan for MQTT brokers: Not connected to WiFi");
    return false;
  }
  
  // Clear previous results
  mqttBrokers.clear();
  
  // Start scan
  scanning = true;
  scanStartTime = millis();
  
  Serial.println("Scanning for MQTT brokers...");
  
  // Get local IP and subnet
  IPAddress localIP = WiFi.localIP();
  IPAddress subnet = WiFi.subnetMask();
  
  // Calculate network address
  IPAddress networkIP = IPAddress(
    localIP[0] & subnet[0],
    localIP[1] & subnet[1],
    localIP[2] & subnet[2],
    localIP[3] & subnet[3]
  );
  
  // Calculate broadcast address
  IPAddress broadcastIP = IPAddress(
    networkIP[0] | (~subnet[0] & 255),
    networkIP[1] | (~subnet[1] & 255),
    networkIP[2] | (~subnet[2] & 255),
    networkIP[3] | (~subnet[3] & 255)
  );
  
  // Determine scan range
  IPAddress startIP, endIP;
  
  if (networkPrefix.length() > 0) {
    // Use provided prefix
    int lastDot = networkPrefix.lastIndexOf('.');
    if (lastDot > 0) {
      String baseIP = networkPrefix.substring(0, lastDot + 1);
      startIP.fromString(baseIP + "1");
      endIP.fromString(baseIP + "254");
    } else {
      // Invalid prefix, use local subnet
      startIP = IPAddress(networkIP[0], networkIP[1], networkIP[2], 1);
      endIP = IPAddress(networkIP[0], networkIP[1], networkIP[2], 254);
    }
  } else {
    // Use local subnet
    startIP = IPAddress(networkIP[0], networkIP[1], networkIP[2], 1);
    endIP = IPAddress(networkIP[0], networkIP[1], networkIP[2], 254);
  }
  
  Serial.print("Scanning IP range: ");
  Serial.print(startIP.toString());
  Serial.print(" to ");
  Serial.println(endIP.toString());
  
  // Scan each IP address
  int brokersFound = 0;
  for (int i = startIP[3]; i <= endIP[3]; i++) {
    IPAddress ip(startIP[0], startIP[1], startIP[2], i);
    
    // Skip our own IP
    if (ip == localIP) {
      continue;
    }
    
    // Test each port
    for (uint16_t port = startPort; port <= endPort; port++) {
      if (testMQTTBroker(ip.toString(), port)) {
        MQTTBroker broker;
        broker.ip = ip.toString();
        broker.port = port;
        broker.requiresAuth = false; // We don't know yet
        broker.supportsTLS = false;  // We don't know yet
        broker.isAccessible = true;
        
        mqttBrokers.push_back(broker);
        brokersFound++;
        
        Serial.print("Found MQTT broker at ");
        Serial.print(ip.toString());
        Serial.print(":");
        Serial.println(port);
      }
    }
  }
  
  Serial.print("Found ");
  Serial.print(brokersFound);
  Serial.println(" MQTT brokers");
  
  // Clean up
  scanning = false;
  
  return (brokersFound > 0);
}

#ifdef ENABLE_BLUETOOTH
bool NetworkScanner::scanBLEDevices(unsigned long timeout) {
  // BLE scanning implementation would go here
  // Not implemented in this version
  return false;
}
#endif

std::vector<WiFiNetwork>& NetworkScanner::getWiFiNetworks() {
  return wifiNetworks;
}

std::vector<MQTTBroker>& NetworkScanner::getMQTTBrokers() {
  return mqttBrokers;
}

#ifdef ENABLE_BLUETOOTH
std::vector<BLEDevice>& NetworkScanner::getBLEDevices() {
  return bleDevices;
}
#endif

void NetworkScanner::clearResults(uint8_t networkType) {
  switch (networkType) {
    case NETWORK_TYPE_WIFI:
      wifiNetworks.clear();
      break;
    case NETWORK_TYPE_MQTT:
      mqttBrokers.clear();
      break;
    #ifdef ENABLE_BLUETOOTH
    case NETWORK_TYPE_BLE:
      bleDevices.clear();
      break;
    #endif
  }
}

void NetworkScanner::clearAllResults() {
  wifiNetworks.clear();
  mqttBrokers.clear();
  #ifdef ENABLE_BLUETOOTH
  bleDevices.clear();
  #endif
}

bool NetworkScanner::isScanning() {
  return scanning;
}

void NetworkScanner::stopScan() {
  if (scanning) {
    WiFi.scanDelete();
    scanning = false;
  }
}

bool NetworkScanner::testMQTTBroker(const String& ip, uint16_t port) {
  WiFiClient client;
  
  // Try to connect to the IP and port
  if (!client.connect(ip.c_str(), port)) {
    return false;
  }
  
  // Connected, now check if it's an MQTT broker
  // This is a simple check that just verifies if the port is open
  // A more thorough check would involve sending MQTT CONNECT packet
  // and checking the response, but that's more complex
  
  client.stop();
  return true;
}

void NetworkScanner::printMACAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
}
