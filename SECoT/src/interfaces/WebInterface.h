/*
 * WebInterface.h
 * 
 * Web-based interface for SECoT
 * Provides a web server for remote control and monitoring
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "../attacks/AttackManager.h"
#include "../utils/NetworkScanner.h"
#include "../config/Config.h"

class WebInterface {
  public:
    WebInterface();
    
    // Initialize the interface
    void begin(AttackManager* attackManager, NetworkScanner* networkScanner);
    
    // Update the interface (called in main loop)
    void update();
    
  private:
    // References to other components
    AttackManager* attackManager;
    NetworkScanner* networkScanner;
    
    // Web server
    AsyncWebServer* server;
    
    // WiFi AP mode
    bool apMode;
    String apSSID;
    String apPassword;
    IPAddress apIP;
    
    // State variables
    bool serverStarted;
    unsigned long lastUpdate;
    
    // Helper methods
    void setupAccessPoint();
    void setupWebServer();
    void handleWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                             AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    // API handlers
    void handleAPIStatus(AsyncWebServerRequest* request);
    void handleAPIAttacks(AsyncWebServerRequest* request);
    void handleAPIAttackStart(AsyncWebServerRequest* request);
    void handleAPIAttackStop(AsyncWebServerRequest* request);
    void handleAPIAttackParams(AsyncWebServerRequest* request);
    void handleAPIAttackSetParam(AsyncWebServerRequest* request);
    void handleAPIScan(AsyncWebServerRequest* request);
    void handleAPIScanResults(AsyncWebServerRequest* request);
    
    // JSON response helpers
    String getStatusJSON();
    String getAttacksJSON();
    String getAttackParamsJSON(uint8_t attackType);
    String getScanResultsJSON(uint8_t scanType);
    
    // WebSocket
    AsyncWebSocket* ws;
    void notifyClients();
    void parseWebSocketMessage(uint8_t* data, size_t len);
};

#endif // WEB_INTERFACE_H
