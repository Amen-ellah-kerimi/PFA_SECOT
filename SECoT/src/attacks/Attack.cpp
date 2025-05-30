/*
 * Attack.cpp
 * 
 * Implementation of the Attack base class
 */

#include "Attack.h"

Attack::Attack(uint8_t type, const String& name) {
  this->type = type;
  this->name = name;
  this->status = ATTACK_STATUS_IDLE;
  this->errorMessage = "";
  this->startTime = 0;
  this->duration = 0;
  this->lastActionTime = 0;
}

Attack::~Attack() {
  // Base destructor
}

uint8_t Attack::getType() const {
  return type;
}

String Attack::getName() const {
  return name;
}

uint8_t Attack::getStatus() const {
  return status;
}

String Attack::getStatusString() const {
  return statusToString(status);
}

String Attack::getErrorMessage() const {
  return errorMessage;
}

bool Attack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool Attack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void Attack::update() {
  // Check if attack should stop due to duration
  if (status == ATTACK_STATUS_RUNNING && duration > 0) {
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime >= duration) {
      stop();
    }
  }
  
  // Handle stopping state
  if (status == ATTACK_STATUS_STOPPING) {
    status = ATTACK_STATUS_IDLE;
  }
}

bool Attack::setParameter(const String& name, const String& value) {
  // Base implementation doesn't have parameters
  return false;
}

String Attack::getParameter(const String& name) const {
  // Base implementation doesn't have parameters
  return "";
}

String Attack::getAllParameters() const {
  // Base implementation returns empty JSON object
  return "{}";
}

String Attack::statusToString(uint8_t status) const {
  switch (status) {
    case ATTACK_STATUS_IDLE:
      return "IDLE";
    case ATTACK_STATUS_RUNNING:
      return "RUNNING";
    case ATTACK_STATUS_STOPPING:
      return "STOPPING";
    case ATTACK_STATUS_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
