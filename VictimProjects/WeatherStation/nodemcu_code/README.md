# Weather Station - NodeMCU Code

This directory contains the Arduino code for the NodeMCU ESP8266 that powers the IoT Weather Station.

## Hardware Requirements

- NodeMCU ESP8266 development board
- DHT22 temperature and humidity sensor
- Breadboard and jumper wires
- Micro USB cable for programming and power
- Optional: 10K resistor (for DHT22 pull-up)
- Optional: LED for status indication

## Wiring

Connect the components as follows:

- DHT22 VCC pin to NodeMCU 3.3V
- DHT22 DATA pin to NodeMCU D4
- DHT22 GND pin to NodeMCU GND
- Optional: 10K resistor between VCC and DATA pins
- Optional: LED anode to NodeMCU D0 through a 220Ω resistor
- Optional: LED cathode to GND

## Software Requirements

1. Arduino IDE (1.8.x or newer)
2. ESP8266 board support package
3. Required libraries:
   - ESP8266WiFi
   - PubSubClient
   - DHT sensor library
   - Adafruit Unified Sensor
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
   - Open `WeatherStation.ino` in Arduino IDE
   - Update WiFi credentials (SSID and password)
   - Update MQTT broker settings (server IP, port, username, password)
   - Adjust MQTT topics if needed
   - Save the changes

5. Upload the code:
   - Connect NodeMCU to your computer via USB
   - Select the correct board: Tools > Board > ESP8266 Boards > NodeMCU 1.0
   - Select the correct port: Tools > Port > (select the COM port for your NodeMCU)
   - Click the Upload button (right arrow icon)

6. Verify operation:
   - Open the Serial Monitor (Tools > Serial Monitor)
   - Set baud rate to 115200
   - You should see connection and sensor reading messages

## Troubleshooting

- **Sensor readings show NaN**: Check the wiring of the DHT22 sensor. Ensure the data pin has a pull-up resistor.
- **Cannot connect to WiFi**: Verify your WiFi credentials and ensure the NodeMCU is within range of your router.
- **Cannot connect to MQTT broker**: Check the broker IP address and credentials. Ensure the broker is running and accessible from your network.
- **Compilation errors**: Make sure all required libraries are installed.

## Security Considerations

This code is intentionally simplified for educational purposes. In a production environment, consider implementing:

- Secure MQTT connection using TLS/SSL
- Strong authentication mechanisms
- Encrypted WiFi connection
- OTA (Over-The-Air) updates
- Secure storage of credentials

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
