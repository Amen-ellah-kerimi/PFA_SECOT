# IoT Security Audit System

## 🔐 Project Overview
This project is a comprehensive IoT security audit system designed for educational purposes in cybersecurity. It demonstrates common vulnerabilities in IoT systems and how to secure them properly. The system consists of:

1. **SECoT (Security Evaluation and Compromise Toolkit)** - ESP32-based security audit tool for demonstrating attacks
2. **Victim IoT Projects** - Real-world IoT implementations with intentional security flaws
3. **Secure Versions** - Properly secured implementations of the same IoT projects

## 🎯 Educational Goals

- Demonstrate real-world IoT security vulnerabilities
- Showcase attack vectors and exploitation techniques
- Provide secure implementations as a reference
- Raise awareness about IoT security best practices
- Create a hands-on learning environment for IoT security concepts

## 📁 Project Structure

```text
project-root/
│
├── SECoT/                          # Security Evaluation and Compromise Toolkit
│   ├── src/                        # Modular C++ source code
│   │   ├── attacks/                # Attack implementation modules
│   │   ├── config/                 # Configuration settings
│   │   ├── interfaces/             # User interface implementations
│   │   └── utils/                  # Utility functions and helpers
│   ├── data/                       # Data files used by SECoT
│   ├── flutter_apps/               # Flutter-based mobile interface (optional)
│   ├── SECoT.ino                   # Main Arduino sketch file
│   └── web interface files         # HTML, CSS, JS for web control interface
│
├── SECoT_CLI_Tool/                 # Command-line interface tool for SECoT
│   ├── src/                        # Source code for the CLI tool
│   ├── config.json                 # Configuration file for the CLI tool
│   ├── Cargo.toml                  # Rust project manifest
│   ├── Cargo.lock                  # Lock file for dependencies
│   ├── DEVELOPMENT_NOTES.md        # Development notes for the CLI tool
│   └── README.md                   # Documentation for the CLI tool
│
└── VictimProjects/                 # Vulnerable IoT project implementations
    ├── WeatherStation/             # Weather monitoring system
    │   ├── nodemcu_code/           # ESP8266 Arduino code
    │   ├── flutter_app/            # Mobile app for monitoring
    │   ├── web_dashboard/          # Web interface for monitoring
    │   └── deployment/             # Deployment scripts and configurations
    │
    └── SmartLight/                 # Smart lighting control system
        ├── nodemcu_code/           # ESP8266 Arduino code
        ├── flutter_app/            # Mobile app for control
        ├── web_dashboard/          # Web interface for control
        └── deployment/             # Deployment scripts and configurations
```

## 🛠️ Technologies Used

- **Hardware**:
  - ESP32 (for SECoT security audit tool)
  - ESP8266/NodeMCU (for victim IoT devices)
  - Various sensors (DHT22, LDR, PIR motion sensors)
  - Actuators (LEDs, relays)

- **Firmware/Backend**:
  - Arduino framework (C/C++)
  - MQTT protocol for IoT communication
  - Python Flask for web dashboard backends
  - Rust (for SECoT_CLI_Tool)

- **Frontend**:
  - Flutter for cross-platform mobile apps
  - React with Vite for web dashboards
  - Chart.js for data visualization

- **Deployment**:
  - Docker and Docker Compose
  - Mosquitto MQTT broker
  - Nginx web server (optional)

## 🧪 Security Features & Attacks

### Attack Capabilities (SECoT)

- **WiFi Attacks**: Deauthentication, beacon flooding, probe request spamming
- **MQTT Attacks**: Broker scanning, message spoofing, fake device emulation
- **Network Attacks**: ARP spoofing, passive sniffing, evil twin
- **Multiple Interfaces**: Serial terminal, TFT screen, and web interface

### Security Principles (Secure Versions)

- TLS/SSL encryption for MQTT and HTTP
- Strong authentication (username/password, JWT)
- Message integrity (checksums, hashing)
- Input validation and sanitization
- Secure storage of credentials
- Access control and authorization

## 🔄 Victim Projects

Each victim project has both insecure and secure implementations:

### Weather Station

- Temperature and humidity monitoring
- Historical data visualization
- Mobile and web interfaces
- MQTT-based communication

### Smart Light Controller

- RGB LED control with adjustable color and brightness
- Scene presets and scheduling
- Motion detection integration
- Mobile and web control interfaces

## 📚 Getting Started

1. **Set up SECoT**:
   - Follow instructions in `SECoT/README.md`
   - Configure hardware connections
   - Upload firmware using Arduino IDE

2. **Set up SECoT_CLI_Tool**:
   - Follow instructions in `SECoT_CLI_Tool/README.md`
   - Ensure Rust is installed
   - Build and run the CLI tool using Cargo

3. **Deploy a Victim Project**:
   - Choose a project (WeatherStation or SmartLight)
   - Follow instructions in the project's README.md
   - Set up the hardware according to schematics
   - Deploy the MQTT broker using provided Docker scripts
   - Build and run the Flutter app and/or web dashboard

4. **Perform Security Testing**:
   - Use SECoT to demonstrate attacks against the victim project
   - Observe the effects on the victim device and interfaces
   - Deploy the secure version and repeat the tests to demonstrate protection

## 📋 Requirements

- ESP32 development board (for SECoT)
- ESP8266/NodeMCU boards (for victim projects)
- Basic electronic components (sensors, LEDs, resistors, etc.)
- Computer with Arduino IDE
- Docker for deployment
- Flutter SDK for mobile app development
- Modern web browser for dashboards
- Rust (for SECoT_CLI_Tool)

## ⚠️ Disclaimer

This project is intended for educational purposes only in a controlled laboratory environment. The techniques demonstrated should not be used against real systems without explicit permission.
