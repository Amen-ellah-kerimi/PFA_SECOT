/*
 * TFTInterface.h
 * 
 * TFT display interface for SECoT
 * Provides a graphical menu system and visual feedback for attacks
 */

#ifndef TFT_INTERFACE_H
#define TFT_INTERFACE_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "../attacks/AttackManager.h"
#include "../utils/NetworkScanner.h"
#include "../config/Config.h"

// Menu item structure
struct MenuItem {
  String name;
  uint8_t id;
  uint8_t parentId;
  bool isFolder;
  bool isAttack;
  uint8_t attackType;
};

// Screen states
#define SCREEN_MAIN_MENU 0
#define SCREEN_ATTACK_MENU 1
#define SCREEN_SCAN_MENU 2
#define SCREEN_ATTACK_RUNNING 3
#define SCREEN_ATTACK_CONFIG 4
#define SCREEN_SCAN_RESULTS 5
#define SCREEN_DEVICE_INFO 6
#define SCREEN_STATS 7
#define SCREEN_ABOUT 8

// Button definitions
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 3
#define BUTTON_SELECT 4
#define BUTTON_BACK 5

// UI constants
#define MENU_ITEMS_PER_SCREEN 6
#define MENU_ITEM_HEIGHT 20
#define MENU_ITEM_PADDING 5
#define MENU_HEADER_HEIGHT 30
#define MENU_FOOTER_HEIGHT 20

class TFTInterface {
  public:
    TFTInterface();
    
    // Initialize the interface
    void begin(AttackManager* attackManager, NetworkScanner* networkScanner);
    
    // Update the interface (called in main loop)
    void update();
    
    // Handle button presses
    void handleButton(uint8_t button);
    
  private:
    // References to other components
    AttackManager* attackManager;
    NetworkScanner* networkScanner;
    
    // TFT display
    TFT_eSPI* tft;
    
    // Menu system
    std::vector<MenuItem> menuItems;
    uint8_t currentMenuId;
    uint8_t selectedMenuItem;
    uint8_t menuScrollOffset;
    uint8_t currentScreen;
    
    // Selected attack/device
    uint8_t selectedAttackType;
    String selectedDeviceMAC;
    
    // State variables
    bool needsRedraw;
    unsigned long lastUpdate;
    unsigned long lastButtonCheck;
    
    // Button state
    bool buttonState[6];
    bool lastButtonState[6];
    
    // Helper methods
    void initializeMenu();
    void drawScreen();
    void drawMainMenu();
    void drawAttackMenu();
    void drawScanMenu();
    void drawAttackRunning();
    void drawAttackConfig();
    void drawScanResults();
    void drawDeviceInfo();
    void drawStats();
    void drawAbout();
    
    void drawHeader(const String& title);
    void drawFooter(const String& leftAction, const String& rightAction);
    void drawMenuItem(const MenuItem& item, int y, bool selected);
    void drawProgressBar(int x, int y, int width, int height, int percentage);
    void drawStatusIcon(int x, int y, bool active);
    
    void navigateToParentMenu();
    void navigateToMenu(uint8_t menuId);
    void selectMenuItem(uint8_t index);
    
    void startSelectedAttack();
    void stopSelectedAttack();
    void startSelectedScan();
    
    void checkButtons();
    bool isButtonPressed(uint8_t button);
    
    // Get menu items for current menu
    std::vector<MenuItem> getCurrentMenuItems();
    
    // Get attack name from type
    String getAttackName(uint8_t type);
    
    // Format MAC address for display
    String formatMACAddress(const String& mac);
};

#endif // TFT_INTERFACE_H
