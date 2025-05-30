/*
 * SmartLight.ino
 * 
 * ESP8266 NodeMCU code for a smart light control system
 * Controls RGB LED and monitors light status
 * Publishes and subscribes to MQTT topics for remote control
 * 
 * Hardware:
 * - NodeMCU ESP8266
 * - RGB LED (or individually connected LEDs)
 * - LDR (Light Dependent Resistor) for ambient light sensing
 * - Optional: PIR motion sensor for presence detection
 * 
 * Libraries required:
 * - ESP8266WiFi
 * - PubSubClient
 * - ArduinoJson
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Pin Definitions
#define RED_PIN D1       // Red LED pin
#define GREEN_PIN D2     // Green LED pin
#define BLUE_PIN D3      // Blue LED pin
#define LDR_PIN A0       // Light sensor analog pin
#define PIR_PIN D5       // PIR motion sensor pin (optional)

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// MQTT Broker settings
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* client_id = "smart_light_1";

// MQTT Topics
const char* light_state_topic = "home/smartlight/state";
const char* light_command_topic = "home/smartlight/command";
const char* light_brightness_topic = "home/smartlight/brightness";
const char* light_color_topic = "home/smartlight/color";
const char* light_ambient_topic = "home/smartlight/ambient";
const char* light_motion_topic = "home/smartlight/motion";
const char* status_topic = "home/smartlight/status";

// Light state variables
bool lightState = false;
int brightness = 255;
int redValue = 255;
int greenValue = 255;
int blueValue = 255;
int ambientLight = 0;
bool motionDetected = false;

// Timing variables
unsigned long lastPublishTime = 0;
const long publishInterval = 10000;  // Publish state every 10 seconds
unsigned long lastMotionTime = 0;
const long motionTimeout = 60000;    // Motion timeout after 1 minute
unsigned long lastAmbientReadTime = 0;
const long ambientReadInterval = 5000; // Read ambient light every 5 seconds

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\nSmart Light System Starting...");
  
  // Initialize pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  
  // Set initial LED state (off)
  setLightState(false);
  
  // Connect to WiFi
  setupWiFi();
  
  // Configure MQTT connection
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  // Initial sensor readings
  readAmbientLight();
  
  Serial.println("Setup complete");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    setupWiFi();
  }
  
  // Check MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  // MQTT client loop processing
  mqttClient.loop();
  
  // Read ambient light at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastAmbientReadTime > ambientReadInterval) {
    lastAmbientReadTime = currentMillis;
    readAmbientLight();
  }
  
  // Check motion sensor
  checkMotionSensor();
  
  // Publish state at regular intervals
  if (currentMillis - lastPublishTime > publishInterval) {
    lastPublishTime = currentMillis;
    publishState();
  }
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Connected to WiFi
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID if not set
    String clientId = client_id;
    if (clientId == "") {
      clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
    }
    
    // Attempt to connect
    bool connected = false;
    if (mqtt_user != "" && mqtt_password != "") {
      connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_password);
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }
    
    if (connected) {
      Serial.println("connected");
      
      // Publish a connection message
      String statusMessage = "{\"status\":\"online\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
      mqttClient.publish(status_topic, statusMessage.c_str(), true);
      
      // Subscribe to command topics
      mqttClient.subscribe(light_command_topic);
      mqttClient.subscribe(light_brightness_topic);
      mqttClient.subscribe(light_color_topic);
      
      // Publish initial state
      publishState();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  // Process message based on topic
  if (strcmp(topic, light_command_topic) == 0) {
    // Handle on/off commands
    if (strcmp(message, "ON") == 0 || strcmp(message, "on") == 0 || strcmp(message, "1") == 0) {
      setLightState(true);
    } else if (strcmp(message, "OFF") == 0 || strcmp(message, "off") == 0 || strcmp(message, "0") == 0) {
      setLightState(false);
    } else if (strcmp(message, "TOGGLE") == 0 || strcmp(message, "toggle") == 0) {
      setLightState(!lightState);
    }
  } 
  else if (strcmp(topic, light_brightness_topic) == 0) {
    // Handle brightness commands
    int newBrightness = atoi(message);
    if (newBrightness >= 0 && newBrightness <= 255) {
      brightness = newBrightness;
      if (lightState) {
        updateLightOutput();
      }
    }
  } 
  else if (strcmp(topic, light_color_topic) == 0) {
    // Handle color commands in JSON format
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error) {
      if (doc.containsKey("r")) redValue = doc["r"];
      if (doc.containsKey("g")) greenValue = doc["g"];
      if (doc.containsKey("b")) blueValue = doc["b"];
      
      if (lightState) {
        updateLightOutput();
      }
    }
  }
  
  // Publish updated state
  publishState();
}

void setLightState(bool state) {
  lightState = state;
  updateLightOutput();
}

void updateLightOutput() {
  if (lightState) {
    // Calculate scaled values based on brightness
    int scaledRed = map(redValue, 0, 255, 0, brightness);
    int scaledGreen = map(greenValue, 0, 255, 0, brightness);
    int scaledBlue = map(blueValue, 0, 255, 0, brightness);
    
    // Write to LED pins (PWM)
    analogWrite(RED_PIN, scaledRed);
    analogWrite(GREEN_PIN, scaledGreen);
    analogWrite(BLUE_PIN, scaledBlue);
    
    Serial.printf("Light ON: R=%d, G=%d, B=%d, Brightness=%d\n", 
                 scaledRed, scaledGreen, scaledBlue, brightness);
  } else {
    // Turn off all LEDs
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
    
    Serial.println("Light OFF");
  }
}

void readAmbientLight() {
  // Read analog value from LDR
  int rawValue = analogRead(LDR_PIN);
  
  // Map to 0-100 range (0 = dark, 100 = bright)
  ambientLight = map(rawValue, 0, 1023, 0, 100);
  
  Serial.print("Ambient light level: ");
  Serial.println(ambientLight);
}

void checkMotionSensor() {
  // Read motion sensor
  bool currentMotion = digitalRead(PIR_PIN) == HIGH;
  
  // If motion is detected, update timestamp
  if (currentMotion) {
    lastMotionTime = millis();
    
    // If this is a new motion detection, publish it
    if (!motionDetected) {
      motionDetected = true;
      mqttClient.publish(light_motion_topic, "1", true);
      Serial.println("Motion detected");
    }
  } 
  // If no motion and timeout has passed, clear motion state
  else if (motionDetected && (millis() - lastMotionTime > motionTimeout)) {
    motionDetected = false;
    mqttClient.publish(light_motion_topic, "0", true);
    Serial.println("Motion timeout");
  }
}

void publishState() {
  // Create JSON document for state
  DynamicJsonDocument stateDoc(256);
  
  stateDoc["state"] = lightState ? "ON" : "OFF";
  stateDoc["brightness"] = brightness;
  stateDoc["color"] = {
    {"r", redValue},
    {"g", greenValue},
    {"b", blueValue}
  };
  stateDoc["ambient"] = ambientLight;
  stateDoc["motion"] = motionDetected;
  stateDoc["timestamp"] = millis();
  
  // Serialize JSON to string
  String stateJson;
  serializeJson(stateDoc, stateJson);
  
  // Publish state
  mqttClient.publish(light_state_topic, stateJson.c_str(), true);
  
  // Also publish ambient light separately
  String ambientStr = String(ambientLight);
  mqttClient.publish(light_ambient_topic, ambientStr.c_str(), true);
}
