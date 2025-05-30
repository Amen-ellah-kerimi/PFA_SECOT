# Smart Light Control System - Deployment

This directory contains deployment scripts and configuration files for the Smart Light Control System.

## Deployment Options

The system can be deployed in two modes:

1. **Basic Deployment** - No encryption or authentication (for development only)
2. **Secure Deployment** - TLS/SSL encryption and user authentication (recommended for production)

## Prerequisites

- Docker and Docker Compose
- Node.js and npm (for building the frontend)
- OpenSSL (for generating certificates)

## Directory Structure

```bash
deployment/
├── docker-compose.yml           # Basic deployment configuration
├── docker-compose-secure.yml    # Secure deployment configuration
├── mosquitto.conf               # Basic Mosquitto configuration
├── mosquitto-secure.conf        # Secure Mosquitto configuration
├── acl                          # Access control list for Mosquitto
├── passwd                       # Password file for Mosquitto (created during setup)
├── nginx.conf                   # Nginx configuration for secure deployment
├── setup.sh                     # Setup script for Linux/macOS
├── setup.ps1                    # Setup script for Windows
└── certs/                       # SSL/TLS certificates (created during setup)
```

## Setup Instructions

### Linux/macOS

1. Make the setup script executable:
   ```bash
   chmod +x setup.sh
   ```

2. Run the setup script with sudo:
   ```bash
   sudo ./setup.sh
   ```

3. Follow the prompts to complete the setup.

### Windows

1. Open PowerShell as Administrator.

2. Run the setup script:
   ```powershell
   .\setup.ps1
   ```

3. Follow the prompts to complete the setup.

## Manual Deployment

If you prefer to set up the system manually, follow these steps:

### Basic Deployment

1. Create the password file for Mosquitto (if using authentication):
   ```bash
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -c passwd smart_light
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd admin admin123
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd web_dashboard dashboard123
   ```

2. Build the frontend:
   ```bash
   cd ../web_dashboard/frontend
   npm install
   npm run build
   cd ../../deployment
   ```

3. Start the services:
   ```bash
   docker-compose up -d
   ```

### Secure Deployment

1. Generate self-signed certificates:
   ```bash
   mkdir -p certs
   
   # Generate CA key and certificate
   openssl genrsa -out certs/ca.key 2048
   openssl req -new -x509 -days 3650 -key certs/ca.key -out certs/ca.crt -subj "/CN=Smart Light CA"
   
   # Generate server key and certificate
   openssl genrsa -out certs/server.key 2048
   openssl req -new -key certs/server.key -out certs/server.csr -subj "/CN=mqtt.smartlight.local"
   openssl x509 -req -in certs/server.csr -CA certs/ca.crt -CAkey certs/ca.key -CAcreateserial -out certs/server.crt -days 365
   ```

2. Create the password file for Mosquitto (if not already created):
   ```bash
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -c passwd smart_light
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd admin admin123
   docker run --rm -it eclipse-mosquitto mosquitto_passwd -b passwd web_dashboard dashboard123
   ```

3. Build the frontend:
   ```bash
   cd ../web_dashboard/frontend
   npm install
   npm run build
   cd ../../deployment
   ```

4. Start the services with the secure configuration:
   ```bash
   docker-compose -f docker-compose-secure.yml up -d
   ```

## Accessing the System

After deployment, you can access the system at:

- **Web Dashboard**: http://localhost (basic) or https://localhost (secure)
- **Backend API**: http://localhost:5000 (basic) or https://localhost:5443 (secure)
- **MQTT Broker**:
  - MQTT: localhost:1883 (basic) or localhost:8883 (secure)
  - WebSockets: ws://localhost:9001 (basic) or wss://localhost:9443 (secure)

## Security Considerations

The basic deployment is intended for development and testing only. It does not use encryption or authentication, which makes it vulnerable to various attacks.

For production use, always use the secure deployment option, which includes:

- TLS/SSL encryption for all connections
- User authentication for the MQTT broker
- Access control lists to restrict topic access
- Secure default settings

## Troubleshooting

- **Certificate Issues**: If you encounter certificate warnings in your browser, you may need to add the self-signed certificate to your browser's trusted certificates.
- **Connection Refused**: Ensure that the ports are not being used by other services.
- **Authentication Failures**: Check that the username and password in the client match those in the passwd file.

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
