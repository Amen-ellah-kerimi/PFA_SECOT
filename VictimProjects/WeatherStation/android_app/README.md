# Weather Station Android App

This Android application provides a modern interface to monitor and visualize weather data from the Weather Station NodeMCU device. It connects to the MQTT broker to receive real-time temperature and humidity readings and displays them in an intuitive dashboard.

## Features

- Real-time temperature and humidity monitoring
- Historical data visualization using line charts
- Material Design UI with modern aesthetics
- MQTT integration for reliable data communication
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
   private static final String CLIENT_ID = "weatherstation-android";
   private static final String USERNAME = "your-username";
   private static final String PASSWORD = "your-password";
   ```
4. Build and run the application

## MQTT Topics

The app subscribes to the following MQTT topics:
- `home/weatherstation/temperature` - Current temperature readings
- `home/weatherstation/humidity` - Current humidity readings
- `home/weatherstation/status` - Device connection status
- `home/weatherstation/data` - Combined sensor data

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
- If charts are not updating:
  - Check MQTT connection status
  - Verify data format in incoming messages
  - Ensure proper JSON parsing

## License

This project is licensed under the MIT License - see the LICENSE file for details. 