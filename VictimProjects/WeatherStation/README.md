# IoT Weather Station Project

A complete IoT Weather Station project that demonstrates a real-world IoT system with hardware, mobile app, web dashboard, and MQTT communication.

## Project Overview

This project implements a complete IoT Weather Station system with the following components:

1. **NodeMCU ESP8266 Device** - Reads temperature and humidity data from a DHT22 sensor and publishes it to an MQTT broker
2. **Flutter Mobile App** - Connects to the MQTT broker to display real-time sensor data with charts
3. **Web Dashboard** - Browser-based interface for monitoring the weather station
4. **Deployment Scripts** - Docker-based deployment for the MQTT broker and web dashboard

## Directory Structure

- `nodemcu_code/` - Arduino code for the NodeMCU ESP8266
- `flutter_app/` - Flutter mobile application
- `web_dashboard/` - Web-based dashboard
- `deployment/` - Deployment scripts and configuration files

## Hardware Requirements

- NodeMCU ESP8266 development board
- DHT22 temperature and humidity sensor
- Breadboard and jumper wires
- Micro USB cable for programming and power
- Optional: 10K resistor (for DHT22 pull-up)
- Optional: LED for status indication

## Software Requirements

- Arduino IDE (for NodeMCU programming)
- Flutter SDK (for mobile app development)
- Docker and Docker Compose (for deployment)
- MQTT broker (e.g., Mosquitto)
- Web server (e.g., Nginx)

## Getting Started

### 1. Set up the NodeMCU device

Follow the instructions in the `nodemcu_code/README.md` file to:
- Connect the DHT22 sensor to the NodeMCU
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

- The NodeMCU reads sensor data and publishes it to the MQTT broker
- The Flutter app subscribes to the MQTT broker to receive sensor data
- The web dashboard connects to the MQTT broker via WebSockets
- All components can communicate bidirectionally through the MQTT broker

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
