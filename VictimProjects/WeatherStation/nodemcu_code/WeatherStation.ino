/*
 * WeatherStation.ino
 * 
 * ESP8266 NodeMCU code for a simple IoT weather station
 * Reads temperature and humidity from a DHT22 sensor
 * Publishes data to an MQTT broker
 * 
 * Hardware:
 * - NodeMCU ESP8266
 * - DHT22 temperature and humidity sensor
 * - Optional: LED for status indication
 * 
 * Libraries required:
 * - ESP8266WiFi
 * - PubSubClient
 * - DHT sensor library
 * - Adafruit Unified Sensor
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Pin Definitions
#define DHTPIN D4       // DHT22 data pin
#define DHTTYPE DHT22   // DHT sensor type
#define LED_PIN D0      // Built-in LED for status indication

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// MQTT Broker settings
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* client_id = "weather_station_1";

// MQTT Topics
const char* temperature_topic = "home/weatherstation/temperature";
const char* humidity_topic = "home/weatherstation/humidity";
const char* status_topic = "home/weatherstation/status";

// Timing variables
unsigned long lastSensorReadTime = 0;
const long sensorReadInterval = 10000;  // Read sensor every 10 seconds
unsigned long lastMqttReconnectAttempt = 0;
const long mqttReconnectInterval = 5000;  // Try to reconnect every 5 seconds

// Global variables
float temperature = 0.0;
float humidity = 0.0;
bool ledState = false;

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\nWeather Station Starting...");
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off (active low)
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  setupWiFi();
  
  // Configure MQTT connection
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  // Initial sensor reading
  readSensor();
  
  // Indicate setup complete
  blinkLED(3);
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
    unsigned long currentMillis = millis();
    if (currentMillis - lastMqttReconnectAttempt > mqttReconnectInterval) {
      lastMqttReconnectAttempt = currentMillis;
      if (reconnectMQTT()) {
        lastMqttReconnectAttempt = 0;
      }
    }
  } else {
    // MQTT client loop processing
    mqttClient.loop();
  }
  
  // Read sensor at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorReadTime > sensorReadInterval) {
    lastSensorReadTime = currentMillis;
    readSensor();
    publishData();
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
    digitalWrite(LED_PIN, ledState);  // Blink LED while connecting
    ledState = !ledState;
  }
  
  // Connected to WiFi
  digitalWrite(LED_PIN, HIGH);  // LED off
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

bool reconnectMQTT() {
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
    
    // Subscribe to command topics if needed
    // mqttClient.subscribe("home/weatherstation/commands");
    
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
    return false;
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Process message
  // This is where you would add code to handle commands
  // For example, changing reporting frequency or LED state
}

void readSensor() {
  // Read humidity
  float newHumidity = dht.readHumidity();
  // Read temperature in Celsius
  float newTemperature = dht.readTemperature();
  
  // Check if any reads failed
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Update global variables if read was successful
  humidity = newHumidity;
  temperature = newTemperature;
  
  // Print values to serial
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  // Blink LED to indicate successful reading
  blinkLED(1);
}

void publishData() {
  if (!mqttClient.connected()) {
    return;
  }
  
  // Publish temperature
  String tempStr = String(temperature, 1);
  mqttClient.publish(temperature_topic, tempStr.c_str(), true);
  
  // Publish humidity
  String humStr = String(humidity, 1);
  mqttClient.publish(humidity_topic, humStr.c_str(), true);
  
  // Publish detailed JSON data
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["timestamp"] = millis();
  jsonDoc["device_id"] = client_id;
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  mqttClient.publish("home/weatherstation/data", jsonString.c_str(), true);
  
  Serial.println("Data published to MQTT broker");
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LOW);  // LED on
    delay(100);
    digitalWrite(LED_PIN, HIGH); // LED off
    delay(100);
  }
}
