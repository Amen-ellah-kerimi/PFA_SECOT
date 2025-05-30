# SECOT (Security & IoT) - Cybersecurity Attack Simulation & Analysis CLI

This project is a Rust-based Command Line Interface (CLI) tool designed to simplify and orchestrate various Wi-Fi and IoT cybersecurity attack simulations, primarily utilizing Kali Linux tools. It provides an intuitive interface, structured output, and logging capabilities to enhance the understanding and analysis of network vulnerabilities.

## Project Overview

The CLI allows users to:
- Perform Wi-Fi network scanning to discover Access Points and connected clients.
- Execute deauthentication attacks to disconnect devices from networks.
- Deploy Evil Twin access points to trick devices into connecting to attacker-controlled networks.
- Initiate ARP spoofing attacks for Man-in-the-Middle scenarios.
- Simulate various attacks against MQTT brokers, including flooding, fake data publishing, and credential brute-forcing.
- Analyze captured packet data (e.g., MQTT traffic from .pcap files) and review logged attack outcomes.

## Technologies & Tools

### CLI
- **Rust**: For high performance, memory safety, and robust CLI development.
  - `clap`: For powerful command-line argument parsing.
  - `tokio`: Asynchronous runtime for concurrent operations and managing external processes.
  - `colored`: For aesthetically pleasing, colored terminal output.
  - `prettytable-rs`: For displaying structured data in readable tables.
  - `chrono`: For timestamping logs and data.
  - `serde_json`: For structured logging.

### Kali Linux Tools (Orchestrated)
- `aircrack-ng` suite (`airodump-ng`, `aireplay-ng`)
- `ettercap`
- `mdk4` (or `mdk3`)
- `wifiphisher`
- `hydra`
- `john` (John the Ripper)
- `tshark` (Wireshark command-line)
- `mosquitto-clients` (`mosquitto_pub`, `mosquitto_sub`)
- `dnsmasq`, `apache2`, `iptables` (for network setup in Evil Twin)

## Setup and Installation

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/secot.git
cd secot
