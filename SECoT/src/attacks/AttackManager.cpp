/*
 * AttackManager.cpp
 *
 * Implementation of the AttackManager class
 */

#include "AttackManager.h"
#include "DeauthAttack.h"
#include "BeaconFloodAttack.h"
#include "ProbeSpamAttack.h"
#include "MqttSpoofAttack.h"
#include "ArpSpoofAttack.h"
#include "EvilTwinAttack.h"
#include "PassiveSniffAttack.h"

// Forward declarations of other attack classes
#ifdef ENABLE_BLUETOOTH
class BluetoothScanAttack;
#endif
#ifdef ENABLE_JAMMING
class JammingAttack;
#endif

AttackManager::AttackManager() {
  // Initialize attack instances to nullptr
  deauthAttack = nullptr;
  beaconFloodAttack = nullptr;
  probeSpamAttack = nullptr;
  arpSpoofAttack = nullptr;
  mqttSpoofAttack = nullptr;
  evilTwinAttack = nullptr;
  passiveSniffAttack = nullptr;
  #ifdef ENABLE_BLUETOOTH
  bluetoothScanAttack = nullptr;
  #endif
  #ifdef ENABLE_JAMMING
  jammingAttack = nullptr;
  #endif
}

AttackManager::~AttackManager() {
  // Clean up attack instances
  if (deauthAttack) delete deauthAttack;
  if (beaconFloodAttack) delete beaconFloodAttack;
  if (probeSpamAttack) delete probeSpamAttack;
  if (arpSpoofAttack) delete arpSpoofAttack;
  if (mqttSpoofAttack) delete mqttSpoofAttack;
  if (evilTwinAttack) delete evilTwinAttack;
  if (passiveSniffAttack) delete passiveSniffAttack;
  #ifdef ENABLE_BLUETOOTH
  if (bluetoothScanAttack) delete bluetoothScanAttack;
  #endif
  #ifdef ENABLE_JAMMING
  if (jammingAttack) delete jammingAttack;
  #endif
}

void AttackManager::begin() {
  // Create attack instances
  deauthAttack = new DeauthAttack();
  beaconFloodAttack = new BeaconFloodAttack();
  probeSpamAttack = new ProbeSpamAttack();
  arpSpoofAttack = new ArpSpoofAttack();
  mqttSpoofAttack = new MqttSpoofAttack();
  evilTwinAttack = new EvilTwinAttack();
  passiveSniffAttack = new PassiveSniffAttack();
  #ifdef ENABLE_BLUETOOTH
  // bluetoothScanAttack = new BluetoothScanAttack();
  #endif
  #ifdef ENABLE_JAMMING
  // jammingAttack = new JammingAttack();
  #endif

  // Add attacks to the vector
  attacks.push_back(deauthAttack);
  attacks.push_back(beaconFloodAttack);
  attacks.push_back(probeSpamAttack);
  attacks.push_back(arpSpoofAttack);
  attacks.push_back(mqttSpoofAttack);
  attacks.push_back(evilTwinAttack);
  attacks.push_back(passiveSniffAttack);
  #ifdef ENABLE_BLUETOOTH
  // attacks.push_back(bluetoothScanAttack);
  #endif
  #ifdef ENABLE_JAMMING
  // attacks.push_back(jammingAttack);
  #endif
}

void AttackManager::update() {
  // Update all attacks
  for (Attack* attack : attacks) {
    if (attack) {
      attack->update();
    }
  }
}

Attack* AttackManager::getAttack(uint8_t type) {
  // Find attack by type
  for (Attack* attack : attacks) {
    if (attack && attack->getType() == type) {
      return attack;
    }
  }

  return nullptr;
}

Attack* AttackManager::getAttackByIndex(int index) {
  // Check if index is valid
  if (index >= 0 && index < attacks.size()) {
    return attacks[index];
  }

  return nullptr;
}

int AttackManager::getAttackCount() const {
  return attacks.size();
}

bool AttackManager::startAttack(uint8_t type, unsigned long duration) {
  // Find attack by type
  Attack* attack = getAttack(type);

  // Start the attack if found
  if (attack) {
    return attack->start(duration);
  }

  return false;
}

bool AttackManager::stopAttack(uint8_t type) {
  // Find attack by type
  Attack* attack = getAttack(type);

  // Stop the attack if found
  if (attack) {
    return attack->stop();
  }

  return false;
}

void AttackManager::stopAllAttacks() {
  // Stop all attacks
  for (Attack* attack : attacks) {
    if (attack && attack->getStatus() == ATTACK_STATUS_RUNNING) {
      attack->stop();
    }
  }
}

bool AttackManager::isAnyAttackRunning() const {
  // Check if any attack is running
  for (Attack* attack : attacks) {
    if (attack && attack->getStatus() == ATTACK_STATUS_RUNNING) {
      return true;
    }
  }

  return false;
}

int AttackManager::getRunningAttackCount() const {
  // Count running attacks
  int count = 0;

  for (Attack* attack : attacks) {
    if (attack && attack->getStatus() == ATTACK_STATUS_RUNNING) {
      count++;
    }
  }

  return count;
}
