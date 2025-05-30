/*
 * EvilTwinAttack.h
 * 
 * Evil Twin Attack module
 * Creates a fake access point that mimics a legitimate one
 */

#ifndef EVIL_TWIN_ATTACK_H
#define EVIL_TWIN_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <vector>
#include "Attack.h"

// Maximum number of clients to track
#define MAX_EVIL_TWIN_CLIENTS 10

// Client structure
struct EvilTwinClient {
  IPAddress ip;
  uint8_t mac[6];
  String hostname;
  unsigned long connectTime;
  bool authenticated;
  String username;
  String password;
};

class EvilTwinAttack : public Attack {
  public:
    EvilTwinAttack();
    virtual ~EvilTwinAttack();
    
    // Override base class methods
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION) override;
    virtual bool stop() override;
    virtual void update() override;
    virtual bool setParameter(const String& name, const String& value) override;
    virtual String getParameter(const String& name) const override;
    virtual String getAllParameters() const override;
    
  private:
    // Attack parameters
    String targetSSID;
    String apPassword;
    bool openNetwork;
    IPAddress apIP;
    IPAddress netMask;
    uint8_t channel;
    int maxClients;
    bool captivePortal;
    String portalTitle;
    String portalSubtitle;
    String redirectURL;
    
    // State variables
    std::vector<EvilTwinClient> clients;
    bool apActive;
    
    // DNS and web server
    DNSServer* dnsServer;
    WebServer* webServer;
    
    // Helper methods
    bool scanForTargetAP();
    void setupAP();
    void setupDNS();
    void setupWebServer();
    void handleRoot();
    void handleLogin();
    void handleNotFound();
    void printMACAddress(const uint8_t* mac);
    String getMACAddressString(const uint8_t* mac);
    void addClient(WiFiClient client);
    void removeClient(IPAddress ip);
    EvilTwinClient* findClient(IPAddress ip);
    String getClientList();
    String getHTMLHeader();
    String getHTMLFooter();
    String getLoginForm();
    String urlDecode(String input);
};

#endif // EVIL_TWIN_ATTACK_H
