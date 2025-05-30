# SECoT - Security Evaluation and Compromise Toolkit

## Overview

SECoT is an ESP32-based security audit tool designed for educational purposes to demonstrate common vulnerabilities in IoT systems. It's inspired by tools like Flipper Zero, ESP32 Marauder, and ESP Ghost, but with a focus on IoT security testing and awareness.

⚠️ **Educational Purposes Only**: This tool is designed for educational and authorized security testing only. Using it against systems without permission may be illegal and unethical.

## Features

- **WiFi Attacks**: Deauthentication, beacon flooding, probe request spamming
- **MQTT Attacks**: Broker scanning, message spoofing, fake device emulation
- **Network Attacks**: ARP spoofing, passive sniffing, evil twin
- **Multiple Interfaces**: Serial terminal, TFT screen, and web interface
- **Modular Design**: Easy to extend with new attack modules

## Hardware Requirements

- ESP32-WROOM-32D development board
- Optional: TFT display (ILI9341 or similar)
- Optional: 10dBi external antenna for extended range
- Optional: 2x 170-point breadboards (or 1 large one)
- USB cable for power and programming

## Software Requirements

- Arduino IDE (1.8.x or newer)
- ESP32 board support package
- Required libraries:
  - WiFi.h (included in ESP32 board package)
  - PubSubClient (for MQTT attacks)
  - esp_wifi.h (included in ESP32 board package)
  - TFT_eSPI (if using TFT display)

## Installation

1. Install Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to Additional Board Manager URLs
   - Go to Tools > Board > Boards Manager
   - Search for ESP32 and install
3. Install required libraries:
   - Go to Tools > Manage Libraries
   - Search for and install:
     - PubSubClient
     - TFT_eSPI (if using TFT display)
4. Clone or download this repository
5. Open `SECoT.ino` in Arduino IDE
6. Configure options in `src/config/Config.h`
7. Upload to your ESP32 board

## Usage

### Serial Interface

The serial interface provides a command-line interface to control SECoT. Connect to the ESP32 using a serial monitor at 115200 baud.

Available commands:
- `help` - Show help message
- `scan [type]` - Scan for networks (wifi, mqtt, ble)
- `attack <type> [duration]` - Start an attack (optional duration in ms)
- `stop [type]` - Stop an attack (or all if no type specified)
- `status` - Show status of all attacks
- `set <attack> <param> <value>` - Set attack parameter
- `get <attack> <param>` - Get attack parameter
- `clear` - Clear the screen

#### CLI Tool Integration

SECoT supports direct communication with the SECoT CLI Tool via serial port. When connected to the CLI Tool:
- The device automatically detects the CLI Tool and switches to a machine-friendly JSON communication mode
- Commands can be sent from the CLI Tool and responses are returned in JSON format
- The TFT display (if enabled) will automatically pause updates while connected to the CLI Tool
- All commands available in interactive mode are also available through the CLI Tool

### TFT Interface (if enabled)

If you have a TFT display connected, SECoT provides a graphical interface with:
- Menu system for selecting attacks
- Network scanning results
- Attack status and parameters
- Visual feedback during attacks

### Web Interface (if enabled)

The web interface allows you to control SECoT from a web browser:
1. Connect to the WiFi network created by SECoT (default SSID: `SECoT_AP`, password: `securitytool`)
2. Open a web browser and navigate to `http://192.168.4.1`
3. Use the web interface to scan networks, configure and launch attacks

## Attack Modules

### WiFi Deauthentication Attack
Sends deauthentication frames to disconnect clients from their WiFi network.

Parameters:
- `ssid` - Target network SSID
- `bssid` - Target network BSSID (MAC address)
- `channel` - WiFi channel (1-14)
- `mac` - Target client MAC address
- `all` - Attack all clients (true/false)
- `frames` - Number of frames per burst
- `interval` - Interval between bursts (ms)

### MQTT Spoofing Attack
Connects to an MQTT broker and publishes fake data to topics.

Parameters:
- `ssid` - WiFi network SSID
- `password` - WiFi password
- `server` - MQTT broker address
- `port` - MQTT broker port
- `clientid` - MQTT client ID
- `username` - MQTT username (if required)
- `password` - MQTT password (if required)
- `prefix` - Topic prefix
- `topic_temp` - Temperature topic
- `topic_humidity` - Humidity topic
- `mode` - Attack mode (realistic, extreme, oscillating, fixed)
- `interval` - Publish interval (ms)

### ARP Spoofing Attack
Performs ARP cache poisoning to intercept network traffic.

Parameters:
- `ssid` - WiFi network SSID
- `password` - WiFi password
- `target` - Target device IP
- `gateway` - Gateway IP
- `broker` - MQTT broker IP
- `mode` - Attack mode (gateway, broker, both)
- `interval` - ARP packet interval (ms)

## Extending SECoT

SECoT is designed to be modular and extensible. To add a new attack module:

1. Create a new class that inherits from the `Attack` base class
2. Implement the required methods (start, stop, update, etc.)
3. Add your attack to the `AttackManager` in `AttackManager.cpp`
4. Update the command handlers in `SerialInterface.cpp`

## Victim Simulation

For testing purposes, you can use the included victim simulation tools:
- Weather Station (MQTT-based temperature and humidity sensor)
- Smart Light Controller (MQTT-controlled LED)
- Door Lock System (MQTT-based unlock mechanism)

These simulations run on a separate computer and demonstrate the effects of the attacks.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by [ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder)
- Inspired by [ESP Ghost](https://github.com/justcallmekoko/ESP-Ghost)
- Inspired by [Flipper Zero](https://flipperzero.one/)

## Disclaimer

This tool is provided for educational purposes only. The authors are not responsible for any misuse or damage caused by this tool. Always obtain proper authorization before testing security on any system you don't own.
