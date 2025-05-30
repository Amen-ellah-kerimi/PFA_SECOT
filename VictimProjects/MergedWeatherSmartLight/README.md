# Merged Weather Station and Smart Light System

This project combines the functionality of a Weather Station and Smart Light system into a single ESP8266 NodeMCU-based IoT device. The system monitors temperature and humidity while providing smart LED control capabilities.

## Features

### Weather Station
- Temperature monitoring using DHT11 sensor
- Humidity monitoring using DHT11 sensor
- Real-time data display on 16x2 LCD
- Alert system with buzzer for threshold violations
- Real-time data publishing to MQTT broker
- Status monitoring and reporting

### Smart Light
- Control of three LEDs (Red, Yellow, Blue)
- Individual LED state tracking
- Duration tracking for each LED (ON/OFF time)
- Ambient light sensing
- MQTT-based remote control

## Hardware Requirements

- ESP8266 NodeMCU
- DHT11 Temperature and Humidity Sensor
- 16x2 LCD Display (I2C)
- Buzzer for alerts
- 3 LEDs (Red, Yellow, Blue)
- LDR (Light Dependent Resistor)
- Resistors (220Ω for LEDs, 10kΩ for LDR)
- Jumper wires
- Breadboard

## Pin Configuration

- DHT11 Data Pin: D4
- Red LED: D1
- Yellow LED: D2
- Blue LED: D3
- Buzzer: D5
- LDR: A0
- Status LED: D0
- LCD I2C: SDA (D2), SCL (D1)

## Software Requirements

### Arduino IDE Libraries
- ESP8266WiFi
- PubSubClient
- DHT sensor library
- Adafruit Unified Sensor
- ArduinoJson
- LiquidCrystal_I2C
- Wire

## Alert Thresholds

The system includes configurable thresholds for alerts:
- High Temperature: 30.0°C
- Low Temperature: 10.0°C
- High Humidity: 80.0%
- Low Humidity: 20.0%

When thresholds are exceeded:
- Buzzer activates with alert pattern
- Alert message published to MQTT
- LCD displays alert status

## MQTT Topics

### Weather Station
- `home/weatherstation/temperature` - Temperature readings
- `home/weatherstation/humidity` - Humidity readings
- `home/weatherstation/status` - Device status and detailed data
- `home/weatherstation/alert` - Alert notifications

### Smart Light
- `SmartLight/LED/RED` - Red LED control
- `SmartLight/LED/YELLOW` - Yellow LED control
- `SmartLight/LED/BLUE` - Blue LED control
- `home/smartlight/status` - LED states and timing information

## Setup Instructions

1. Install required libraries in Arduino IDE
2. Configure WiFi credentials in the code
3. Configure MQTT broker settings
4. Connect hardware components according to pin configuration
5. Upload the code to your NodeMCU
6. Monitor the Serial console for connection status

## LCD Display

The 16x2 LCD shows:
- First row: Temperature and Humidity readings
- Second row: LED status (ON/OFF) for all three LEDs

## LED Control Commands

Send the following commands to the respective MQTT topics:
- `ON` or `on` or `1` - Turn LED on
- `OFF` or `off` or `0` - Turn LED off
- `TOGGLE` or `toggle` - Toggle LED state

## Security Features

The system supports three MQTT broker security scenarios:

1. No Security
   - No authentication
   - No TLS
   - Anonymous publishing allowed

2. Basic Security
   - 4-digit PIN authentication
   - ACLs enabled
   - Weak passwords (for testing)

3. Secure Broker
   - Strong passwords
   - TLS/SSL encryption
   - Full access control

## Web Dashboard

The system includes a web dashboard that displays:
- Real-time temperature and humidity readings
- LED status and control
- Duration tracking for each LED
- Alert history and status
- Dark/Light mode toggle

## Android Apps

Two separate Android apps are available:
1. Weather Station App
   - Real-time weather data display
   - Historical data graphs
   - Alert notifications
   - Dark/Light mode support

2. Smart Light App
   - LED control interface
   - Status monitoring
   - Duration tracking
   - Dark/Light mode support

## Deployment

The project includes Python scripts for automated deployment of MQTT broker scenarios. Requirements:
- Docker Desktop
- Python 3.x
- Required Python packages (see requirements.txt)

## Contributing

Feel free to submit issues and enhancement requests! 