# SECoT Development Notes

## Project Overview
SECoT (Security Evaluation and Compromise Toolkit) is an ESP32-based security audit tool inspired by tools like Flipper Zero, ESP32 Marauder, and ESP Ghost. It's designed to run on an ESP32-WROOM-32D board with optional support for a TFT screen, mobile/PC control, and various network attack capabilities for testing real IoT devices and networks.

## Development Goals

### 1. Improve & Modularize the Existing Code
- ✅ Refactor long .ino files into .h/.cpp modules (1 per attack/tool)
- ✅ Clean and comment messy code
- ✅ Create a modular architecture with proper OOP design
- ✅ Implement base classes for attacks and utilities

### 2. Add Realistic Attacks/Tools
- ✅ WiFi Deauthentication Attack
- ✅ Beacon Flood Attack
- ✅ Probe Request Spam Attack
- ✅ MQTT Spoofing Attack
- ✅ ARP Spoofing Attack
- ✅ Evil Twin Attack
- ✅ Passive Sniffing Attack
- ⬜ Bluetooth Scanning Attack (if supported)
- ⬜ 2.4GHz Jammer (if supported)

### 3. Interface Improvements
- ✅ PC Terminal Interface (basic implementation)
- ✅ TFT Screen Interface (Marauder-like)
- ✅ Web Interface (for remote control)

### 4. Real IoT Device Testing Environment
- ⬜ Flutter apps for real IoT device monitoring:
  - ⬜ IoT Smart Lock Control App
  - ⬜ Temperature/Humidity Sensor Dashboard  
  - ⬜ MQTT Command & Control Interface
  - ⬜ Network Status Monitor

## Current Progress

### Completed Components
1. **Project Structure**
   - Created modular architecture with proper directories
   - Implemented base Attack class and AttackManager
   - Added utility classes (Logger, NetworkScanner)
   - Created configuration system

2. **Attack Modules**
   - DeauthAttack: WiFi deauthentication attack
   - BeaconFloodAttack: Floods area with fake WiFi networks
   - ProbeSpamAttack: Sends probe requests for different networks
   - MqttSpoofAttack: Spoofs MQTT messages
   - ArpSpoofAttack: ARP cache poisoning attack
   - EvilTwinAttack: Creates fake access point with captive portal
   - PassiveSniffAttack: Captures and analyzes WiFi packets

3. **Interfaces**
   - SerialInterface: Command-line interface for controlling SECoT

### In Progress
1. **Flutter App Development**
   - Need to start developing Flutter apps for real IoT devices
   - Implement MQTT connectivity
   - Create real-time data visualization

### Next Steps

1. **Develop Flutter Apps for Real IoT Devices**
   - Create Flutter apps that connect to actual IoT devices
   - Implement real-time data visualization
   - Show visual effects of attacks (e.g., MQTT spoofing, disconnections)
   - Connect to real hardware via MQTT, HTTP, or other protocols

2. **Add Bluetooth Support (if hardware allows)**
   - Implement BluetoothScanAttack class
   - Add BLE device discovery and analysis

3. **Testing and Refinement**
   - Test all attack modules on real IoT devices
   - Optimize for ESP32-WROOM-32D performance
   - Improve error handling and user feedback

## Implementation Notes

### Hardware Considerations

- Target hardware is ESP32-WROOM-32D
- Optional TFT screen (like ILI9341)
- Optional 10dBi antenna for extended range
- Limited RAM and CPU resources

### Target IoT Devices

- Real NodeMCU ESP8266 devices with various sensors:
  - DHT11/DHT22 temperature/humidity sensors
  - LCD displays
  - Buzzers
  - GPS modules
  - LEDs
  - Relays
- Different Arduino sketches will be flashed to create various IoT use cases:
  - Weather station
  - Smart lock
  - Home automation controller
  - Security system

### Testing Environment

- Private AP network for ethical testing
- Real IoT devices with permission from owners
- Isolated testing environment to prevent unintended disruption
- Visual feedback through device LCDs, LEDs, and Flutter apps

### Code Organization

- `SECoT.ino`: Main entry point
- `src/config/Config.h`: Configuration settings
- `src/attacks/`: Attack modules
- `src/interfaces/`: User interfaces
- `src/utils/`: Utility classes

### Attack Module Template

Each attack module follows this pattern:

1. Header file with class definition
2. Implementation file with methods
3. Parameters for customization
4. Start/stop/update lifecycle methods

### Interface Design

- SerialInterface: Command-line with help system
- TFTInterface: Menu-driven with visual feedback
- WebInterface: Responsive design for remote control

## Flutter App Development

- Create separate Flutter apps for each IoT use case
- Implement real-time data visualization
- Connect to MQTT brokers for live data
- Show visual effects of attacks:
  - Temperature/humidity graph spikes during MQTT spoofing
  - Connection loss indicators during deauth attacks
  - Unauthorized access alerts during ARP spoofing
  - Lock/unlock status changes during command injection

## IoT Device Arduino Code

- Develop different Arduino sketches for ESP8266/NodeMCU:
  - Weather station with DHT sensor and LCD display
  - Smart lock with relay and status LEDs
  - Security system with motion sensor and buzzer
  - Home automation with multiple relays and sensors
- Implement MQTT communication for all devices
- Add visual/audio feedback for attack detection

## References

- [ESP32 Marauder](https://github.com/justcallmekoko/ESP32Marauder)
- [ESP Ghost](https://github.com/justcallmekoko/ESP-Ghost)
- [Flipper Zero](https://flipperzero.one/)

## Important Reminders

- Do not rebuild from scratch, continue enhancing the existing structure
- Focus on modular, well-commented code
- Optimize for ESP32-WROOM-32D performance
- Implement proper error handling
- Follow OOP best practices
- Keep security educational focus
- Test on real IoT devices with proper permission
- Document attack impacts on real devices
- Ensure all testing is done ethically and responsibly
