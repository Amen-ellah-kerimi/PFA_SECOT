/*
 * AttackManager.h
 * 
 * Manages all attack modules
 */

#ifndef ATTACK_MANAGER_H
#define ATTACK_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "Attack.h"

// Forward declarations of attack classes
class DeauthAttack;
class BeaconFloodAttack;
class ProbeSpamAttack;
class ArpSpoofAttack;
class MqttSpoofAttack;
class EvilTwinAttack;
class PassiveSniffAttack;
#ifdef ENABLE_BLUETOOTH
class BluetoothScanAttack;
#endif
#ifdef ENABLE_JAMMING
class JammingAttack;
#endif

class AttackManager {
  public:
    AttackManager();
    ~AttackManager();
    
    // Initialize the attack manager
    void begin();
    
    // Update all attacks (called in main loop)
    void update();
    
    // Get attack by type
    Attack* getAttack(uint8_t type);
    
    // Get attack by index
    Attack* getAttackByIndex(int index);
    
    // Get number of available attacks
    int getAttackCount() const;
    
    // Start an attack by type
    bool startAttack(uint8_t type, unsigned long duration = DEFAULT_ATTACK_DURATION);
    
    // Stop an attack by type
    bool stopAttack(uint8_t type);
    
    // Stop all running attacks
    void stopAllAttacks();
    
    // Check if any attack is running
    bool isAnyAttackRunning() const;
    
    // Get running attack count
    int getRunningAttackCount() const;
    
  private:
    std::vector<Attack*> attacks;
    
    // Attack instances
    DeauthAttack* deauthAttack;
    BeaconFloodAttack* beaconFloodAttack;
    ProbeSpamAttack* probeSpamAttack;
    ArpSpoofAttack* arpSpoofAttack;
    MqttSpoofAttack* mqttSpoofAttack;
    EvilTwinAttack* evilTwinAttack;
    PassiveSniffAttack* passiveSniffAttack;
    #ifdef ENABLE_BLUETOOTH
    BluetoothScanAttack* bluetoothScanAttack;
    #endif
    #ifdef ENABLE_JAMMING
    JammingAttack* jammingAttack;
    #endif
};

#endif // ATTACK_MANAGER_H
