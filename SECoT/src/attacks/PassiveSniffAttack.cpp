/*
 * PassiveSniffAttack.cpp
 * 
 * Implementation of the Passive WiFi Sniffing Attack module
 */

#include "PassiveSniffAttack.h"
#include <WiFi.h>

// Initialize static members
std::vector<SniffedPacket> PassiveSniffAttack::packetHistory;
std::map<String, SniffedDevice> PassiveSniffAttack::devices;
PassiveSniffAttack* PassiveSniffAttack::instance = nullptr;

PassiveSniffAttack::PassiveSniffAttack() : Attack(ATTACK_TYPE_PASSIVE_SNIFF, "Passive Sniffing") {
  // Initialize parameters with defaults
  startChannel = 1;
  endChannel = 11;
  channelHopInterval = 500; // ms
  captureBeacons = true;
  captureProbes = true;
  captureData = true;
  captureManagement = true;
  captureControl = false;
  filterBSSID = "";
  filterSSID = "";
  
  // Initialize state variables
  currentChannel = startChannel;
  lastChannelHop = 0;
  
  // Initialize statistics
  totalPackets = 0;
  beaconCount = 0;
  probeReqCount = 0;
  probeRespCount = 0;
  deauthCount = 0;
  dataCount = 0;
  
  // Set instance pointer for static callback
  instance = this;
}

PassiveSniffAttack::~PassiveSniffAttack() {
  // Clean up
  stop();
  instance = nullptr;
}

bool PassiveSniffAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Clear packet history and device list
  packetHistory.clear();
  devices.clear();
  
  // Reset statistics
  totalPackets = 0;
  beaconCount = 0;
  probeReqCount = 0;
  probeRespCount = 0;
  deauthCount = 0;
  dataCount = 0;
  
  // Initialize WiFi in station mode
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  
  // Set initial channel
  currentChannel = startChannel;
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  
  // Set promiscuous mode callback
  esp_wifi_set_promiscuous_rx_cb(&wifiSnifferCallback);
  
  // Enable promiscuous mode
  esp_wifi_set_promiscuous(true);
  
  Serial.println("Passive Sniffing Attack started");
  Serial.print("Scanning channels ");
  Serial.print(startChannel);
  Serial.print(" to ");
  Serial.println(endChannel);
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  lastChannelHop = millis();
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool PassiveSniffAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disable promiscuous mode
  esp_wifi_set_promiscuous(false);
  
  // Restore WiFi
  WiFi.disconnect();
  
  Serial.println("Passive Sniffing Attack stopped");
  Serial.print("Total packets captured: ");
  Serial.println(totalPackets);
  Serial.print("Unique devices found: ");
  Serial.println(devices.size());
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void PassiveSniffAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    // Channel hopping
    unsigned long currentMillis = millis();
    if (currentMillis - lastChannelHop >= channelHopInterval) {
      hopChannel();
      lastChannelHop = currentMillis;
    }
  }
}

bool PassiveSniffAttack::setParameter(const String& name, const String& value) {
  if (name == "startchannel") {
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
  else if (name == "hopinterval") {
    int interval = value.toInt();
    if (interval >= 100) {
      channelHopInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 100ms)";
      return false;
    }
  }
  else if (name == "beacons") {
    if (value == "true" || value == "1") {
      captureBeacons = true;
      return true;
    } else if (value == "false" || value == "0") {
      captureBeacons = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "probes") {
    if (value == "true" || value == "1") {
      captureProbes = true;
      return true;
    } else if (value == "false" || value == "0") {
      captureProbes = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "data") {
    if (value == "true" || value == "1") {
      captureData = true;
      return true;
    } else if (value == "false" || value == "0") {
      captureData = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "management") {
    if (value == "true" || value == "1") {
      captureManagement = true;
      return true;
    } else if (value == "false" || value == "0") {
      captureManagement = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "control") {
    if (value == "true" || value == "1") {
      captureControl = true;
      return true;
    } else if (value == "false" || value == "0") {
      captureControl = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "filterbssid") {
    filterBSSID = value;
    return true;
  }
  else if (name == "filterssid") {
    filterSSID = value;
    return true;
  }
  else if (name == "clear") {
    packetHistory.clear();
    devices.clear();
    totalPackets = 0;
    beaconCount = 0;
    probeReqCount = 0;
    probeRespCount = 0;
    deauthCount = 0;
    dataCount = 0;
    return true;
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String PassiveSniffAttack::getParameter(const String& name) const {
  if (name == "startchannel") {
    return String(startChannel);
  }
  else if (name == "endchannel") {
    return String(endChannel);
  }
  else if (name == "hopinterval") {
    return String(channelHopInterval);
  }
  else if (name == "beacons") {
    return captureBeacons ? "true" : "false";
  }
  else if (name == "probes") {
    return captureProbes ? "true" : "false";
  }
  else if (name == "data") {
    return captureData ? "true" : "false";
  }
  else if (name == "management") {
    return captureManagement ? "true" : "false";
  }
  else if (name == "control") {
    return captureControl ? "true" : "false";
  }
  else if (name == "filterbssid") {
    return filterBSSID;
  }
  else if (name == "filterssid") {
    return filterSSID;
  }
  else if (name == "devices") {
    return getDeviceList();
  }
  else if (name == "packets") {
    return getPacketList();
  }
  else if (name == "stats") {
    return getStatistics();
  }
  else if (name == "channel") {
    return String(currentChannel);
  }
  
  return "";
}

String PassiveSniffAttack::getAllParameters() const {
  String params = "{";
  params += "\"startchannel\":" + String(startChannel) + ",";
  params += "\"endchannel\":" + String(endChannel) + ",";
  params += "\"hopinterval\":" + String(channelHopInterval) + ",";
  params += "\"beacons\":" + String(captureBeacons ? "true" : "false") + ",";
  params += "\"probes\":" + String(captureProbes ? "true" : "false") + ",";
  params += "\"data\":" + String(captureData ? "true" : "false") + ",";
  params += "\"management\":" + String(captureManagement ? "true" : "false") + ",";
  params += "\"control\":" + String(captureControl ? "true" : "false") + ",";
  params += "\"filterbssid\":\"" + filterBSSID + "\",";
  params += "\"filterssid\":\"" + filterSSID + "\",";
  params += "\"channel\":" + String(currentChannel) + ",";
  params += "\"total_packets\":" + String(totalPackets) + ",";
  params += "\"device_count\":" + String(devices.size()) + ",";
  params += "\"packet_history_count\":" + String(packetHistory.size());
  params += "}";
  
  return params;
}

void IRAM_ATTR PassiveSniffAttack::wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  // Check if instance exists
  if (!instance) {
    return;
  }
  
  // Process the packet
  instance->processPacket((wifi_promiscuous_pkt_t*)buf, type);
}

void PassiveSniffAttack::processPacket(const wifi_promiscuous_pkt_t* packet, wifi_promiscuous_pkt_type_t type) {
  // Increment total packet count
  totalPackets++;
  
  // Get packet header
  const wifi_ieee80211_packet_t* ieee80211 = (wifi_ieee80211_packet_t*)packet->payload;
  const wifi_ieee80211_mac_hdr_t* hdr = &ieee80211->hdr;
  
  // Extract packet type and subtype
  uint8_t frameControl = hdr->frame_ctrl;
  uint8_t packetType = (frameControl & 0x0C) >> 2;
  uint8_t packetSubtype = (frameControl & 0xF0) >> 4;
  
  // Check if we should capture this packet type
  if (!shouldCapturePacket(packetType, packetSubtype)) {
    return;
  }
  
  // Create packet record
  SniffedPacket sniffedPacket;
  sniffedPacket.type = packetType;
  sniffedPacket.subtype = packetSubtype;
  memcpy(sniffedPacket.srcMac, hdr->addr2, 6);
  memcpy(sniffedPacket.dstMac, hdr->addr1, 6);
  sniffedPacket.rssi = packet->rx_ctrl.rssi;
  sniffedPacket.channel = packet->rx_ctrl.channel;
  sniffedPacket.timestamp = millis();
  sniffedPacket.length = packet->rx_ctrl.sig_len;
  
  // Update packet type counts
  if (packetType == PACKET_TYPE_MANAGEMENT) {
    if (packetSubtype == MGMT_SUBTYPE_BEACON) {
      beaconCount++;
      
      // Extract SSID from beacon
      const uint8_t* tagPtr = packet->payload + sizeof(wifi_ieee80211_mac_hdr_t) + 12;
      uint8_t tagId = tagPtr[0];
      uint8_t tagLen = tagPtr[1];
      
      if (tagId == 0 && tagLen > 0 && tagLen <= 32) {
        char ssid[33] = {0};
        memcpy(ssid, tagPtr + 2, tagLen);
        
        // Add or update device
        addDevice(hdr->addr2, String(ssid), packet->rx_ctrl.rssi, packet->rx_ctrl.channel, true);
      }
    }
    else if (packetSubtype == MGMT_SUBTYPE_PROBE_REQ) {
      probeReqCount++;
      
      // Add or update device (client)
      addDevice(hdr->addr2, "", packet->rx_ctrl.rssi, packet->rx_ctrl.channel, false);
    }
    else if (packetSubtype == MGMT_SUBTYPE_PROBE_RESP) {
      probeRespCount++;
    }
    else if (packetSubtype == MGMT_SUBTYPE_DEAUTH) {
      deauthCount++;
    }
  }
  else if (packetType == PACKET_TYPE_DATA) {
    dataCount++;
    
    // Add or update devices
    addDevice(hdr->addr1, "", packet->rx_ctrl.rssi, packet->rx_ctrl.channel, false);
    addDevice(hdr->addr2, "", packet->rx_ctrl.rssi, packet->rx_ctrl.channel, false);
  }
  
  // Add to packet history
  if (packetHistory.size() >= MAX_PACKET_HISTORY) {
    packetHistory.erase(packetHistory.begin());
  }
  packetHistory.push_back(sniffedPacket);
}

void PassiveSniffAttack::hopChannel() {
  // Move to next channel
  currentChannel++;
  if (currentChannel > endChannel) {
    currentChannel = startChannel;
  }
  
  // Set the new channel
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  
  // Debug output
  Serial.print("Switched to channel ");
  Serial.println(currentChannel);
}

bool PassiveSniffAttack::shouldCapturePacket(uint8_t type, uint8_t subtype) {
  // Check packet type
  if (type == PACKET_TYPE_MANAGEMENT) {
    if (!captureManagement) {
      return false;
    }
    
    // Check specific management frame subtypes
    if (subtype == MGMT_SUBTYPE_BEACON && !captureBeacons) {
      return false;
    }
    
    if ((subtype == MGMT_SUBTYPE_PROBE_REQ || subtype == MGMT_SUBTYPE_PROBE_RESP) && !captureProbes) {
      return false;
    }
  }
  else if (type == PACKET_TYPE_CONTROL && !captureControl) {
    return false;
  }
  else if (type == PACKET_TYPE_DATA && !captureData) {
    return false;
  }
  
  return true;
}

void PassiveSniffAttack::addDevice(const uint8_t* mac, const String& ssid, int rssi, uint8_t channel, bool isAP) {
  // Create MAC address string
  String macStr = getMACAddressString(mac);
  
  // Check if device already exists
  auto it = devices.find(macStr);
  if (it != devices.end()) {
    // Update existing device
    it->second.rssi = rssi;
    it->second.channel = channel;
    it->second.lastSeen = millis();
    it->second.packetCount++;
    
    // Update SSID if provided and device is an AP
    if (isAP && ssid.length() > 0) {
      it->second.ssid = ssid;
      it->second.isAP = true;
    }
  } else {
    // Add new device if we haven't reached the limit
    if (devices.size() < MAX_SNIFF_DEVICES) {
      SniffedDevice device;
      memcpy(device.mac, mac, 6);
      device.ssid = isAP ? ssid : "";
      device.rssi = rssi;
      device.channel = channel;
      device.lastSeen = millis();
      device.packetCount = 1;
      device.isAP = isAP;
      
      devices[macStr] = device;
    }
  }
}

String PassiveSniffAttack::getMACAddressString(const uint8_t* mac) const {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

String PassiveSniffAttack::getPacketTypeString(uint8_t type, uint8_t subtype) const {
  if (type == PACKET_TYPE_MANAGEMENT) {
    switch (subtype) {
      case MGMT_SUBTYPE_ASSOC_REQ: return "Association Request";
      case MGMT_SUBTYPE_ASSOC_RESP: return "Association Response";
      case MGMT_SUBTYPE_REASSOC_REQ: return "Reassociation Request";
      case MGMT_SUBTYPE_REASSOC_RESP: return "Reassociation Response";
      case MGMT_SUBTYPE_PROBE_REQ: return "Probe Request";
      case MGMT_SUBTYPE_PROBE_RESP: return "Probe Response";
      case MGMT_SUBTYPE_BEACON: return "Beacon";
      case MGMT_SUBTYPE_ATIM: return "ATIM";
      case MGMT_SUBTYPE_DISASSOC: return "Disassociation";
      case MGMT_SUBTYPE_AUTH: return "Authentication";
      case MGMT_SUBTYPE_DEAUTH: return "Deauthentication";
      default: return "Management";
    }
  } else if (type == PACKET_TYPE_CONTROL) {
    return "Control";
  } else if (type == PACKET_TYPE_DATA) {
    return "Data";
  } else {
    return "Unknown";
  }
}

String PassiveSniffAttack::getDeviceList() const {
  String result = "[";
  
  int count = 0;
  for (const auto& pair : devices) {
    const SniffedDevice& device = pair.second;
    
    // Apply BSSID filter if set
    if (filterBSSID.length() > 0 && pair.first != filterBSSID) {
      continue;
    }
    
    // Apply SSID filter if set
    if (filterSSID.length() > 0 && device.ssid != filterSSID) {
      continue;
    }
    
    if (count > 0) {
      result += ",";
    }
    
    result += "{";
    result += "\"mac\":\"" + pair.first + "\",";
    result += "\"ssid\":\"" + device.ssid + "\",";
    result += "\"rssi\":" + String(device.rssi) + ",";
    result += "\"channel\":" + String(device.channel) + ",";
    result += "\"last_seen\":" + String((millis() - device.lastSeen) / 1000) + ",";
    result += "\"packet_count\":" + String(device.packetCount) + ",";
    result += "\"is_ap\":" + String(device.isAP ? "true" : "false");
    result += "}";
    
    count++;
  }
  
  result += "]";
  
  return result;
}

String PassiveSniffAttack::getPacketList() const {
  String result = "[";
  
  for (size_t i = 0; i < packetHistory.size(); i++) {
    const SniffedPacket& packet = packetHistory[i];
    
    if (i > 0) {
      result += ",";
    }
    
    result += "{";
    result += "\"type\":\"" + getPacketTypeString(packet.type, packet.subtype) + "\",";
    result += "\"src\":\"" + getMACAddressString(packet.srcMac) + "\",";
    result += "\"dst\":\"" + getMACAddressString(packet.dstMac) + "\",";
    result += "\"rssi\":" + String(packet.rssi) + ",";
    result += "\"channel\":" + String(packet.channel) + ",";
    result += "\"time\":" + String((millis() - packet.timestamp) / 1000) + ",";
    result += "\"length\":" + String(packet.length);
    result += "}";
  }
  
  result += "]";
  
  return result;
}

String PassiveSniffAttack::getStatistics() const {
  String result = "{";
  result += "\"total_packets\":" + String(totalPackets) + ",";
  result += "\"beacon_count\":" + String(beaconCount) + ",";
  result += "\"probe_req_count\":" + String(probeReqCount) + ",";
  result += "\"probe_resp_count\":" + String(probeRespCount) + ",";
  result += "\"deauth_count\":" + String(deauthCount) + ",";
  result += "\"data_count\":" + String(dataCount) + ",";
  result += "\"device_count\":" + String(devices.size()) + ",";
  result += "\"ap_count\":" + String(std::count_if(devices.begin(), devices.end(), 
                                                  [](const auto& pair) { return pair.second.isAP; }));
  result += "}";
  
  return result;
}
