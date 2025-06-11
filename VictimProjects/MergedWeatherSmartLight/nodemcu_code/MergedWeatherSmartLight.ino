/*
 * MergedWeatherSmartLight.ino
 * 
 * ESP8266 NodeMCU code for a combined IoT system
 * Features:
 * - Weather Station functionality (temperature and humidity monitoring)
 * - Smart Light control with RGB LEDs
 * - LCD display for real-time information
 * - Buzzer for alerts and notifications
 * - Enhanced MQTT integration with security support
 * - Status monitoring and reporting
 * 
 * Hardware:
 * - NodeMCU ESP8266
 * - DHT11 temperature and humidity sensor
 * - 16x2 LCD display (I2C)
 * - Buzzer for alerts
 * - 3 LEDs (Red, Yellow, Blue)
 * 
 * Libraries required:
 * - ESP8266WiFi
 * - PubSubClient
 * - DHT sensor library
 * - Adafruit Unified Sensor
 * - ArduinoJson
 * - LiquidCrystal_I2C
 * - Wire
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define DHTPIN D4       // DHT11 data pin
#define DHTTYPE DHT11   // DHT sensor type
#define RED_PIN D1      // Red LED pin
#define YELLOW_PIN D2   // Yellow LED pin
#define BLUE_PIN D3     // Blue LED pin
#define BUZZER_PIN D5   // Buzzer pin
#define STATUS_LED D0   // Status LED pin

// LCD Configuration
#define LCD_ADDRESS 0x27  // I2C address of the LCD
#define LCD_COLUMNS 16    // Number of columns in LCD
#define LCD_ROWS 2        // Number of rows in LCD

// Alert Thresholds
#define TEMP_HIGH_THRESHOLD 30.0  // High temperature threshold (°C)
#define TEMP_LOW_THRESHOLD 10.0   // Low temperature threshold (°C)
#define HUMIDITY_HIGH_THRESHOLD 80.0  // High humidity threshold (%)
#define HUMIDITY_LOW_THRESHOLD 20.0   // Low humidity threshold (%)

// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// MQTT Broker configurations
struct MqttConfig {
  const char* server;
  int port;
  const char* username;
  const char* password;
  bool useTLS;
};

// CA Certificate for secure MQTT
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G
A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp
Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4
MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG
A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBAKtFvNZyRHM6b2VZZWPrZWyxVxJZ8ok0My7o
1Y14mmfx/E5kzhri2jqxnP+Ic4R9mqvzdgkn3gOS19UNM5O3UFLJEFqcu60diKSO
v1hEtF+p7uHM2NDkbMHXBPim2L7p2iU/rWxxz1miJvduUqqi5Zt3dagyeZ1nu9Mz
KpImrUHR4W64idjquZ3aav3k/k/xxp5lhOp9VkiEHwdB4+3tURHoPAqVsgLYeYHk
tztgQDdwvQKXPgXBV6Yc9V6G3uUE0OM5+Ni0WYwI/cPtikj2zRbHnNcjDg2HTFZH
fJYprW1VxGVtQ4+njbGxJvUFwO5EdhvgBBKYYm7Y3+rvR+Ne8O/vY3zr2dqi/F2M
c+CAwEAAaNCMEAwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYD
VR0OBBYEFI/wS3+oLkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBL
QQNlrn7E1AL2Nh7kxAZh8I84ReLMPQ6a/YlGkOmD4z9bZxXPi5VEbF6wUjHtP6Zz
54U4Vb5Sd1l6YPsa3C872IEW6tuvWbuVQwzN2e72Db+3td7LHuKcZEmk1Pvj8jOS
Z0MMgxQ0sON5jl76S1aYfpzT0c0JvjYPtqtD9GDpnN55oOJmKPaEABaUnhF3TN7f
5Vx2xp7io+VsYETkA8j6KQj0ln1TSqU2WfGpnaW6SPD/fsj6znEZ2vCzeLAAwGez
61JNtmeN1g15GSDc5E4k1WO5YcnBMyoYdUECwUDiB3+m5WjW8M8KtQNtMNQvfS5
-----END CERTIFICATE-----
)EOF";

// MQTT configurations for different security levels
const MqttConfig mqttConfigs[] = {
  // No security
  {"192.168.152.76", 1883, "", "", false},
  // Basic security
  {"192.168.152.76", 1884, "weather_station", "1234", false},
  // Secure TLS
  {"192.168.152.76", 8883, "weather_station", "WeatherStation2024!", true}
};

// Current MQTT configuration index
int currentMqttConfig = 0;

// Client ID for MQTT
const char* client_id = "MergedWeatherSmartLight";

// MQTT Topics
// Weather Station Topics
const char* temperature_topic = "home/weatherstation/temperature";
const char* humidity_topic = "home/weatherstation/humidity";
const char* weather_status_topic = "home/weatherstation/status";
const char* weather_data_topic = "home/weatherstation/data";

// Smart Light Topics
const char* light_state_topic = "home/smartlight/state";
const char* light_command_topic = "home/smartlight/command";
const char* light_brightness_topic = "home/smartlight/brightness";
const char* light_color_topic = "home/smartlight/color";
const char* light_motion_topic = "home/smartlight/motion";
const char* light_status_topic = "home/smartlight/status";

// Timing variables
unsigned long lastSensorReadTime = 0;
const long sensorReadInterval = 10000;  // Read sensor every 10 seconds
unsigned long lastMqttReconnectAttempt = 0;
const long mqttReconnectInterval = 5000;  // Try to reconnect every 5 seconds
unsigned long lastLCDUpdateTime = 0;
const long lcdUpdateInterval = 1000;     // Update LCD every second
unsigned long lastBuzzerTime = 0;
const long buzzerInterval = 5000;        // Buzzer alert interval

// LED state tracking
struct LEDState {
  bool isOn;
  unsigned long lastStateChange;
  unsigned long totalOnTime;
  unsigned long totalOffTime;
};

LEDState redLED = {false, 0, 0, 0};
LEDState yellowLED = {false, 0, 0, 0};
LEDState blueLED = {false, 0, 0, 0};

// Weather variables
float temperature = 0.0;
float humidity = 0.0;
bool alertActive = false;

// Initialize components
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
WiFiClient espClient;
WiFiClientSecure espClientSecure;
PubSubClient* mqttClient = nullptr;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\nMerged Weather Station and Smart Light System Starting...");
  
  // Initialize pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  // Set initial LED states (off)
  setLEDState(RED_PIN, false);
  setLEDState(YELLOW_PIN, false);
  setLEDState(BLUE_PIN, false);
  setLEDState(STATUS_LED, false);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize I2C and LCD
  Wire.begin();
  Serial.println("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  Serial.println("LCD initialized");
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  setupWiFi();
  
  // Initialize MQTT client
  initializeMqtt();
  
  // Initial sensor reading
  readSensor();
  
  // Indicate setup complete
  blinkStatusLED(3);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  Serial.println("Setup complete");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    setupWiFi();
  }
  
  // Check MQTT connection
  if (!mqttClient || !mqttClient->connected()) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMqttReconnectAttempt > mqttReconnectInterval) {
      lastMqttReconnectAttempt = currentMillis;
      
      // Try to reconnect with current config first
      if (!reconnectMQTT()) {
        // If current config fails, try other configs
        delete mqttClient;
        mqttClient = nullptr;
        initializeMqtt();
      }
    }
  } else {
    // MQTT client loop processing
    mqttClient->loop();
  }
  
  // Read sensors at regular intervals
  unsigned long currentMillis = millis();
  
  // Update LED timing statistics
  updateLEDTiming(redLED, currentMillis);
  updateLEDTiming(yellowLED, currentMillis);
  updateLEDTiming(blueLED, currentMillis);
  
  if (currentMillis - lastSensorReadTime > sensorReadInterval) {
    lastSensorReadTime = currentMillis;
    readSensor();
    publishWeatherData();
    checkAlerts();
  }
  
  if (currentMillis - lastLCDUpdateTime > lcdUpdateInterval) {
    lastLCDUpdateTime = currentMillis;
    updateLCD();
  }
  
  // Handle buzzer alerts
  if (alertActive && (currentMillis - lastBuzzerTime > buzzerInterval)) {
    lastBuzzerTime = currentMillis;
    triggerBuzzer();
  }
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    blinkStatusLED(1);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void initializeMqtt() {
  // Try each MQTT configuration until one succeeds
  for (int i = 0; i < 3; i++) {
    currentMqttConfig = i;
    const MqttConfig& config = mqttConfigs[i];
    
    if (config.useTLS) {
      // Use secure client for TLS
      espClientSecure.setCACert((const uint8_t*)ca_cert, strlen(ca_cert));
      mqttClient = new PubSubClient(espClientSecure);
    } else {
      // Use regular client for non-TLS
      mqttClient = new PubSubClient(espClient);
    }
    
    mqttClient->setServer(config.server, config.port);
    mqttClient->setCallback(mqttCallback);
    
    if (reconnectMQTT()) {
      Serial.print("Successfully connected to MQTT broker with config ");
      Serial.println(i);
      return;
    }
    
    // If connection failed, delete the client and try next config
    delete mqttClient;
    mqttClient = nullptr;
  }
  
  Serial.println("Failed to connect to any MQTT broker");
}

bool reconnectMQTT() {
  const MqttConfig& config = mqttConfigs[currentMqttConfig];
  
  if (mqttClient->connect(client_id, config.username, config.password)) {
    Serial.println("Connected to MQTT broker");
    
    // Subscribe to Smart Light topics
    mqttClient->subscribe(light_command_topic);
    mqttClient->subscribe(light_brightness_topic);
    mqttClient->subscribe(light_color_topic);
    
    // Publish initial status
    publishStatus();
    return true;
  }
  return false;
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
  
  // Process LED commands
  if (strcmp(topic, light_command_topic) == 0) {
    if (strcmp(message, "ON") == 0 || strcmp(message, "on") == 0 || strcmp(message, "1") == 0) {
      handleLEDCommand(RED_PIN, redLED, "ON");
      handleLEDCommand(YELLOW_PIN, yellowLED, "ON");
      handleLEDCommand(BLUE_PIN, blueLED, "ON");
    } else if (strcmp(message, "OFF") == 0 || strcmp(message, "off") == 0 || strcmp(message, "0") == 0) {
      handleLEDCommand(RED_PIN, redLED, "OFF");
      handleLEDCommand(YELLOW_PIN, yellowLED, "OFF");
      handleLEDCommand(BLUE_PIN, blueLED, "OFF");
    }
  }
  else if (strcmp(topic, light_brightness_topic) == 0) {
    int brightness = atoi(message);
    // Map brightness (0-255) to LED states
    bool state = brightness > 0;
    handleLEDCommand(RED_PIN, redLED, state ? "ON" : "OFF");
    handleLEDCommand(YELLOW_PIN, yellowLED, state ? "ON" : "OFF");
    handleLEDCommand(BLUE_PIN, blueLED, state ? "ON" : "OFF");
  }
  else if (strcmp(topic, light_color_topic) == 0) {
    // Parse RGB color JSON
    JsonDocument doc;
    deserializeJson(doc, message);
    
    if (doc["r"].is<int>()) {
      int r = doc["r"];
      handleLEDCommand(RED_PIN, redLED, r > 0 ? "ON" : "OFF");
    }
    if (doc["g"].is<int>()) {
      int g = doc["g"];
      handleLEDCommand(YELLOW_PIN, yellowLED, g > 0 ? "ON" : "OFF");
    }
    if (doc["b"].is<int>()) {
      int b = doc["b"];
      handleLEDCommand(BLUE_PIN, blueLED, b > 0 ? "ON" : "OFF");
    }
  }
  
  // Publish updated LED states
  publishLEDStates();
}

void handleLEDCommand(int pin, LEDState& led, const char* command) {
  bool newState = false;
  
  if (strcmp(command, "ON") == 0 || strcmp(command, "on") == 0 || strcmp(command, "1") == 0) {
    newState = true;
  } else if (strcmp(command, "OFF") == 0 || strcmp(command, "off") == 0 || strcmp(command, "0") == 0) {
    newState = false;
  } else if (strcmp(command, "TOGGLE") == 0 || strcmp(command, "toggle") == 0) {
    newState = !led.isOn;
  }
  
  if (newState != led.isOn) {
    led.lastStateChange = millis();
    if (led.isOn) {
      led.totalOnTime += (millis() - led.lastStateChange);
    } else {
      led.totalOffTime += (millis() - led.lastStateChange);
    }
    led.isOn = newState;
    setLEDState(pin, newState);
  }
}

void updateLEDTiming(LEDState& led, unsigned long currentMillis) {
  if (led.isOn) {
    led.totalOnTime += (currentMillis - led.lastStateChange);
  } else {
    led.totalOffTime += (currentMillis - led.lastStateChange);
  }
  led.lastStateChange = currentMillis;
}

void setLEDState(int pin, bool state) {
  digitalWrite(pin, state ? LOW : HIGH);  // Active low
}

void readSensor() {
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  humidity = newHumidity;
  temperature = newTemperature;
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

void publishWeatherData() {
  if (!mqttClient || !mqttClient->connected()) return;
  
  // Publish temperature
  String tempStr = String(temperature, 1);
  mqttClient->publish(temperature_topic, tempStr.c_str(), true);
  
  // Publish humidity
  String humStr = String(humidity, 1);
  mqttClient->publish(humidity_topic, humStr.c_str(), true);
  
  // Publish detailed JSON data
  JsonDocument jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["timestamp"] = millis();
  jsonDoc["device_id"] = client_id;
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  mqttClient->publish(weather_status_topic, jsonString.c_str(), true);
  mqttClient->publish(weather_data_topic, jsonString.c_str(), true);
}

void publishLEDStates() {
  if (!mqttClient || !mqttClient->connected()) return;
  
  JsonDocument jsonDoc;
  
  // Calculate overall light state
  bool anyLightOn = redLED.isOn || yellowLED.isOn || blueLED.isOn;
  jsonDoc["state"] = anyLightOn ? "ON" : "OFF";
  
  // Calculate brightness (0-255)
  int brightness = 0;
  if (redLED.isOn) brightness += 85;
  if (yellowLED.isOn) brightness += 85;
  if (blueLED.isOn) brightness += 85;
  jsonDoc["brightness"] = brightness;
  
  // Calculate color
  jsonDoc["color"]["r"] = redLED.isOn ? 255 : 0;
  jsonDoc["color"]["g"] = yellowLED.isOn ? 255 : 0;
  jsonDoc["color"]["b"] = blueLED.isOn ? 255 : 0;
  
  // Add motion state (always false for now, as we don't have a motion sensor)
  jsonDoc["motion"] = false;
  
  // Add timing information
  jsonDoc["timestamp"] = millis();
  jsonDoc["device_id"] = client_id;
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  mqttClient->publish(light_state_topic, jsonString.c_str(), true);
  mqttClient->publish(light_status_topic, jsonString.c_str(), true);
}

void blinkStatusLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(STATUS_LED, LOW);
    delay(100);
    digitalWrite(STATUS_LED, HIGH);
    delay(100);
  }
}

void updateLCD() {
  Serial.println("Updating LCD...");
  Serial.print("Current Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  
  lcd.clear();
  
  // First row: Temperature and Humidity
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C H:");
  lcd.print(humidity, 1);
  lcd.print("%");
  
  // Second row: LED status
  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.print(redLED.isOn ? "ON " : "OFF");
  lcd.print("Y:");
  lcd.print(yellowLED.isOn ? "ON " : "OFF");
  lcd.print("B:");
  lcd.print(blueLED.isOn ? "ON" : "OFF");
  
  Serial.println("LCD update complete");
}

void checkAlerts() {
  bool newAlert = false;
  String alertMessage = "";
  
  if (temperature > TEMP_HIGH_THRESHOLD) {
    newAlert = true;
    alertMessage = "High Temperature Alert!";
  } else if (temperature < TEMP_LOW_THRESHOLD) {
    newAlert = true;
    alertMessage = "Low Temperature Alert!";
  }
  
  if (humidity > HUMIDITY_HIGH_THRESHOLD) {
    newAlert = true;
    alertMessage += " High Humidity Alert!";
  } else if (humidity < HUMIDITY_LOW_THRESHOLD) {
    newAlert = true;
    alertMessage += " Low Humidity Alert!";
  }
  
  if (newAlert != alertActive) {
    alertActive = newAlert;
    if (alertActive) {
      // Publish alert to MQTT
      JsonDocument jsonDoc;
      jsonDoc["alert"] = alertMessage;
      jsonDoc["temperature"] = temperature;
      jsonDoc["humidity"] = humidity;
      jsonDoc["timestamp"] = millis();
      
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      mqttClient->publish(weather_data_topic, jsonString.c_str(), true);
      
      // Trigger buzzer
      triggerBuzzer();
    }
  }
}

void triggerBuzzer() {
  // Play alert pattern
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}

void publishStatus() {
  if (!mqttClient || !mqttClient->connected()) return;
  
  JsonDocument jsonDoc;
  jsonDoc["status"] = "online";
  jsonDoc["device_id"] = client_id;
  jsonDoc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  mqttClient->publish(weather_status_topic, jsonString.c_str(), true);
} 