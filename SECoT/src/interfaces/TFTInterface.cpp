/*
 * TFTInterface.cpp
 *
 * Implementation of the TFT display interface for SECoT
 */

#include "TFTInterface.h"

// Define colors
#define TFT_BACKGROUND TFT_BLACK
#define TFT_TEXT TFT_WHITE
#define TFT_HEADER_BG 0x5AEB  // Dark blue
#define TFT_FOOTER_BG 0x5AEB  // Dark blue
#define TFT_SELECTED_BG 0x04FF // Bright blue
#define TFT_MENU_BG 0x2124    // Dark gray
#define TFT_SUCCESS 0x07E0    // Green
#define TFT_WARNING 0xFD20    // Orange
#define TFT_ERROR 0xF800      // Red
#define TFT_INFO 0x07FF       // Cyan

TFTInterface::TFTInterface() {
  // Initialize variables
  currentMenuId = 0;
  selectedMenuItem = 0;
  menuScrollOffset = 0;
  currentScreen = SCREEN_MAIN_MENU;
  selectedAttackType = 0;
  selectedDeviceMAC = "";
  needsRedraw = true;
  lastUpdate = 0;
  lastButtonCheck = 0;

  // Initialize button states
  for (int i = 0; i < 6; i++) {
    buttonState[i] = false;
    lastButtonState[i] = false;
  }
}

void TFTInterface::begin(AttackManager* attackManager, NetworkScanner* networkScanner) {
  this->attackManager = attackManager;
  this->networkScanner = networkScanner;

  // Initialize TFT display
  tft = new TFT_eSPI();
  tft->init();
  tft->setRotation(0); // Portrait mode
  tft->fillScreen(TFT_BACKGROUND);

  // Set text properties
  tft->setTextColor(TFT_TEXT);
  tft->setTextSize(1);

  // Initialize menu system
  initializeMenu();

  // Draw initial screen
  drawScreen();

  Serial.println("TFT interface initialized");
}

void TFTInterface::update() {
  unsigned long currentMillis = millis();

  // Check buttons every 50ms
  if (currentMillis - lastButtonCheck >= 50) {
    checkButtons();
    lastButtonCheck = currentMillis;
  }

  // Update screen every 250ms
  if (currentMillis - lastUpdate >= 250 || needsRedraw) {
    drawScreen();
    lastUpdate = currentMillis;
    needsRedraw = false;
  }
}

void TFTInterface::initializeMenu() {
  menuItems.clear();

  // Main menu (ID 0)
  menuItems.push_back({.name = "Attacks", .id = 1, .parentId = 0, .isFolder = true, .isAttack = false, .attackType = 0});
  menuItems.push_back({.name = "Scan", .id = 2, .parentId = 0, .isFolder = true, .isAttack = false, .attackType = 0});
  menuItems.push_back({.name = "Statistics", .id = 3, .parentId = 0, .isFolder = false, .isAttack = false, .attackType = 0});
  menuItems.push_back({.name = "About", .id = 4, .parentId = 0, .isFolder = false, .isAttack = false, .attackType = 0});

  // Attacks submenu (ID 1)
  menuItems.push_back({.name = "WiFi Deauth", .id = 10, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_DEAUTH});
  menuItems.push_back({.name = "Beacon Flood", .id = 11, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_BEACON_FLOOD});
  menuItems.push_back({.name = "Probe Spam", .id = 12, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_PROBE_SPAM});
  menuItems.push_back({.name = "ARP Spoof", .id = 13, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_ARP_SPOOF});
  menuItems.push_back({.name = "MQTT Spoof", .id = 14, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_MQTT_SPOOF});
  menuItems.push_back({.name = "Evil Twin", .id = 15, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_EVIL_TWIN});
  menuItems.push_back({.name = "Passive Sniff", .id = 16, .parentId = 1, .isFolder = false, .isAttack = true, .attackType = ATTACK_TYPE_PASSIVE_SNIFF});

  // Scan submenu (ID 2)
  menuItems.push_back({.name = "WiFi Networks", .id = 20, .parentId = 2, .isFolder = false, .isAttack = false, .attackType = 0});
  menuItems.push_back({.name = "MQTT Brokers", .id = 21, .parentId = 2, .isFolder = false, .isAttack = false, .attackType = 0});
  menuItems.push_back({.name = "BLE Devices", .id = 22, .parentId = 2, .isFolder = false, .isAttack = false, .attackType = 0});
}

void TFTInterface::drawScreen() {
  switch (currentScreen) {
    case SCREEN_MAIN_MENU:
      drawMainMenu();
      break;
    case SCREEN_ATTACK_MENU:
      drawAttackMenu();
      break;
    case SCREEN_SCAN_MENU:
      drawScanMenu();
      break;
    case SCREEN_ATTACK_RUNNING:
      drawAttackRunning();
      break;
    case SCREEN_ATTACK_CONFIG:
      drawAttackConfig();
      break;
    case SCREEN_SCAN_RESULTS:
      drawScanResults();
      break;
    case SCREEN_DEVICE_INFO:
      drawDeviceInfo();
      break;
    case SCREEN_STATS:
      drawStats();
      break;
    case SCREEN_ABOUT:
      drawAbout();
      break;
  }
}

void TFTInterface::drawHeader(const String& title) {
  // Draw header background
  tft->fillRect(0, 0, tft->width(), MENU_HEADER_HEIGHT, TFT_HEADER_BG);

  // Draw title
  tft->setTextColor(TFT_TEXT);
  tft->setTextSize(2);
  tft->setCursor(10, 10);
  tft->print(title);

  // Draw battery indicator (placeholder)
  tft->drawRect(tft->width() - 30, 10, 20, 10, TFT_TEXT);
  tft->fillRect(tft->width() - 30, 10, 15, 10, TFT_SUCCESS);

  // Draw separator line
  tft->drawFastHLine(0, MENU_HEADER_HEIGHT - 1, tft->width(), TFT_TEXT);

  // Reset text size
  tft->setTextSize(1);
}

void TFTInterface::drawFooter(const String& leftAction, const String& rightAction) {
  int footerY = tft->height() - MENU_FOOTER_HEIGHT;

  // Draw footer background
  tft->fillRect(0, footerY, tft->width(), MENU_FOOTER_HEIGHT, TFT_FOOTER_BG);

  // Draw separator line
  tft->drawFastHLine(0, footerY, tft->width(), TFT_TEXT);

  // Draw left action
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(5, footerY + 5);
  tft->print(leftAction);

  // Draw right action
  int rightWidth = rightAction.length() * 6; // Approximate width
  tft->setCursor(tft->width() - rightWidth - 5, footerY + 5);
  tft->print(rightAction);
}

void TFTInterface::drawMenuItem(const MenuItem& item, int y, bool selected) {
  // Draw background
  if (selected) {
    tft->fillRect(0, y, tft->width(), MENU_ITEM_HEIGHT, TFT_SELECTED_BG);
  } else {
    tft->fillRect(0, y, tft->width(), MENU_ITEM_HEIGHT, TFT_MENU_BG);
  }

  // Draw item text
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, y + 6);
  tft->print(item.name);

  // Draw folder/attack indicator
  if (item.isFolder) {
    // Draw folder icon
    tft->fillRect(tft->width() - 20, y + 5, 10, 10, TFT_TEXT);
    tft->fillTriangle(tft->width() - 15, y + 5, tft->width() - 10, y + 10, tft->width() - 15, y + 15, TFT_MENU_BG);
  } else if (item.isAttack) {
    // Draw attack status if applicable
    Attack* attack = attackManager->getAttack(item.attackType);
    if (attack && attack->getStatus() == ATTACK_STATUS_RUNNING) {
      drawStatusIcon(tft->width() - 20, y + 5, true);
    }
  }
}

void TFTInterface::drawProgressBar(int x, int y, int width, int height, int percentage) {
  // Draw border
  tft->drawRect(x, y, width, height, TFT_TEXT);

  // Calculate fill width
  int fillWidth = (width - 2) * percentage / 100;

  // Draw fill
  if (percentage > 0) {
    tft->fillRect(x + 1, y + 1, fillWidth, height - 2, TFT_SUCCESS);
  }
}

void TFTInterface::drawStatusIcon(int x, int y, bool active) {
  if (active) {
    tft->fillCircle(x + 5, y + 5, 5, TFT_SUCCESS);
  } else {
    tft->drawCircle(x + 5, y + 5, 5, TFT_ERROR);
  }
}

void TFTInterface::drawMainMenu() {
  // Draw header
  drawHeader("SECoT");

  // Draw menu items
  std::vector<MenuItem> items = getCurrentMenuItems();
  int startY = MENU_HEADER_HEIGHT;

  for (int i = menuScrollOffset; i < min(menuScrollOffset + MENU_ITEMS_PER_SCREEN, (int)items.size()); i++) {
    int y = startY + (i - menuScrollOffset) * MENU_ITEM_HEIGHT;
    drawMenuItem(items[i], y, i == selectedMenuItem);
  }

  // Fill remaining space
  if (items.size() - menuScrollOffset < MENU_ITEMS_PER_SCREEN) {
    int y = startY + (items.size() - menuScrollOffset) * MENU_ITEM_HEIGHT;
    tft->fillRect(0, y, tft->width(), tft->height() - y - MENU_FOOTER_HEIGHT, TFT_BACKGROUND);
  }

  // Draw footer
  drawFooter("Select", "Back");
}

void TFTInterface::drawAttackMenu() {
  // Draw header
  drawHeader("Attacks");

  // Draw menu items
  std::vector<MenuItem> items = getCurrentMenuItems();
  int startY = MENU_HEADER_HEIGHT;

  for (int i = menuScrollOffset; i < min(menuScrollOffset + MENU_ITEMS_PER_SCREEN, (int)items.size()); i++) {
    int y = startY + (i - menuScrollOffset) * MENU_ITEM_HEIGHT;
    drawMenuItem(items[i], y, i == selectedMenuItem);
  }

  // Fill remaining space
  if (items.size() - menuScrollOffset < MENU_ITEMS_PER_SCREEN) {
    int y = startY + (items.size() - menuScrollOffset) * MENU_ITEM_HEIGHT;
    tft->fillRect(0, y, tft->width(), tft->height() - y - MENU_FOOTER_HEIGHT, TFT_BACKGROUND);
  }

  // Draw footer
  drawFooter("Select", "Back");
}

void TFTInterface::drawScanMenu() {
  // Draw header
  drawHeader("Scan");

  // Draw menu items
  std::vector<MenuItem> items = getCurrentMenuItems();
  int startY = MENU_HEADER_HEIGHT;

  for (int i = menuScrollOffset; i < min(menuScrollOffset + MENU_ITEMS_PER_SCREEN, (int)items.size()); i++) {
    int y = startY + (i - menuScrollOffset) * MENU_ITEM_HEIGHT;
    drawMenuItem(items[i], y, i == selectedMenuItem);
  }

  // Fill remaining space
  if (items.size() - menuScrollOffset < MENU_ITEMS_PER_SCREEN) {
    int y = startY + (items.size() - menuScrollOffset) * MENU_ITEM_HEIGHT;
    tft->fillRect(0, y, tft->width(), tft->height() - y - MENU_FOOTER_HEIGHT, TFT_BACKGROUND);
  }

  // Draw footer
  drawFooter("Select", "Back");
}

void TFTInterface::drawAttackRunning() {
  Attack* attack = attackManager->getAttack(selectedAttackType);
  if (!attack) {
    // If attack not found, go back to attack menu
    currentScreen = SCREEN_ATTACK_MENU;
    return;
  }

  // Draw header
  drawHeader(getAttackName(selectedAttackType));

  // Draw attack status
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, MENU_HEADER_HEIGHT + 10);
  tft->print("Status: ");

  // Set color based on status
  uint16_t statusColor;
  switch (attack->getStatus()) {
    case ATTACK_STATUS_RUNNING:
      statusColor = TFT_SUCCESS;
      break;
    case ATTACK_STATUS_ERROR:
      statusColor = TFT_ERROR;
      break;
    default:
      statusColor = TFT_WARNING;
      break;
  }

  tft->setTextColor(statusColor);
  tft->print(attack->getStatusString());

  // Draw attack parameters
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, MENU_HEADER_HEIGHT + 30);
  tft->print("Parameters:");

  // Get attack parameters as JSON
  String params = attack->getAllParameters();

  // Display parameters (simplified)
  int y = MENU_HEADER_HEIGHT + 50;
  int maxLines = 5; // Maximum number of parameter lines to show
  int lineCount = 0;

  // Very simple JSON parsing (just for display)
  params.replace("{", "");
  params.replace("}", "");
  params.replace("\"", "");

  int startPos = 0;
  int commaPos;

  while ((commaPos = params.indexOf(',', startPos)) != -1 && lineCount < maxLines) {
    String paramPair = params.substring(startPos, commaPos);
    int colonPos = paramPair.indexOf(':');

    if (colonPos != -1) {
      String name = paramPair.substring(0, colonPos);
      String value = paramPair.substring(colonPos + 1);

      tft->setCursor(20, y);
      tft->print(name);
      tft->print(": ");
      tft->print(value);

      y += 20;
      lineCount++;
    }

    startPos = commaPos + 1;
  }

  // Draw elapsed time
  unsigned long elapsedTime = millis() - attack->startTime;
  tft->setCursor(10, tft->height() - MENU_FOOTER_HEIGHT - 30);
  tft->print("Elapsed time: ");
  tft->print(elapsedTime / 1000);
  tft->print(" s");

  // Draw footer
  if (attack->getStatus() == ATTACK_STATUS_RUNNING) {
    drawFooter("Stop", "Back");
  } else {
    drawFooter("Start", "Back");
  }
}

void TFTInterface::drawAttackConfig() {
  Attack* attack = attackManager->getAttack(selectedAttackType);
  if (!attack) {
    // If attack not found, go back to attack menu
    currentScreen = SCREEN_ATTACK_MENU;
    return;
  }

  // Draw header
  drawHeader("Configure " + getAttackName(selectedAttackType));

  // Draw attack parameters
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, MENU_HEADER_HEIGHT + 10);
  tft->print("Parameters:");

  // Get attack parameters as JSON
  String params = attack->getAllParameters();

  // Display parameters (simplified)
  int y = MENU_HEADER_HEIGHT + 30;
  int maxLines = 8; // Maximum number of parameter lines to show
  int lineCount = 0;

  // Very simple JSON parsing (just for display)
  params.replace("{", "");
  params.replace("}", "");
  params.replace("\"", "");

  int startPos = 0;
  int commaPos;

  while ((commaPos = params.indexOf(',', startPos)) != -1 && lineCount < maxLines) {
    String paramPair = params.substring(startPos, commaPos);
    int colonPos = paramPair.indexOf(':');

    if (colonPos != -1) {
      String name = paramPair.substring(0, colonPos);
      String value = paramPair.substring(colonPos + 1);

      tft->setCursor(20, y);
      tft->print(name);
      tft->print(": ");
      tft->print(value);

      y += 20;
      lineCount++;
    }

    startPos = commaPos + 1;
  }

  // Draw instructions
  tft->setCursor(10, tft->height() - MENU_FOOTER_HEIGHT - 40);
  tft->print("Use serial interface to set parameters");

  // Draw footer
  drawFooter("Start", "Back");
}

void TFTInterface::drawScanResults() {
  // Determine which scan results to show
  String title;
  std::vector<String> items;

  if (currentMenuId == 20) {
    // WiFi scan results
    title = "WiFi Networks";

    std::vector<WiFiNetwork>& networks = networkScanner->getWiFiNetworks();
    for (const WiFiNetwork& network : networks) {
      String item = network.ssid + " (Ch:" + String(network.channel) + " RSSI:" + String(network.rssi) + ")";
      items.push_back(item);
    }
  } else if (currentMenuId == 21) {
    // MQTT broker scan results
    title = "MQTT Brokers";

    std::vector<MQTTBroker>& brokers = networkScanner->getMQTTBrokers();
    for (const MQTTBroker& broker : brokers) {
      String item = broker.ip + ":" + String(broker.port);
      items.push_back(item);
    }
  } else if (currentMenuId == 22) {
    // BLE scan results
    title = "BLE Devices";

    #ifdef ENABLE_BLUETOOTH
    std::vector<BLEDevice>& devices = networkScanner->getBLEDevices();
    for (const BLEDevice& device : devices) {
      String item = device.name + " (" + device.address + ")";
      items.push_back(item);
    }
    #endif
  }

  // Draw header
  drawHeader(title);

  // Draw scan results
  int startY = MENU_HEADER_HEIGHT;

  if (items.size() == 0) {
    // No results
    tft->setTextColor(TFT_TEXT);
    tft->setCursor(10, startY + 20);
    tft->print("No results found");
  } else {
    // Draw items
    for (int i = menuScrollOffset; i < min(menuScrollOffset + MENU_ITEMS_PER_SCREEN, (int)items.size()); i++) {
      int y = startY + (i - menuScrollOffset) * MENU_ITEM_HEIGHT;

      // Draw background
      if (i == selectedMenuItem) {
        tft->fillRect(0, y, tft->width(), MENU_ITEM_HEIGHT, TFT_SELECTED_BG);
      } else {
        tft->fillRect(0, y, tft->width(), MENU_ITEM_HEIGHT, TFT_MENU_BG);
      }

      // Draw item text
      tft->setTextColor(TFT_TEXT);
      tft->setCursor(10, y + 6);

      // Truncate text if too long
      String text = items[i];
      if (text.length() > 30) {
        text = text.substring(0, 27) + "...";
      }

      tft->print(text);
    }

    // Fill remaining space
    if (items.size() - menuScrollOffset < MENU_ITEMS_PER_SCREEN) {
      int y = startY + (items.size() - menuScrollOffset) * MENU_ITEM_HEIGHT;
      tft->fillRect(0, y, tft->width(), tft->height() - y - MENU_FOOTER_HEIGHT, TFT_BACKGROUND);
    }
  }

  // Draw footer
  drawFooter("Select", "Back");
}

void TFTInterface::drawDeviceInfo() {
  // This screen would show detailed info about a selected device
  // For now, just a placeholder

  // Draw header
  drawHeader("Device Info");

  // Draw device info
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, MENU_HEADER_HEIGHT + 20);
  tft->print("MAC: ");
  tft->print(selectedDeviceMAC);

  // Draw footer
  drawFooter("", "Back");
}

void TFTInterface::drawStats() {
  // Draw header
  drawHeader("Statistics");

  // Draw attack statistics
  tft->setTextColor(TFT_TEXT);
  tft->setCursor(10, MENU_HEADER_HEIGHT + 10);
  tft->print("Attack Statistics:");

  int y = MENU_HEADER_HEIGHT + 30;

  // Count running attacks
  int runningAttacks = attackManager->getRunningAttackCount();

  tft->setCursor(20, y);
  tft->print("Running Attacks: ");
  tft->print(runningAttacks);
  y += 20;

  // Show individual attack status
  for (int i = 0; i < attackManager->getAttackCount(); i++) {
    Attack* attack = attackManager->getAttackByIndex(i);
    if (attack) {
      tft->setCursor(20, y);
      tft->print(attack->getName());
      tft->print(": ");

      // Set color based on status
      uint16_t statusColor;
      switch (attack->getStatus()) {
        case ATTACK_STATUS_RUNNING:
          statusColor = TFT_SUCCESS;
          break;
        case ATTACK_STATUS_ERROR:
          statusColor = TFT_ERROR;
          break;
        default:
          statusColor = TFT_TEXT;
          break;
      }

      tft->setTextColor(statusColor);
      tft->print(attack->getStatusString());
      tft->setTextColor(TFT_TEXT);

      y += 20;
    }
  }

  // Draw network statistics
  tft->setCursor(10, y + 10);
  tft->print("Network Statistics:");
  y += 30;

  tft->setCursor(20, y);
  tft->print("WiFi Networks: ");
  tft->print(networkScanner->getWiFiNetworks().size());
  y += 20;

  tft->setCursor(20, y);
  tft->print("MQTT Brokers: ");
  tft->print(networkScanner->getMQTTBrokers().size());
  y += 20;

  #ifdef ENABLE_BLUETOOTH
  tft->setCursor(20, y);
  tft->print("BLE Devices: ");
  tft->print(networkScanner->getBLEDevices().size());
  y += 20;
  #endif

  // Draw footer
  drawFooter("", "Back");
}

void TFTInterface::drawAbout() {
  // Draw header
  drawHeader("About SECoT");

  // Draw about information
  tft->setTextColor(TFT_TEXT);

  int y = MENU_HEADER_HEIGHT + 20;

  tft->setCursor(10, y);
  tft->print("SECoT - Security Evaluation and");
  y += 15;

  tft->setCursor(10, y);
  tft->print("Compromise Toolkit");
  y += 30;

  tft->setCursor(10, y);
  tft->print("Version: 1.0.0");
  y += 20;

  tft->setCursor(10, y);
  tft->print("ESP32-WROOM-32D");
  y += 30;

  tft->setCursor(10, y);
  tft->print("For educational purposes only");
  y += 15;

  tft->setCursor(10, y);
  tft->print("Use responsibly and legally");

  // Draw footer
  drawFooter("", "Back");
}

void TFTInterface::handleButton(uint8_t button) {
  // Update button state
  buttonState[button] = true;
}

void TFTInterface::checkButtons() {
  // Check for button presses
  for (int i = 0; i < 6; i++) {
    if (buttonState[i] && !lastButtonState[i]) {
      // Button pressed
      switch (currentScreen) {
        case SCREEN_MAIN_MENU:
        case SCREEN_ATTACK_MENU:
        case SCREEN_SCAN_MENU:
          // Menu navigation
          switch (i) {
            case BUTTON_UP:
              if (selectedMenuItem > 0) {
                selectedMenuItem--;
                if (selectedMenuItem < menuScrollOffset) {
                  menuScrollOffset = selectedMenuItem;
                }
                needsRedraw = true;
              }
              break;
            case BUTTON_DOWN:
              if (selectedMenuItem < getCurrentMenuItems().size() - 1) {
                selectedMenuItem++;
                if (selectedMenuItem >= menuScrollOffset + MENU_ITEMS_PER_SCREEN) {
                  menuScrollOffset = selectedMenuItem - MENU_ITEMS_PER_SCREEN + 1;
                }
                needsRedraw = true;
              }
              break;
            case BUTTON_SELECT:
              selectMenuItem(selectedMenuItem);
              needsRedraw = true;
              break;
            case BUTTON_BACK:
              navigateToParentMenu();
              needsRedraw = true;
              break;
          }
          break;

        case SCREEN_ATTACK_CONFIG:
          // Attack configuration
          switch (i) {
            case BUTTON_SELECT:
              startSelectedAttack();
              needsRedraw = true;
              break;
            case BUTTON_BACK:
              currentScreen = SCREEN_ATTACK_MENU;
              needsRedraw = true;
              break;
          }
          break;

        case SCREEN_ATTACK_RUNNING:
          // Running attack
          switch (i) {
            case BUTTON_SELECT:
              // Toggle attack state
              Attack* attack = attackManager->getAttack(selectedAttackType);
              if (attack) {
                if (attack->getStatus() == ATTACK_STATUS_RUNNING) {
                  stopSelectedAttack();
                } else {
                  startSelectedAttack();
                }
                needsRedraw = true;
              }
              break;
            case BUTTON_BACK:
              // Stop attack and go back
              stopSelectedAttack();
              currentScreen = SCREEN_ATTACK_MENU;
              needsRedraw = true;
              break;
          }
          break;

        case SCREEN_SCAN_RESULTS:
          // Scan results
          switch (i) {
            case BUTTON_UP:
              if (selectedMenuItem > 0) {
                selectedMenuItem--;
                if (selectedMenuItem < menuScrollOffset) {
                  menuScrollOffset = selectedMenuItem;
                }
                needsRedraw = true;
              }
              break;
            case BUTTON_DOWN:
              if (selectedMenuItem < networkScanner->getWiFiNetworks().size() - 1) {
                selectedMenuItem++;
                if (selectedMenuItem >= menuScrollOffset + MENU_ITEMS_PER_SCREEN) {
                  menuScrollOffset = selectedMenuItem - MENU_ITEMS_PER_SCREEN + 1;
                }
                needsRedraw = true;
              }
              break;
            case BUTTON_SELECT:
              // View device details
              if (currentMenuId == 20 && selectedMenuItem < networkScanner->getWiFiNetworks().size()) {
                // WiFi network details
                WiFiNetwork network = networkScanner->getWiFiNetworks()[selectedMenuItem];
                char macStr[18];
                snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                         network.bssid[0], network.bssid[1], network.bssid[2],
                         network.bssid[3], network.bssid[4], network.bssid[5]);
                selectedDeviceMAC = String(macStr);
                currentScreen = SCREEN_DEVICE_INFO;
                needsRedraw = true;
              }
              break;
            case BUTTON_BACK:
              currentScreen = SCREEN_SCAN_MENU;
              needsRedraw = true;
              break;
          }
          break;

        case SCREEN_DEVICE_INFO:
        case SCREEN_STATS:
        case SCREEN_ABOUT:
          // Information screens
          if (i == BUTTON_BACK) {
            navigateToParentMenu();
            needsRedraw = true;
          }
          break;
      }
    }

    // Update last button state
    lastButtonState[i] = buttonState[i];
    buttonState[i] = false;
  }
}

std::vector<MenuItem> TFTInterface::getCurrentMenuItems() {
  std::vector<MenuItem> items;

  for (const MenuItem& item : menuItems) {
    if (item.parentId == currentMenuId) {
      items.push_back(item);
    }
  }

  return items;
}

void TFTInterface::navigateToParentMenu() {
  // Find parent menu ID
  for (const MenuItem& item : menuItems) {
    if (item.id == currentMenuId) {
      currentMenuId = item.parentId;
      selectedMenuItem = 0;
      menuScrollOffset = 0;

      // Update current screen based on menu ID
      if (currentMenuId == 0) {
        currentScreen = SCREEN_MAIN_MENU;
      } else if (currentMenuId == 1) {
        currentScreen = SCREEN_ATTACK_MENU;
      } else if (currentMenuId == 2) {
        currentScreen = SCREEN_SCAN_MENU;
      }

      break;
    }
  }
}

void TFTInterface::navigateToMenu(uint8_t menuId) {
  currentMenuId = menuId;
  selectedMenuItem = 0;
  menuScrollOffset = 0;

  // Update current screen based on menu ID
  if (menuId == 0) {
    currentScreen = SCREEN_MAIN_MENU;
  } else if (menuId == 1) {
    currentScreen = SCREEN_ATTACK_MENU;
  } else if (menuId == 2) {
    currentScreen = SCREEN_SCAN_MENU;
  }
}

void TFTInterface::selectMenuItem(uint8_t index) {
  std::vector<MenuItem> items = getCurrentMenuItems();

  if (index < items.size()) {
    MenuItem item = items[index];

    if (item.isFolder) {
      // Navigate to submenu
      navigateToMenu(item.id);
    } else if (item.isAttack) {
      // Select attack
      selectedAttackType = item.attackType;
      currentScreen = SCREEN_ATTACK_CONFIG;
    } else {
      // Handle special menu items
      if (item.id == 3) {
        // Statistics
        currentScreen = SCREEN_STATS;
      } else if (item.id == 4) {
        // About
        currentScreen = SCREEN_ABOUT;
      } else if (item.id == 20) {
        // WiFi scan
        networkScanner->scanWiFiNetworks();
        currentScreen = SCREEN_SCAN_RESULTS;
      } else if (item.id == 21) {
        // MQTT scan
        if (WiFi.status() == WL_CONNECTED) {
          networkScanner->scanMQTTBrokers("");
          currentScreen = SCREEN_SCAN_RESULTS;
        }
      } else if (item.id == 22) {
        // BLE scan
        #ifdef ENABLE_BLUETOOTH
        networkScanner->scanBLEDevices();
        currentScreen = SCREEN_SCAN_RESULTS;
        #endif
      }
    }
  }
}

void TFTInterface::startSelectedAttack() {
  Attack* attack = attackManager->getAttack(selectedAttackType);
  if (attack) {
    if (attack->getStatus() != ATTACK_STATUS_RUNNING) {
      // Start the attack
      if (attack->start()) {
        Serial.print("Started attack: ");
        Serial.println(attack->getName());

        // Switch to attack running screen
        currentScreen = SCREEN_ATTACK_RUNNING;
      } else {
        Serial.print("Failed to start attack: ");
        Serial.println(attack->getErrorMessage());
      }
    }
  }
}

void TFTInterface::stopSelectedAttack() {
  Attack* attack = attackManager->getAttack(selectedAttackType);
  if (attack) {
    if (attack->getStatus() == ATTACK_STATUS_RUNNING) {
      // Stop the attack
      if (attack->stop()) {
        Serial.print("Stopped attack: ");
        Serial.println(attack->getName());
      } else {
        Serial.print("Failed to stop attack: ");
        Serial.println(attack->getErrorMessage());
      }
    }
  }
}

void TFTInterface::startSelectedScan() {
  // Determine which scan to start based on current menu ID
  if (currentMenuId == 20) {
    // WiFi scan
    networkScanner->scanWiFiNetworks();
  } else if (currentMenuId == 21) {
    // MQTT scan
    if (WiFi.status() == WL_CONNECTED) {
      networkScanner->scanMQTTBrokers("");
    }
  } else if (currentMenuId == 22) {
    // BLE scan
    #ifdef ENABLE_BLUETOOTH
    networkScanner->scanBLEDevices();
    #endif
  }
}

String TFTInterface::getAttackName(uint8_t type) {
  switch (type) {
    case ATTACK_TYPE_DEAUTH:
      return "WiFi Deauth";
    case ATTACK_TYPE_BEACON_FLOOD:
      return "Beacon Flood";
    case ATTACK_TYPE_PROBE_SPAM:
      return "Probe Spam";
    case ATTACK_TYPE_ARP_SPOOF:
      return "ARP Spoof";
    case ATTACK_TYPE_MQTT_SPOOF:
      return "MQTT Spoof";
    case ATTACK_TYPE_EVIL_TWIN:
      return "Evil Twin";
    case ATTACK_TYPE_PASSIVE_SNIFF:
      return "Passive Sniff";
    default:
      return "Unknown";
  }
}

String TFTInterface::formatMACAddress(const String& mac) {
  if (mac.length() == 17) {
    return mac.substring(0, 8) + "...";
  }
  return mac;
}
