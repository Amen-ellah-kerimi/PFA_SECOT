# Weather Station - Deployment

This directory contains deployment scripts and configuration files for the Weather Station project.

## Overview

The deployment consists of two main components:

1. **MQTT Broker** - Eclipse Mosquitto running in a Docker container
2. **Web Dashboard** - Nginx serving the web dashboard files

## Deployment Options

### Basic Deployment (Insecure)

This deployment option is suitable for development and testing. It does not use encryption or authentication.

Files:
- `mosquitto.conf` - Basic Mosquitto configuration
- `docker-compose.yml` - Docker Compose configuration for basic deployment

### Secure Deployment

This deployment option adds TLS/SSL encryption and user authentication. It's recommended for production use.

Files:
- `mosquitto-secure.conf` - Secure Mosquitto configuration with TLS and authentication
- `acl` - Access control list for Mosquitto
- `passwd` - Password file for Mosquitto (generated during setup)
- `nginx.conf` - Nginx configuration with HTTPS
- `docker-compose-secure.yml` - Docker Compose configuration for secure deployment

## Prerequisites

- Docker and Docker Compose
- OpenSSL (for generating certificates)
- Basic knowledge of MQTT and Docker

## Setup Instructions

### Linux/macOS

1. Make the setup script executable:
   ```bash
   chmod +x setup.sh
   ```

2. Run the setup script as root:
   ```bash
   sudo ./setup.sh
   ```

### Windows

1. Open PowerShell as Administrator

2. Run the setup script:
   ```powershell
   .\setup.ps1
   ```

## Manual Setup

If you prefer to set up manually or the setup scripts don't work for your environment:

1. Create a password file for Mosquitto:
   ```bash
   # Linux/macOS
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -c passwd weather_station
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd admin admin123
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd web_dashboard dashboard123
   
   # Windows - use pre-generated password file or install Mosquitto locally to use mosquitto_passwd
   ```

2. Generate self-signed certificates:
   ```bash
   mkdir -p certs
   
   # Generate CA key and certificate
   openssl genrsa -out certs/ca.key 2048
   openssl req -new -x509 -days 3650 -key certs/ca.key -out certs/ca.crt -subj "/CN=Weather Station CA"
   
   # Generate server key and certificate
   openssl genrsa -out certs/server.key 2048
   openssl req -new -key certs/server.key -out certs/server.csr -subj "/CN=mqtt.weatherstation.local"
   openssl x509 -req -in certs/server.csr -CA certs/ca.crt -CAkey certs/ca.key -CAcreateserial -out certs/server.crt -days 365
   ```

3. Start the services:
   ```bash
   # Basic deployment
   docker-compose up -d
   
   # Secure deployment
   docker-compose -f docker-compose-secure.yml up -d
   ```

## Default Credentials

The setup scripts create the following default users:

- `weather_station` / `weather123` - For the NodeMCU device
- `admin` / `admin123` - For administrative access
- `web_dashboard` / `dashboard123` - For the web dashboard

**Important:** Change these passwords in a production environment!

## Accessing the Services

- MQTT Broker: `localhost:1883` (MQTT), `localhost:9001` (WebSocket)
- Web Dashboard: `http://localhost` (Basic) or `https://localhost` (Secure)

## Troubleshooting

- **Docker containers not starting**: Check Docker logs with `docker-compose logs`
- **Cannot connect to MQTT broker**: Verify the broker is running with `docker ps` and check firewall settings
- **Certificate errors**: Ensure certificates are properly generated and mounted in the containers

## Security Considerations

The secure deployment includes:
- TLS/SSL encryption for MQTT and HTTP
- User authentication for MQTT
- Access control lists for MQTT topics
- Secure Nginx configuration with HTTP headers

For production use, consider:
- Using proper CA-signed certificates instead of self-signed
- Implementing stronger passwords
- Configuring firewall rules
- Regular security updates

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
