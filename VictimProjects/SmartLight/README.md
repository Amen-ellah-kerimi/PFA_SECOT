# IoT Smart Light Control System

A complete IoT Smart Light Control System that demonstrates a real-world IoT implementation with hardware, mobile app, web dashboard, and MQTT communication.

## Project Overview

This project implements a complete IoT Smart Light Control System with the following components:

1. **NodeMCU ESP8266 Device** - Controls RGB LEDs, monitors ambient light levels, and detects motion
2. **Flutter Mobile App** - Provides intuitive control of light color, brightness, and modes
3. **Web Dashboard** - Browser-based interface for monitoring and controlling the smart light
4. **Deployment Scripts** - Docker-based deployment for the MQTT broker and web dashboard

## Directory Structure

- `nodemcu_code/` - Arduino code for the NodeMCU ESP8266
- `flutter_app/` - Flutter mobile application
- `web_dashboard/` - Web-based dashboard (React frontend + Flask backend)
- `deployment/` - Deployment scripts and configuration files

## Hardware Requirements

- NodeMCU ESP8266 development board
- RGB LED (common cathode) or three separate LEDs (red, green, blue)
- LDR (Light Dependent Resistor) for ambient light sensing
- PIR motion sensor (HC-SR501 or similar)
- Resistors: 3 x 220Ω (for LEDs), 1 x 10kΩ (for LDR pull-down)
- Breadboard and jumper wires
- Micro USB cable for programming and power

## Software Requirements

- Arduino IDE (for NodeMCU programming)
- Flutter SDK (for mobile app development)
- Node.js and npm (for web dashboard frontend)
- Python 3.7+ (for web dashboard backend)
- Docker and Docker Compose (for deployment)
- MQTT broker (e.g., Mosquitto)

## Features

### Hardware Features
- RGB LED control with adjustable color and brightness
- Ambient light sensing
- Motion detection
- Real-time status reporting via MQTT

### Mobile App Features
- On/off control
- RGB color picker
- Brightness slider
- Scene presets (e.g., Reading, Movie, Party)
- Scheduling and timers
- Motion detection alerts
- Light usage statistics

### Web Dashboard Features
- Real-time light status monitoring
- Remote control of all light parameters
- Historical data visualization
- Usage analytics
- User management
- Device configuration

## Getting Started

### 1. Set up the NodeMCU device

Follow the instructions in the `nodemcu_code/README.md` file to:
- Connect the hardware components
- Install required Arduino libraries
- Configure WiFi and MQTT settings
- Upload the code to the NodeMCU

### 2. Deploy the MQTT broker and web dashboard

Follow the instructions in the `deployment/README.md` file to:
- Set up the MQTT broker with Docker
- Configure the web dashboard
- Start the services

### 3. Build and run the Flutter app

Follow the instructions in the `flutter_app/README.md` file to:
- Install Flutter dependencies
- Configure MQTT connection settings
- Build and run the app on your device

## System Architecture

```
┌─────────────┐     MQTT     ┌─────────────┐     HTTP     ┌─────────────┐
│  NodeMCU    │ ───────────> │  MQTT       │ <─────────── │  Web        │
│  ESP8266    │ <─ - - - - - │  Broker     │ ─ - - - - -> │  Dashboard  │
└─────────────┘              └─────────────┘              └─────────────┘
       ^                            ^                            ^
       │                            │                            │
       │                            │                            │
       │                            │                            │
       │                            v                            │
       │                     ┌─────────────┐                     │
       └─ - - - - - - - - - │  Flutter    │ - - - - - - - - - - ┘
                            │  App        │
                            └─────────────┘
```

- The NodeMCU reads sensor data and controls the RGB LED
- The NodeMCU publishes state updates and subscribes to command topics
- The Flutter app and web dashboard connect to the MQTT broker to send commands and receive updates
- All components communicate bidirectionally through the MQTT broker

## Security Considerations

This project is designed for educational purposes and includes both secure and insecure deployment options:

- **Basic Deployment**: No encryption or authentication (for development only)
- **Secure Deployment**: TLS/SSL encryption and user authentication

In a production environment, consider implementing:
- Proper certificate management
- Strong authentication mechanisms
- Secure storage of credentials
- Regular security updates
- Network segmentation

## Educational Purpose

This project is part of the SECoT (Security Evaluation and Compromise Toolkit) and is designed to demonstrate:

1. How real IoT systems are built and deployed
2. Communication between IoT devices and applications
3. Security vulnerabilities in IoT systems
4. Best practices for securing IoT deployments

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
