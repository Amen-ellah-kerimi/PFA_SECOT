# Smart Light Android App

This Android application provides a modern interface to control and monitor the Smart Light NodeMCU device. It connects to the MQTT broker to send commands and receive real-time updates about the light's state, brightness, color temperature, and sensor readings.

## Features

- Power control with on/off toggle
- Brightness adjustment with slider control
- Color temperature control (2700K - 6500K)
- Real-time ambient light monitoring
- Motion detection status
- Material Design UI with modern aesthetics
- MQTT integration for reliable communication
- Connection status monitoring
- Pull-to-refresh functionality

## Requirements

- Android Studio Arctic Fox (2020.3.1) or newer
- Android SDK 21 or higher
- Gradle 7.0 or higher
- Internet connection for MQTT communication

## Setup

1. Clone the repository
2. Open the project in Android Studio
3. Update the MQTT broker configuration in `MqttClient.java`:
   ```java
   private static final String BROKER = "tcp://your-mqtt-broker:1883";
   private static final String CLIENT_ID = "smartlight-android";
   private static final String USERNAME = "your-username";
   private static final String PASSWORD = "your-password";
   ```
4. Build and run the application

## MQTT Topics

The app uses the following MQTT topics:
- `home/smartlight/state` - Current power state (ON/OFF)
- `home/smartlight/command` - Commands to control the light
- `home/smartlight/brightness` - Brightness level (0-100)
- `home/smartlight/color` - Color temperature (2700-6500K)
- `home/smartlight/ambient` - Ambient light sensor readings
- `home/smartlight/motion` - Motion sensor status
- `home/smartlight/status` - Device connection status

## Building the App

1. Open Android Studio
2. Select "Build > Make Project"
3. Connect an Android device or start an emulator
4. Click "Run > Run 'app'"

## Troubleshooting

- If the app fails to connect to the MQTT broker, verify:
  - Internet connection is available
  - MQTT broker is running and accessible
  - Correct broker credentials are configured
  - Firewall is not blocking MQTT traffic (port 1883)
- If controls are not responding:
  - Check MQTT connection status
  - Verify topic names match the NodeMCU configuration
  - Ensure proper message format for commands

## License

This project is licensed under the MIT License - see the LICENSE file for details. 