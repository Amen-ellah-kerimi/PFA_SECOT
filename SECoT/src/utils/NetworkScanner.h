/*
 * NetworkScanner.h
 * 
 * Network scanning utility for SECoT
 */

#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "../config/Config.h"

// Network types
#define NETWORK_TYPE_WIFI 0
#define NETWORK_TYPE_MQTT 1
#define NETWORK_TYPE_BLE 2

// WiFi network structure
struct WiFiNetwork {
  String ssid;
  uint8_t bssid[6];
  int32_t rssi;
  uint8_t channel;
  uint8_t encryptionType;
  bool isHidden;
};

// MQTT broker structure
struct MQTTBroker {
  String ip;
  uint16_t port;
  bool requiresAuth;
  bool supportsTLS;
  bool isAccessible;
};

// BLE device structure
struct BLEDevice {
  String address;
  String name;
  int32_t rssi;
  bool isConnectable;
};

class NetworkScanner {
  public:
    NetworkScanner();
    
    // Initialize the scanner
    void begin();
    
    // Scan for WiFi networks
    bool scanWiFiNetworks(unsigned long timeout = DEFAULT_SCAN_TIME);
    
    // Scan for MQTT brokers on the current network
    bool scanMQTTBrokers(const String& networkPrefix, uint16_t startPort = 1883, uint16_t endPort = 1884);
    
    // Scan for BLE devices
    #ifdef ENABLE_BLUETOOTH
    bool scanBLEDevices(unsigned long timeout = DEFAULT_SCAN_TIME);
    #endif
    
    // Get scan results
    std::vector<WiFiNetwork>& getWiFiNetworks();
    std::vector<MQTTBroker>& getMQTTBrokers();
    #ifdef ENABLE_BLUETOOTH
    std::vector<BLEDevice>& getBLEDevices();
    #endif
    
    // Clear scan results
    void clearResults(uint8_t networkType);
    void clearAllResults();
    
    // Check if scan is in progress
    bool isScanning();
    
    // Stop any ongoing scan
    void stopScan();
    
  private:
    std::vector<WiFiNetwork> wifiNetworks;
    std::vector<MQTTBroker> mqttBrokers;
    #ifdef ENABLE_BLUETOOTH
    std::vector<BLEDevice> bleDevices;
    #endif
    
    bool scanning;
    unsigned long scanStartTime;
    unsigned long scanTimeout;
    
    // Helper methods
    bool testMQTTBroker(const String& ip, uint16_t port);
    void printMACAddress(const uint8_t* mac);
};

#endif // NETWORK_SCANNER_H
