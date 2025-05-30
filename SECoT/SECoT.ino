/*
 * SECoT - Security Evaluation and Compromise Toolkit
 *
 * An ESP32-based security audit tool for IoT systems
 * Inspired by tools like Flipper Zero, ESP32 Marauder, and ESP Ghost
 *
 * This tool is designed for educational purposes only to demonstrate
 * common vulnerabilities in IoT systems. Use responsibly and legally.
 *
 * Hardware: ESP32-WROOM-32D
 * Optional: TFT display (ILI9341 or similar)
 *
 * Author: Amen Ellah Kerimi & Nadine Wellani
 * License: MIT
 */

// Include configuration
#include "src/config/Config.h"

// Include interfaces
#include "src/interfaces/SerialInterface.h"
#ifdef ENABLE_TFT
  #include "src/interfaces/TFTInterface.h"
#endif
#ifdef ENABLE_WEB_INTERFACE
  #include "src/interfaces/WebInterface.h"
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <ArduinoJson.h>
  #include <SPIFFS.h>
#endif

// Include attack modules
#include "src/attacks/AttackManager.h"

// Include utilities
#include "src/utils/NetworkScanner.h"
#include "src/utils/Logger.h"

// Global objects
SerialInterface serialInterface;
AttackManager attackManager;
NetworkScanner networkScanner;
Logger logger;

#ifdef ENABLE_TFT
  TFTInterface tftInterface;

  // Button pins for TFT interface
  const int BUTTON_UP_PIN = 35;
  const int BUTTON_DOWN_PIN = 32;
  const int BUTTON_LEFT_PIN = 33;
  const int BUTTON_RIGHT_PIN = 25;
  const int BUTTON_SELECT_PIN = 26;
  const int BUTTON_BACK_PIN = 27;
#endif

#ifdef ENABLE_WEB_INTERFACE
  WebInterface webInterface;
#endif

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(500);

  // Print welcome message
  Serial.println();
  Serial.println("==============================================");
  Serial.println("  SECoT - Security Evaluation & Compromise Toolkit");
  Serial.println("==============================================");
  Serial.println("  Educational purposes only. Use responsibly.");
  Serial.println("==============================================");
  Serial.println();

  // Initialize logger
  logger.begin();
  logger.log(LOG_INFO, "SECoT", "Initializing...");

  #ifdef ENABLE_TFT
    // Initialize button pins
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_BACK_PIN, INPUT_PULLUP);
    logger.log(LOG_INFO, "SECoT", "Button pins initialized");
  #endif

  // Initialize interfaces
  serialInterface.begin(&attackManager, &networkScanner);

  #ifdef ENABLE_TFT
    tftInterface.begin(&attackManager, &networkScanner);
    logger.log(LOG_INFO, "SECoT", "TFT interface initialized");
  #endif

  #ifdef ENABLE_WEB_INTERFACE
    webInterface.begin(&attackManager, &networkScanner);
    logger.log(LOG_INFO, "SECoT", "Web interface initialized");
  #endif

  // Initialize network scanner
  networkScanner.begin();
  logger.log(LOG_INFO, "SECoT", "Network scanner initialized");

  // Initialize attack manager
  attackManager.begin();
  logger.log(LOG_INFO, "SECoT", "Attack manager initialized");

  // Ready
  logger.log(LOG_INFO, "SECoT", "Initialization complete");
  Serial.println("SECoT is ready. Type 'help' for available commands.");
}

void loop() {
  // Process serial interface
  serialInterface.update();

  #ifdef ENABLE_TFT
    // Only update TFT if not connected to CLI tool
    if (!serialInterface.isConnectedToCLITool()) {
      // Check button states
      if (digitalRead(BUTTON_UP_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_UP);
      }
      if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_DOWN);
      }
      if (digitalRead(BUTTON_LEFT_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_LEFT);
      }
      if (digitalRead(BUTTON_RIGHT_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_RIGHT);
      }
      if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_SELECT);
      }
      if (digitalRead(BUTTON_BACK_PIN) == LOW) {
        tftInterface.handleButton(BUTTON_BACK);
      }

      // Update TFT interface
      tftInterface.update();
    }
  #endif

  #ifdef ENABLE_WEB_INTERFACE
    // Always update web interface regardless of CLI tool connection
    webInterface.update();
  #endif

  // Update attack manager
  attackManager.update();

  // Update network scanner
  networkScanner.update();

  // Small delay to prevent CPU hogging
  delay(10);
}
