/*
 * ProbeSpamAttack.h
 * 
 * WiFi Probe Request Spamming Attack module
 * Floods the area with probe requests for different networks
 */

#ifndef PROBE_SPAM_ATTACK_H
#define PROBE_SPAM_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "esp_wifi_internal.h"
#include <vector>
#include "Attack.h"

// Maximum number of SSIDs to probe
#define MAX_PROBE_SSIDS 50

// Probe request frame constants
#define PROBE_PACKET_SIZE 68
#define PROBE_SSID_MAX_LENGTH 32

class ProbeSpamAttack : public Attack {
  public:
    ProbeSpamAttack();
    virtual ~ProbeSpamAttack();
    
    // Override base class methods
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION) override;
    virtual bool stop() override;
    virtual void update() override;
    virtual bool setParameter(const String& name, const String& value) override;
    virtual String getParameter(const String& name) const override;
    virtual String getAllParameters() const override;
    
  private:
    // Attack parameters
    std::vector<String> ssidList;
    bool randomSSIDs;
    int ssidCount;
    String ssidPrefix;
    bool useRandomMAC;
    uint8_t startChannel;
    uint8_t endChannel;
    unsigned long probeInterval;
    
    // State variables
    uint8_t currentChannel;
    int currentSSIDIndex;
    
    // Probe request frame template
    uint8_t probePacket[PROBE_PACKET_SIZE] = {
      /* IEEE 802.11 Probe Request frame */
      /* Header */
      0x40, 0x00,                         // Frame Control (Type: Management, Subtype: Probe Request)
      0x00, 0x00,                         // Duration
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source: Client MAC (will be randomized)
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // BSSID: Broadcast
      0x00, 0x00,                         // Sequence number (will be filled by hardware)
      
      /* Management Frame Body */
      /* Tagged parameters */
      /* SSID parameter */
      0x00,                               // Element ID: SSID
      0x00,                               // Length (will be filled)
      /* SSID goes here */
      
      /* Supported rates parameter */
      0x01,                               // Element ID: Supported rates
      0x08,                               // Length
      0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, // Rates
      
      /* Extended supported rates parameter */
      0x32,                               // Element ID: Extended supported rates
      0x04,                               // Length
      0x30, 0x48, 0x60, 0x6c,             // Extended rates
      
      /* HT capabilities parameter */
      0x2d,                               // Element ID: HT capabilities
      0x1a,                               // Length
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HT capabilities
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    // Helper methods
    void generateRandomSSIDs();
    void sendProbeRequest(const String& ssid, uint8_t channel);
    void generateRandomMAC(uint8_t* mac);
    void printMACAddress(const uint8_t* mac);
};

#endif // PROBE_SPAM_ATTACK_H
