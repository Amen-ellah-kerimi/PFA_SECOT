/*
 * SerialInterface.h
 *
 * Serial command interface for SECoT
 * Supports both interactive mode and CLI tool communication
 */

#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../attacks/AttackManager.h"
#include "../utils/NetworkScanner.h"

// Serial interface modes
#define SERIAL_MODE_INTERACTIVE 0  // Interactive command line
#define SERIAL_MODE_CLI_TOOL    1  // Communication with SECoT CLI Tool

class SerialInterface {
  public:
    SerialInterface();

    // Initialize the interface
    void begin(AttackManager* attackManager, NetworkScanner* networkScanner);

    // Update the interface (called in main loop)
    void update();

    // Check if connected to CLI tool
    bool isConnectedToCLITool();

    // Set the interface mode
    void setMode(uint8_t mode);

  private:
    AttackManager* attackManager;
    NetworkScanner* networkScanner;

    char commandBuffer[SERIAL_COMMAND_BUFFER];
    int bufferIndex;

    uint8_t interfaceMode;
    bool cliToolConnected;
    unsigned long lastCliToolPing;

    // Process a complete command
    void processCommand(const String& command);

    // Parse command arguments
    int parseArgs(const String& command, String args[], int maxArgs);

    // Command handlers
    void handleHelp();
    void handleScan(const String args[], int argCount);
    void handleAttack(const String args[], int argCount);
    void handleStop(const String args[], int argCount);
    void handleStatus();
    void handleSet(const String args[], int argCount);
    void handleGet(const String args[], int argCount);
    void handleClear();
    void handlePing();

    // CLI Tool specific handlers
    void handleCliToolCommand(const String& command);
    void sendJsonResponse(const JsonDocument& doc);

    // Helper methods
    void printAttackList();
    void printScanOptions();
    void printPrompt();

    // JSON response generators
    void generateWifiScanJson(JsonDocument& doc);
    void generateMqttScanJson(JsonDocument& doc);
    void generateStatusJson(JsonDocument& doc);
};

#endif // SERIAL_INTERFACE_H
