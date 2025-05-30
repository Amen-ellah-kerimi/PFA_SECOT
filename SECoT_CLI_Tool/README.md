# SECoT CLI Tool

## Overview
The SECoT CLI Tool is a command-line interface designed to interact with the SECoT ESP32 card. It allows users to connect to the SECoT device via serial port or MQTT, control the SECoT card, retrieve data, and perform various security-related operations such as network scanning, port scanning, and launching attacks supported by the SECoT card.

## Features
- **Interactive Mode**: Provides a REPL (Read-Eval-Print Loop) for user-friendly interaction.
- **Serial Port Communication**: Connect directly to the SECoT device via serial port.
- **Auto-Detection**: Automatically detect and connect to SECoT devices connected via USB.
- **MQTT Integration**: Connect to an MQTT broker or launch a local broker for communication.
- **Network Scanning**: Discover Wi-Fi networks in the vicinity.
- **Port Scanning**: Scan open ports on a target IP address.
- **Attack Execution**: Trigger attacks supported by the SECoT card (e.g., ARP spoofing, deauthentication).
- **Output Formatting**: Display results in table or JSON format.

## Project Structure
```
SECoT_CLI_Tool/
├── src/
│   ├── main.rs          # Entry point for the CLI tool
│   ├── command/         # Command implementations
│   │   ├── mod.rs       # Command module
│   │   ├── scan.rs,...  # Implementation of scanNetworks
│   ├── models/          # Data models
│   │   ├── mod.rs       # Models module
│   │   ├── device.rs    # Device-related logic
│   │   ├── network.rs   # Network scanning logic
│   │   ├── port.rs,...  # Port scanning logic
│   ├── mqtt/            # MQTT communication logic
│   ├── output/          # Output formatting
│   ├── config/          # Configuration management
│   ├── tests/           # Unit tests
├── Cargo.toml           # Rust project configuration
├── README.md            # Project documentation
```

## Example Usage
### Interactive Mode
```
IoT Security Audit Tool - Interactive Mode
Type 'help' for available commands, 'exit' to quit

iot> scanNetworks --band 2.4
Scanning networks...

=== Command Result ===
+-------------+-------------------+---------+----------+
|    SSID     |       BSSID       | Signal  | Security |
+-------------+-------------------+---------+----------+
|   HomeWiFi  | 00:11:22:33:44:55 |  -65    |   WPA2   |
| GuestAccess | AA:BB:CC:DD:EE:FF |  -72    |   OPEN   |
+-------------+-------------------+---------+----------+
=====================

iot> portScan 192.168.1.1 --ports 1-100
Scanning ports...

=== Command Result ===
+-----------+------------+---------------------+
|   Port    |   Status   |      Service       |
+-----------+------------+---------------------+
|     22    |    open    |        ssh         |
|     80    |    open    |        http        |
|     443   |    open    |        https       |
+-----------+------------+---------------------+
=====================

iot> exit
Disconnected from MQTT broker.
```

## Commands
### `scanNetworks`
- **Description**: Scan for Wi-Fi networks.
- **Usage**: `scanNetworks --band <2.4|5>`

### `portScan`
- **Description**: Scan open ports on a target IP.
- **Usage**: `portScan <IP> --ports <range>`

### `set output`
- **Description**: Set the output format (e.g., table, JSON).
- **Usage**: `set output <format>`

### `exit`
- **Description**: Exit the interactive mode.

## Models
### `models/network.rs`
- **Purpose**: Handles network scanning logic.
- **Key Functions**:
  - `scan_networks(band: &str) -> Vec<Network>`: Scans for Wi-Fi networks.

### `models/port.rs`
- **Purpose**: Handles port scanning logic.
- **Key Functions**:
  - `scan_ports(ip: &str, range: &str) -> Vec<Port>`: Scans open ports on a target IP.

### `models/device.rs`
- **Purpose**: Manages device-related logic.
- **Key Functions**:
  - `connect_to_device()`: Connects to the SECoT card.

## Complete Feature List

The SECoT CLI Tool aims to provide a comprehensive set of features for interacting with the SECoT ESP32 card. Below is the complete list of commands and their purposes:

### Commands
#### Network Scanning
- **`scanNetworks`**: Scan for Wi-Fi networks in the vicinity.
- **`scan_ports`**: Scan open ports on a target IP address.

#### MQTT Operations
- **`mqttPublish`**: Publish messages to an MQTT topic.
- **`mqttSubscribe`**: Subscribe to an MQTT topic and handle incoming messages.
- **`mqttScan`**: Scan for active MQTT brokers on the network.

#### Attack Execution
- **`deauth`**: Perform a deauthentication attack.
- **`beaconFlood`**: Execute a beacon flood attack.
- **`probeSpam`**: Send probe request spam.
- **`arpSpoof`**: Perform an ARP spoofing attack.
- **`mqttSpoof`**: Spoof MQTT messages.
- **`evilTwin`**: Set up an evil twin access point.
- **`passiveSniff`**: Passively sniff network traffic.
- **`bluetoothScan`**: Scan for Bluetooth devices.
- **`jamming`**: Perform signal jamming attacks.

#### Utility Commands
- **`set output`**: Set the output format (e.g., table, JSON).
- **`exit`**: Exit the interactive mode.

#### Testing and Debugging
- **`broker_test`**: Test the MQTT broker connection.

## Future Goals
The SECoT CLI Tool is designed to be extensible. Future updates may include:
- Enhanced attack configurations.
- Real-time monitoring dashboards.
- Integration with external APIs for threat intelligence.
- Support for additional IoT protocols (e.g., CoAP, Zigbee).

## Development
For development notes and tasks, refer to `DEVELOPMENT_NOTES.md`.
