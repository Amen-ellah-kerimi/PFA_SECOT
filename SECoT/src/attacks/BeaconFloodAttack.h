/*
 * BeaconFloodAttack.h
 * 
 * WiFi Beacon Flooding Attack module
 * Floods the 2.4GHz spectrum with fake WiFi networks
 */

#ifndef BEACON_FLOOD_ATTACK_H
#define BEACON_FLOOD_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "esp_wifi_internal.h"
#include <vector>
#include "Attack.h"

// Maximum number of SSIDs to generate
#define MAX_BEACON_SSIDS 50

// Beacon frame constants
#define BEACON_PACKET_SIZE 128
#define BEACON_SSID_MAX_LENGTH 32

class BeaconFloodAttack : public Attack {
  public:
    BeaconFloodAttack();
    virtual ~BeaconFloodAttack();
    
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
    unsigned long beaconInterval;
    bool encryptionEnabled;
    
    // State variables
    uint8_t currentChannel;
    int currentSSIDIndex;
    
    // Beacon frame template
    uint8_t beaconPacket[BEACON_PACKET_SIZE] = {
      /* IEEE 802.11 Beacon frame */
      /* Header */
      0x80, 0x00,                         // Frame Control (Type: Management, Subtype: Beacon)
      0x00, 0x00,                         // Duration
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source: AP MAC (will be randomized)
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID: AP MAC (same as source)
      0x00, 0x00,                         // Sequence number (will be filled by hardware)
      
      /* Management Frame Body */
      /* Fixed parameters */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp (8 bytes)
      0x64, 0x00,                         // Beacon interval (100 TU = 102.4 ms)
      0x01, 0x00,                         // Capability info (ESS)
      
      /* Tagged parameters */
      /* SSID parameter */
      0x00,                               // Element ID: SSID
      0x00,                               // Length (will be filled)
      /* SSID goes here */
      
      /* Supported rates parameter */
      0x01,                               // Element ID: Supported rates
      0x08,                               // Length
      0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24, // Rates
      
      /* Channel parameter */
      0x03,                               // Element ID: Channel
      0x01,                               // Length
      0x01,                               // Channel (will be filled)
      
      /* RSN parameter (for WPA2) - only included if encryption is enabled */
      0x30,                               // Element ID: RSN
      0x14,                               // Length
      0x01, 0x00,                         // Version
      0x00, 0x0f, 0xac, 0x04,             // Group cipher: CCMP
      0x01, 0x00,                         // Pairwise cipher count
      0x00, 0x0f, 0xac, 0x04,             // Pairwise cipher: CCMP
      0x01, 0x00,                         // Auth key management count
      0x00, 0x0f, 0xac, 0x02,             // Auth key management: PSK
      0x00, 0x00                          // RSN capabilities
    };
    
    // Helper methods
    void generateRandomSSIDs();
    void sendBeacon(const String& ssid, uint8_t channel);
    void generateRandomMAC(uint8_t* mac);
    void printMACAddress(const uint8_t* mac);
};

#endif // BEACON_FLOOD_ATTACK_H
