/*
 * PassiveSniffAttack.h
 * 
 * Passive WiFi Sniffing Attack module
 * Captures and analyzes WiFi packets
 */

#ifndef PASSIVE_SNIFF_ATTACK_H
#define PASSIVE_SNIFF_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <vector>
#include <map>
#include "Attack.h"

// Maximum number of devices to track
#define MAX_SNIFF_DEVICES 100

// Maximum number of packets to store
#define MAX_PACKET_HISTORY 50

// Packet types
#define PACKET_TYPE_MANAGEMENT 0
#define PACKET_TYPE_CONTROL 1
#define PACKET_TYPE_DATA 2

// Management frame subtypes
#define MGMT_SUBTYPE_ASSOC_REQ 0
#define MGMT_SUBTYPE_ASSOC_RESP 1
#define MGMT_SUBTYPE_REASSOC_REQ 2
#define MGMT_SUBTYPE_REASSOC_RESP 3
#define MGMT_SUBTYPE_PROBE_REQ 4
#define MGMT_SUBTYPE_PROBE_RESP 5
#define MGMT_SUBTYPE_BEACON 8
#define MGMT_SUBTYPE_ATIM 9
#define MGMT_SUBTYPE_DISASSOC 10
#define MGMT_SUBTYPE_AUTH 11
#define MGMT_SUBTYPE_DEAUTH 12

// Packet structure
struct SniffedPacket {
  uint8_t type;
  uint8_t subtype;
  uint8_t srcMac[6];
  uint8_t dstMac[6];
  int rssi;
  uint8_t channel;
  unsigned long timestamp;
  size_t length;
};

// Device structure
struct SniffedDevice {
  uint8_t mac[6];
  String ssid;
  int rssi;
  uint8_t channel;
  unsigned long lastSeen;
  uint32_t packetCount;
  bool isAP;
};

class PassiveSniffAttack : public Attack {
  public:
    PassiveSniffAttack();
    virtual ~PassiveSniffAttack();
    
    // Override base class methods
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION) override;
    virtual bool stop() override;
    virtual void update() override;
    virtual bool setParameter(const String& name, const String& value) override;
    virtual String getParameter(const String& name) const override;
    virtual String getAllParameters() const override;
    
    // Static callback for packet processing
    static void IRAM_ATTR wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
    
    // Process a captured packet
    void processPacket(const wifi_promiscuous_pkt_t* packet, wifi_promiscuous_pkt_type_t type);
    
  private:
    // Attack parameters
    uint8_t startChannel;
    uint8_t endChannel;
    unsigned long channelHopInterval;
    bool captureBeacons;
    bool captureProbes;
    bool captureData;
    bool captureManagement;
    bool captureControl;
    String filterBSSID;
    String filterSSID;
    
    // State variables
    uint8_t currentChannel;
    unsigned long lastChannelHop;
    
    // Packet and device storage
    static std::vector<SniffedPacket> packetHistory;
    static std::map<String, SniffedDevice> devices;
    static PassiveSniffAttack* instance;
    
    // Statistics
    uint32_t totalPackets;
    uint32_t beaconCount;
    uint32_t probeReqCount;
    uint32_t probeRespCount;
    uint32_t deauthCount;
    uint32_t dataCount;
    
    // Helper methods
    void hopChannel();
    bool shouldCapturePacket(uint8_t type, uint8_t subtype);
    void addDevice(const uint8_t* mac, const String& ssid, int rssi, uint8_t channel, bool isAP);
    String getMACAddressString(const uint8_t* mac) const;
    String getPacketTypeString(uint8_t type, uint8_t subtype) const;
    String getDeviceList() const;
    String getPacketList() const;
    String getStatistics() const;
};

#endif // PASSIVE_SNIFF_ATTACK_H
