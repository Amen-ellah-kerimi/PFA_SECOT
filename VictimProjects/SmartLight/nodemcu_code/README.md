# Smart Light Control System - NodeMCU Code

This directory contains the Arduino code for the NodeMCU ESP8266 that powers the IoT Smart Light Control System.

## Hardware Requirements

- NodeMCU ESP8266 development board
- RGB LED (common cathode) or three separate LEDs (red, green, blue)
- LDR (Light Dependent Resistor) for ambient light sensing
- PIR motion sensor (HC-SR501 or similar)
- Resistors: 3 x 220Ω (for LEDs), 1 x 10kΩ (for LDR pull-down)
- Breadboard and jumper wires
- Micro USB cable for programming and power

## Wiring

Connect the components as follows:

- **RGB LED:**
  - Red pin to NodeMCU D1 (through 220Ω resistor)
  - Green pin to NodeMCU D2 (through 220Ω resistor)
  - Blue pin to NodeMCU D3 (through 220Ω resistor)
  - Common cathode to GND

- **LDR (Light Sensor):**
  - One leg to NodeMCU 3.3V
  - Other leg to NodeMCU A0 and to GND through 10kΩ resistor

- **PIR Motion Sensor:**
  - VCC pin to NodeMCU 3.3V
  - OUT pin to NodeMCU D5
  - GND pin to NodeMCU GND

## Software Requirements

1. Arduino IDE (1.8.x or newer)
2. ESP8266 board support package
3. Required libraries:
   - ESP8266WiFi
   - PubSubClient
   - ArduinoJson

## Setup Instructions

1. Install Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)

2. Add ESP8266 board support:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to "Additional Boards Manager URLs"
   - Go to Tools > Board > Boards Manager
   - Search for "esp8266" and install "ESP8266 by ESP8266 Community"

3. Install required libraries:
   - Go to Tools > Manage Libraries
   - Search for and install each of the required libraries listed above

4. Configure the code:
   - Open `SmartLight.ino` in Arduino IDE
   - Update WiFi credentials (SSID and password)
   - Update MQTT broker settings (server IP, port, username, password)
   - Adjust pin assignments if your wiring differs
   - Save the changes

5. Upload the code:
   - Connect NodeMCU to your computer via USB
   - Select the correct board: Tools > Board > ESP8266 Boards > NodeMCU 1.0
   - Select the correct port: Tools > Port > (select the COM port for your NodeMCU)
   - Click the Upload button (right arrow icon)

6. Verify operation:
   - Open the Serial Monitor (Tools > Serial Monitor)
   - Set baud rate to 115200
   - You should see connection and status messages

## MQTT Topics

The Smart Light system uses the following MQTT topics:

| Topic | Direction | Description | Payload Example |
|-------|-----------|-------------|----------------|
| `home/smartlight/state` | Publish | Current state of the light | `{"state":"ON","brightness":255,"color":{"r":255,"g":255,"b":255},"ambient":75,"motion":true,"timestamp":123456}` |
| `home/smartlight/command` | Subscribe | Commands to control light state | `ON`, `OFF`, or `TOGGLE` |
| `home/smartlight/brightness` | Subscribe | Set brightness level | `255` (0-255) |
| `home/smartlight/color` | Subscribe | Set RGB color | `{"r":255,"g":0,"b":0}` |
| `home/smartlight/ambient` | Publish | Current ambient light level | `75` (0-100) |
| `home/smartlight/motion` | Publish | Motion detection status | `1` or `0` |
| `home/smartlight/status` | Publish | Device status information | `{"status":"online","ip":"192.168.1.100"}` |

## Troubleshooting

- **LEDs not lighting up**: Check the wiring and resistor values. Make sure the common cathode is connected to GND.
- **Cannot connect to WiFi**: Verify your WiFi credentials and ensure the NodeMCU is within range of your router.
- **Cannot connect to MQTT broker**: Check the broker IP address and credentials. Ensure the broker is running and accessible from your network.
- **Motion sensor not triggering**: Adjust the sensitivity and delay settings on the PIR sensor itself (usually has two potentiometers).
- **Ambient light readings incorrect**: Check the LDR wiring and resistor value. You may need to adjust the mapping in the code based on your lighting conditions.

## Security Considerations

This code is intentionally simplified for educational purposes. In a production environment, consider implementing:

- Secure MQTT connection using TLS/SSL
- Strong authentication mechanisms
- Encrypted WiFi connection
- OTA (Over-The-Air) updates
- Secure storage of credentials

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
