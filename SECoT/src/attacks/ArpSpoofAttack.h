/*
 * ArpSpoofAttack.h
 * 
 * ARP Spoofing Attack module
 */

#ifndef ARP_SPOOF_ATTACK_H
#define ARP_SPOOF_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/ip4.h>
#include "Attack.h"

// Attack modes
#define ARP_ATTACK_MODE_GATEWAY 1
#define ARP_ATTACK_MODE_BROKER 2
#define ARP_ATTACK_MODE_BOTH 3

class ArpSpoofAttack : public Attack {
  public:
    ArpSpoofAttack();
    virtual ~ArpSpoofAttack();
    
    // Override base class methods
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION) override;
    virtual bool stop() override;
    virtual void update() override;
    virtual bool setParameter(const String& name, const String& value) override;
    virtual String getParameter(const String& name) const override;
    virtual String getAllParameters() const override;
    
  private:
    // WiFi credentials
    String ssid;
    String password;
    
    // Target IP addresses
    String targetDeviceIp;
    String gatewayIp;
    String mqttBrokerIp;
    
    // Attack parameters
    uint8_t attackMode;
    unsigned long attackInterval;
    
    // MAC addresses
    uint8_t targetDeviceMac[6];
    uint8_t gatewayMac[6];
    uint8_t mqttBrokerMac[6];
    uint8_t myMac[6];
    
    // State variables
    bool targetFound;
    bool gatewayFound;
    bool brokerFound;
    
    // Helper methods
    void setupWiFi();
    void scanNetwork();
    bool getMacFromIp(const String& ipStr, uint8_t* mac);
    void sendFakeArp(uint8_t* targetMac, const String& spoofIp);
    void printMacAddress(const uint8_t* mac);
    bool macAddressFromString(const String& macStr, uint8_t* mac);
    bool ipAddressFromString(const String& ipStr, ip4_addr_t* ip);
};

#endif // ARP_SPOOF_ATTACK_H
