# Development Notes

## Complete Checklist for SECoT CLI Tool

### MQTT Integration
- [ ] Add a command to connect to an MQTT broker.
- [ ] Provide an option to launch a local MQTT broker.
- [ ] Implement MQTT publish functionality (`mqttPublish`).
- [ ] Implement MQTT subscribe functionality (`mqttSubscribe`).
- [ ] Implement MQTT broker scanning (`mqttScan`).

### SECoT Card Control
- [ ] Define commands to trigger specific attacks:
  - [ ] `deauth`: Deauthentication attack.
  - [ ] `beaconFlood`: Beacon flood attack.
  - [ ] `probeSpam`: Probe request spam.
  - [ ] `arpSpoof`: ARP spoofing attack.
  - [ ] `mqttSpoof`: MQTT message spoofing.
  - [ ] `evilTwin`: Evil twin access point.
  - [ ] `passiveSniff`: Passive network sniffing.
  - [ ] `bluetoothScan`: Bluetooth device scanning.
  - [ ] `jamming`: Signal jamming attack.

### Network Scanning
- [ ] Implement Wi-Fi network scanning (`scanNetworks`).
- [ ] Implement port scanning (`scan_ports`).

### CLI Features
- [ ] Enhance the REPL with better error handling and help messages.
- [ ] Add support for command-line arguments for non-interactive usage.
- [ ] Implement output formatting options (e.g., table, JSON).

### Testing and Debugging
- [ ] Write unit tests for all modules.
- [ ] Test MQTT communication with the SECoT ESP32 card.
- [ ] Debug and fix any issues with command execution.
- [ ] Implement `broker_test` for testing MQTT broker connections.

### Documentation
- [ ] Update the `README.md` with new features and examples.
- [ ] Add detailed comments to the code for clarity.

# SECoT CLI Tool Development Notes

## Implementation Status

### Completed Features
- [x] Basic CLI structure with command handling
- [x] MQTT broker integration
- [x] Network scanning functionality
- [x] Port scanning functionality
- [x] Serial port communication with SECoT device
- [x] Auto-detection of SECoT devices
- [x] JSON and table output formatting

### In Progress
- [ ] Comprehensive attack module implementation
- [ ] Enhanced error handling and recovery
- [ ] Improved documentation and examples

### Future Enhancements
- [ ] Add support for additional IoT protocols (e.g., CoAP, Zigbee).
- [ ] Integrate with external APIs for threat intelligence.
- [ ] Develop real-time monitoring dashboards.
- [ ] Enhance attack configurations for advanced use cases.
- [ ] Add support for secure communication (TLS/SSL) with SECoT device

## Notes
- Use the `tokio` crate for asynchronous operations.
- Use the `clap` crate for command-line argument parsing.
- Use the `prettytable-rs` crate for table formatting.
- Ensure compatibility with the SECoT ESP32 card's MQTT protocol.
