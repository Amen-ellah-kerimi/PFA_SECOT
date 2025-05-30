#!/usr/bin/env python3
"""
MQTT Broker Deployment Script
This script automates the deployment of MQTT broker scenarios using Docker.
"""

import os
import sys
import time
import json
import docker
import argparse
from pathlib import Path

class MQTTBrokerDeployer:
    def __init__(self):
        self.client = docker.from_env()
        self.scenarios = {
            'no_security': {
                'image': 'eclipse-mosquitto:latest',
                'config': 'mosquitto_no_security.conf',
                'port': 1883,
                'volumes': {
                    '/mosquitto/config': {'bind': '/mosquitto/config', 'mode': 'ro'},
                    '/mosquitto/data': {'bind': '/mosquitto/data', 'mode': 'rw'},
                    '/mosquitto/log': {'bind': '/mosquitto/log', 'mode': 'rw'}
                }
            },
            'basic_security': {
                'image': 'eclipse-mosquitto:latest',
                'config': 'mosquitto_basic_security.conf',
                'port': 1884,
                'volumes': {
                    '/mosquitto/config': {'bind': '/mosquitto/config', 'mode': 'ro'},
                    '/mosquitto/data': {'bind': '/mosquitto/data', 'mode': 'rw'},
                    '/mosquitto/log': {'bind': '/mosquitto/log', 'mode': 'rw'}
                }
            },
            'secure': {
                'image': 'eclipse-mosquitto:latest',
                'config': 'mosquitto_secure.conf',
                'port': 8883,
                'volumes': {
                    '/mosquitto/config': {'bind': '/mosquitto/config', 'mode': 'ro'},
                    '/mosquitto/data': {'bind': '/mosquitto/data', 'mode': 'rw'},
                    '/mosquitto/log': {'bind': '/mosquitto/log', 'mode': 'rw'},
                    '/mosquitto/certs': {'bind': '/mosquitto/certs', 'mode': 'ro'}
                }
            }
        }

    def create_config_files(self):
        """Create configuration files for each security scenario."""
        config_dir = Path('config')
        config_dir.mkdir(exist_ok=True)

        # No Security Configuration
        no_security_config = """
listener 1883
allow_anonymous true
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
"""
        with open(config_dir / 'mosquitto_no_security.conf', 'w') as f:
            f.write(no_security_config)

        # Basic Security Configuration
        basic_security_config = """
listener 1884
password_file /mosquitto/config/passwd
acl_file /mosquitto/config/acl
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
"""
        with open(config_dir / 'mosquitto_basic_security.conf', 'w') as f:
            f.write(basic_security_config)

        # Create password file for basic security
        with open(config_dir / 'passwd', 'w') as f:
            f.write('user1:1234\n')  # Weak password for testing

        # Create ACL file for basic security
        acl_config = """
user user1
topic readwrite #
"""
        with open(config_dir / 'acl', 'w') as f:
            f.write(acl_config)

        # Secure Configuration
        secure_config = """
listener 8883
cafile /mosquitto/certs/ca.crt
certfile /mosquitto/certs/server.crt
keyfile /mosquitto/certs/server.key
require_certificate true
password_file /mosquitto/config/passwd_secure
acl_file /mosquitto/config/acl_secure
persistence true
persistence_location /mosquitto/data/
log_dest file /mosquitto/log/mosquitto.log
"""
        with open(config_dir / 'mosquitto_secure.conf', 'w') as f:
            f.write(secure_config)

        # Create secure password file
        with open(config_dir / 'passwd_secure', 'w') as f:
            f.write('secure_user:$7$1$password123$stronghash\n')  # Strong password

        # Create secure ACL file
        secure_acl = """
user secure_user
topic readwrite home/#
topic read SmartLight/#
"""
        with open(config_dir / 'acl_secure', 'w') as f:
            f.write(secure_acl)

    def deploy_scenario(self, scenario_name):
        """Deploy a specific MQTT broker scenario."""
        if scenario_name not in self.scenarios:
            print(f"Error: Unknown scenario '{scenario_name}'")
            return False

        scenario = self.scenarios[scenario_name]
        container_name = f"mqtt_broker_{scenario_name}"

        try:
            # Stop and remove existing container if it exists
            try:
                container = self.client.containers.get(container_name)
                container.stop()
                container.remove()
            except docker.errors.NotFound:
                pass

            # Create and start new container
            container = self.client.containers.run(
                scenario['image'],
                name=container_name,
                ports={f"{scenario['port']}/tcp": scenario['port']},
                volumes=scenario['volumes'],
                detach=True
            )

            print(f"Successfully deployed {scenario_name} MQTT broker on port {scenario['port']}")
            return True

        except Exception as e:
            print(f"Error deploying {scenario_name} scenario: {str(e)}")
            return False

    def deploy_all(self):
        """Deploy all MQTT broker scenarios."""
        self.create_config_files()
        
        for scenario in self.scenarios:
            if not self.deploy_scenario(scenario):
                print(f"Failed to deploy {scenario} scenario")
                return False
        
        print("All scenarios deployed successfully!")
        return True

def main():
    parser = argparse.ArgumentParser(description='Deploy MQTT broker scenarios')
    parser.add_argument('--scenario', choices=['no_security', 'basic_security', 'secure', 'all'],
                      default='all', help='Scenario to deploy')
    
    args = parser.parse_args()
    
    deployer = MQTTBrokerDeployer()
    
    if args.scenario == 'all':
        deployer.deploy_all()
    else:
        deployer.create_config_files()
        deployer.deploy_scenario(args.scenario)

if __name__ == '__main__':
    main() 