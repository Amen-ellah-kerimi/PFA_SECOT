/*
 * Attack.h
 * 
 * Base class for all attack modules
 */

#ifndef ATTACK_H
#define ATTACK_H

#include <Arduino.h>
#include "../config/Config.h"

// Attack types
#define ATTACK_TYPE_DEAUTH 0
#define ATTACK_TYPE_BEACON_FLOOD 1
#define ATTACK_TYPE_PROBE_SPAM 2
#define ATTACK_TYPE_ARP_SPOOF 3
#define ATTACK_TYPE_MQTT_SPOOF 4
#define ATTACK_TYPE_EVIL_TWIN 5
#define ATTACK_TYPE_PASSIVE_SNIFF 6
#define ATTACK_TYPE_BLUETOOTH_SCAN 7
#define ATTACK_TYPE_JAMMING 8

// Attack status
#define ATTACK_STATUS_IDLE 0
#define ATTACK_STATUS_RUNNING 1
#define ATTACK_STATUS_STOPPING 2
#define ATTACK_STATUS_ERROR 3

class Attack {
  public:
    Attack(uint8_t type, const String& name);
    virtual ~Attack();
    
    // Get attack information
    uint8_t getType() const;
    String getName() const;
    uint8_t getStatus() const;
    String getStatusString() const;
    String getErrorMessage() const;
    
    // Start the attack with optional duration (0 = run until stopped)
    virtual bool start(unsigned long duration = DEFAULT_ATTACK_DURATION);
    
    // Stop the attack
    virtual bool stop();
    
    // Update the attack (called in main loop)
    virtual void update();
    
    // Set attack parameters
    virtual bool setParameter(const String& name, const String& value);
    
    // Get attack parameters
    virtual String getParameter(const String& name) const;
    
    // Get all parameters as a JSON string
    virtual String getAllParameters() const;
    
  protected:
    uint8_t type;
    String name;
    uint8_t status;
    String errorMessage;
    
    unsigned long startTime;
    unsigned long duration;
    unsigned long lastActionTime;
    
    // Convert status to string
    String statusToString(uint8_t status) const;
};

#endif // ATTACK_H
