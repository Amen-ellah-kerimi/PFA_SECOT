/*
 * MqttSpoofAttack.h
 * 
 * MQTT Spoofing Attack module
 */

#ifndef MQTT_SPOOF_ATTACK_H
#define MQTT_SPOOF_ATTACK_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Attack.h"

// Attack modes
#define MQTT_ATTACK_MODE_REALISTIC_DRIFT 0
#define MQTT_ATTACK_MODE_EXTREME_VALUES 1
#define MQTT_ATTACK_MODE_OSCILLATING 2
#define MQTT_ATTACK_MODE_FIXED_VALUE 3

class MqttSpoofAttack : public Attack {
  public:
    MqttSpoofAttack();
    virtual ~MqttSpoofAttack();
    
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
    
    // MQTT Broker settings
    String mqttServer;
    int mqttPort;
    String mqttClientId;
    String mqttUsername;
    String mqttPassword;
    
    // Target MQTT topics
    String topicPrefix;
    String topicTemperature;
    String topicHumidity;
    
    // Attack parameters
    uint8_t attackMode;
    unsigned long publishInterval;
    
    // Variables for fake data
    float fakeTemperature;
    float fakeHumidity;
    bool increasing;
    int modeCounter;
    
    // MQTT client
    WiFiClient espClient;
    PubSubClient* mqttClient;
    
    // Helper methods
    void setupWiFi();
    void reconnectMqtt();
    void generateFakeData();
    void publishFakeData();
    void switchAttackMode();
    String attackModeToString(uint8_t mode) const;
};

#endif // MQTT_SPOOF_ATTACK_H
