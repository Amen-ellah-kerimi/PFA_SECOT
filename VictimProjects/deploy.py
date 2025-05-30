#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
from pathlib import Path

# Configuration
PROJECTS = ['WeatherStation', 'SmartLight']
SECURITY_LEVELS = ['no-security', 'basic-security', 'secure']
PORTS = {
    'WeatherStation': {
        'mqtt': 1883,
        'mqtt_secure': 8883,
        'websocket': 9001,
        'websocket_secure': 9443,
        'web': 8080
    },
    'SmartLight': {
        'mqtt': 1884,
        'mqtt_secure': 8884,
        'websocket': 9002,
        'websocket_secure': 9444,
        'web': 8081
    }
}

def run_command(command):
    """Run a shell command and print its output"""
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    for line in process.stdout:
        print(line, end='')
    process.wait()
    return process.returncode

def create_mosquitto_conf(project, security_level):
    """Create mosquitto configuration file for the specified security level"""
    ports = PORTS[project]
    conf_content = f"""# Mosquitto configuration for {project} - {security_level}

# Network settings
listener {ports['mqtt']}
listener {ports['websocket']} 0.0.0.0
protocol websockets

# Security settings
"""
    
    if security_level == 'no-security':
        conf_content += """allow_anonymous true
"""
    else:
        conf_content += f"""allow_anonymous false
password_file /mosquitto/config/passwd
acl_file /mosquitto/config/acl-{security_level}
"""
    
    if security_level == 'secure':
        conf_content += f"""
# TLS/SSL settings
listener {ports['mqtt_secure']}
listener {ports['websocket_secure']} 0.0.0.0
protocol websockets

cafile /mosquitto/config/certs/ca.crt
certfile /mosquitto/config/certs/server.crt
keyfile /mosquitto/config/certs/server.key
tls_version tlsv1.2
"""
    
    conf_content += """
# Persistence
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
log_dest stdout
log_type all

# Default settings
max_queued_messages 1000
max_packet_size 0
"""
    
    with open(f'VictimProjects/{project}/deployment/mosquitto-{security_level}.conf', 'w') as f:
        f.write(conf_content)

def create_docker_compose(project, security_level):
    """Create docker-compose.yml for the specified project and security level"""
    ports = PORTS[project]
    compose_content = f"""version: '3.8'

services:
  mqtt:
    image: eclipse-mosquitto:latest
    ports:
      - "{ports['mqtt']}:1883"
      - "{ports['websocket']}:9001"
    volumes:
      - ./{project}/deployment/mosquitto-{security_level}.conf:/mosquitto/config/mosquitto.conf
      - ./{project}/deployment/acl-{security_level}:/mosquitto/config/acl
      - ./{project}/deployment/passwd:/mosquitto/config/passwd
      - ./{project}/deployment/data:/mosquitto/data
      - ./{project}/deployment/log:/mosquitto/log
    restart: unless-stopped

  web:
    build:
      context: ../web_dashboard
      dockerfile: Dockerfile
    ports:
      - "{ports['web']}:80"
    environment:
      - MQTT_BROKER=mqtt
      - MQTT_PORT=1883
      - MQTT_WEBSOCKET_PORT=9001
      - MQTT_USERNAME=web_dashboard
      - MQTT_PASSWORD={'5678' if security_level == 'basic-security' else 'WebDashboard2024!' if security_level == 'secure' else ''}
    depends_on:
      - mqtt
    restart: unless-stopped
"""
    
    if security_level == 'secure':
        compose_content += f"""
  nginx:
    image: nginx:alpine
    ports:
      - "{ports['mqtt_secure']}:8883"
      - "{ports['websocket_secure']}:9443"
    volumes:
      - ./{project}/deployment/nginx.conf:/etc/nginx/nginx.conf
      - ./{project}/deployment/certs:/etc/nginx/certs
    depends_on:
      - mqtt
    restart: unless-stopped
"""
    
    with open(f'VictimProjects/{project}/deployment/docker-compose.yml', 'w') as f:
        f.write(compose_content)

def create_nginx_conf(project):
    """Create nginx configuration for secure MQTT"""
    ports = PORTS[project]
    nginx_conf = f"""events {{
    worker_connections 1024;
}}

stream {{
    upstream mqtt_secure {{
        server mqtt:8883;
    }}

    upstream websocket_secure {{
        server mqtt:9443;
    }}

    server {{
        listen {ports['mqtt_secure']} ssl;
        proxy_pass mqtt_secure;

        ssl_certificate /etc/nginx/certs/server.crt;
        ssl_certificate_key /etc/nginx/certs/server.key;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers HIGH:!aNULL:!MD5;
    }}

    server {{
        listen {ports['websocket_secure']} ssl;
        proxy_pass websocket_secure;

        ssl_certificate /etc/nginx/certs/server.crt;
        ssl_certificate_key /etc/nginx/certs/server.key;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers HIGH:!aNULL:!MD5;
    }}
}}
"""
    with open(f'VictimProjects/{project}/deployment/nginx.conf', 'w') as f:
        f.write(nginx_conf)

def setup_security(project, security_level):
    """Setup security files for the specified level"""
    if security_level == 'no-security':
        return

    # Create password file
    if security_level == 'basic-security':
        passwd_content = f"""# Basic security passwords (4-digit PINs)
weather_station:1234
web_dashboard:5678
android_app:9012
admin:0000
"""
    else:  # secure
        passwd_content = f"""# Secure passwords
weather_station:WeatherStation2024!
web_dashboard:WebDashboard2024!
android_app:AndroidApp2024!
admin:Admin2024!
"""

    with open(f'VictimProjects/{project}/deployment/passwd', 'w') as f:
        f.write(passwd_content)

    # Create certificates for secure level
    if security_level == 'secure':
        certs_dir = f'VictimProjects/{project}/deployment/certs'
        os.makedirs(certs_dir, exist_ok=True)
        
        # Generate self-signed certificates
        run_command(f"""openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
            -keyout {certs_dir}/server.key -out {certs_dir}/server.crt \
            -subj "/C=US/ST=State/L=City/O=Organization/CN=localhost" """)
        
        # Copy server certificate as CA certificate for simplicity
        shutil.copy(f'{certs_dir}/server.crt', f'{certs_dir}/ca.crt')

def deploy(project, security_level):
    """Deploy the specified project with the given security level"""
    print(f"\nDeploying {project} with {security_level} security...")
    
    # Create necessary directories
    os.makedirs(f'VictimProjects/{project}/deployment/data', exist_ok=True)
    os.makedirs(f'VictimProjects/{project}/deployment/log', exist_ok=True)
    
    # Create configurations
    create_mosquitto_conf(project, security_level)
    create_docker_compose(project, security_level)
    
    # Setup security
    setup_security(project, security_level)
    
    # Create nginx config for secure level
    if security_level == 'secure':
        create_nginx_conf(project)
    
    # Start services
    os.chdir(f'VictimProjects/{project}/deployment')
    run_command('docker-compose down')
    run_command('docker-compose up -d')
    os.chdir('../../..')

def main():
    """Main deployment function"""
    if len(sys.argv) != 2 or sys.argv[1] not in SECURITY_LEVELS:
        print(f"Usage: {sys.argv[0]} <security-level>")
        print(f"Available security levels: {', '.join(SECURITY_LEVELS)}")
        sys.exit(1)

    security_level = sys.argv[1]
    
    # Stop all running containers
    run_command('docker-compose -f VictimProjects/WeatherStation/deployment/docker-compose.yml down')
    run_command('docker-compose -f VictimProjects/SmartLight/deployment/docker-compose.yml down')
    
    # Deploy both projects
    for project in PROJECTS:
        deploy(project, security_level)
    
    print("\nDeployment completed!")
    print("\nAccess points:")
    for project in PROJECTS:
        ports = PORTS[project]
        print(f"\n{project}:")
        print(f"  Web Dashboard: http://localhost:{ports['web']}")
        print(f"  MQTT: localhost:{ports['mqtt']}")
        print(f"  WebSocket: ws://localhost:{ports['websocket']}")
        if security_level == 'secure':
            print(f"  Secure MQTT: localhost:{ports['mqtt_secure']}")
            print(f"  Secure WebSocket: wss://localhost:{ports['websocket_secure']}")

if __name__ == '__main__':
    main() 