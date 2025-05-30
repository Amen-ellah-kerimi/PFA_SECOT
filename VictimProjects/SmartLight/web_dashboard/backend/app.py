import os
import json
import time
from datetime import datetime
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
import paho.mqtt.client as mqtt
import threading
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__, static_folder='../frontend/dist')
CORS(app)  # Enable CORS for all routes

# MQTT settings
mqtt_settings = {
    'broker': os.environ.get('MQTT_BROKER', 'localhost'),
    'port': int(os.environ.get('MQTT_PORT', 1883)),
    'username': os.environ.get('MQTT_USERNAME', ''),
    'password': os.environ.get('MQTT_PASSWORD', ''),
    'client_id': f'flask_backend_{int(time.time())}',
    'use_tls': os.environ.get('MQTT_USE_TLS', 'false').lower() == 'true',
    'topics': {
        'state': 'home/smartlight/state',
        'command': 'home/smartlight/command',
        'brightness': 'home/smartlight/brightness',
        'color': 'home/smartlight/color',
        'ambient': 'home/smartlight/ambient',
        'motion': 'home/smartlight/motion',
        'status': 'home/smartlight/status'
    }
}

# In-memory data store
data_store = {
    'light_state': {
        'state': 'OFF',
        'brightness': 255,
        'color': {'r': 255, 'g': 255, 'b': 255},
        'ambient': 0,
        'motion': False,
        'timestamp': 0
    },
    'brightness_history': [],
    'ambient_history': [],
    'motion_history': [],
    'device_status': {},
    'last_update': None
}

# Maximum number of readings to keep
MAX_READINGS = 100

# MQTT client
mqtt_client = None
mqtt_connected = False

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    global mqtt_connected
    if rc == 0:
        logger.info("Connected to MQTT broker")
        mqtt_connected = True

        # Subscribe to topics
        for topic in mqtt_settings['topics'].values():
            client.subscribe(topic)
            logger.info(f"Subscribed to {topic}")
    else:
        logger.error(f"Failed to connect to MQTT broker with code {rc}")
        mqtt_connected = False

def on_disconnect(client, userdata, rc):
    global mqtt_connected
    logger.info("Disconnected from MQTT broker")
    mqtt_connected = False

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode('utf-8')
    logger.debug(f"Received message on {topic}: {payload}")

    try:
        # Update last update time
        data_store['last_update'] = datetime.now().isoformat()

        # Process message based on topic
        if topic == mqtt_settings['topics']['state']:
            state_data = json.loads(payload)
            data_store['light_state'] = state_data

            # Add to history
            if 'brightness' in state_data:
                data_store['brightness_history'].append({
                    'value': state_data['brightness'],
                    'timestamp': datetime.now().isoformat()
                })
                # Limit the number of readings
                if len(data_store['brightness_history']) > MAX_READINGS:
                    data_store['brightness_history'] = data_store['brightness_history'][-MAX_READINGS:]

            if 'ambient' in state_data:
                data_store['ambient_history'].append({
                    'value': state_data['ambient'],
                    'timestamp': datetime.now().isoformat()
                })
                # Limit the number of readings
                if len(data_store['ambient_history']) > MAX_READINGS:
                    data_store['ambient_history'] = data_store['ambient_history'][-MAX_READINGS:]

            if 'motion' in state_data:
                data_store['motion_history'].append({
                    'value': state_data['motion'],
                    'timestamp': datetime.now().isoformat()
                })
                # Limit the number of readings
                if len(data_store['motion_history']) > MAX_READINGS:
                    data_store['motion_history'] = data_store['motion_history'][-MAX_READINGS:]

        elif topic == mqtt_settings['topics']['ambient']:
            ambient = int(payload)
            data_store['light_state']['ambient'] = ambient

            # Add to history
            data_store['ambient_history'].append({
                'value': ambient,
                'timestamp': datetime.now().isoformat()
            })
            # Limit the number of readings
            if len(data_store['ambient_history']) > MAX_READINGS:
                data_store['ambient_history'] = data_store['ambient_history'][-MAX_READINGS:]

        elif topic == mqtt_settings['topics']['motion']:
            motion = payload == '1'
            data_store['light_state']['motion'] = motion

            # Add to history
            data_store['motion_history'].append({
                'value': motion,
                'timestamp': datetime.now().isoformat()
            })
            # Limit the number of readings
            if len(data_store['motion_history']) > MAX_READINGS:
                data_store['motion_history'] = data_store['motion_history'][-MAX_READINGS:]

        elif topic == mqtt_settings['topics']['status']:
            data_store['device_status'] = json.loads(payload)

    except Exception as e:
        logger.error(f"Error processing message: {e}")

# Initialize MQTT client
def init_mqtt():
    global mqtt_client

    # Create MQTT client with proper API version
    mqtt_client = mqtt.Client(client_id=mqtt_settings['client_id'])
    mqtt_client._client_id = mqtt_settings['client_id']

    # Set callbacks
    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_message = on_message

    # Set credentials if provided
    if mqtt_settings['username'] and mqtt_settings['password']:
        mqtt_client.username_pw_set(mqtt_settings['username'], mqtt_settings['password'])

    # Set TLS if enabled
    if mqtt_settings['use_tls']:
        mqtt_client.tls_set()

    # Connect to broker
    try:
        mqtt_client.connect(mqtt_settings['broker'], mqtt_settings['port'], 60)
        mqtt_client.loop_start()
        logger.info(f"Connecting to MQTT broker at {mqtt_settings['broker']}:{mqtt_settings['port']}")
    except Exception as e:
        logger.error(f"Failed to connect to MQTT broker: {e}")

# API routes
@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify({
        'mqtt_connected': mqtt_connected,
        'broker': mqtt_settings['broker'],
        'port': mqtt_settings['port'],
        'last_update': data_store['last_update'],
        'readings_count': {
            'brightness': len(data_store['brightness_history']),
            'ambient': len(data_store['ambient_history']),
            'motion': len(data_store['motion_history'])
        },
        'device_status': data_store['device_status']
    })

@app.route('/api/data', methods=['GET'])
def get_data():
    return jsonify({
        'light_state': data_store['light_state'],
        'brightness_history': data_store['brightness_history'],
        'ambient_history': data_store['ambient_history'],
        'motion_history': data_store['motion_history'],
        'device_status': data_store['device_status'],
        'last_update': data_store['last_update']
    })

@app.route('/api/light', methods=['GET'])
def get_light():
    return jsonify(data_store['light_state'])

@app.route('/api/brightness', methods=['GET'])
def get_brightness_history():
    return jsonify(data_store['brightness_history'])

@app.route('/api/ambient', methods=['GET'])
def get_ambient_history():
    return jsonify(data_store['ambient_history'])

@app.route('/api/motion', methods=['GET'])
def get_motion_history():
    return jsonify(data_store['motion_history'])

@app.route('/api/device', methods=['GET'])
def get_device():
    return jsonify(data_store['device_status'])

@app.route('/api/publish', methods=['POST'])
def publish_message():
    if not mqtt_connected:
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'topic' not in data or 'message' not in data:
        return jsonify({'success': False, 'error': 'Missing topic or message'}), 400

    topic = data['topic']
    message = data['message']

    try:
        mqtt_client.publish(topic, message)
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error publishing message: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/command', methods=['POST'])
def send_command():
    if not mqtt_connected:
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'command' not in data:
        return jsonify({'success': False, 'error': 'Missing command'}), 400

    command = data['command']

    try:
        mqtt_client.publish(mqtt_settings['topics']['command'], command)
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error sending command: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/brightness', methods=['POST'])
def set_brightness():
    if not mqtt_connected:
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'brightness' not in data:
        return jsonify({'success': False, 'error': 'Missing brightness value'}), 400

    brightness = data['brightness']

    try:
        mqtt_client.publish(mqtt_settings['topics']['brightness'], str(brightness))
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error setting brightness: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/color', methods=['POST'])
def set_color():
    if not mqtt_connected:
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'color' not in data:
        return jsonify({'success': False, 'error': 'Missing color value'}), 400

    color = data['color']

    try:
        mqtt_client.publish(mqtt_settings['topics']['color'], json.dumps(color))
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error setting color: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/clear', methods=['POST'])
def clear_data():
    data_store['brightness_history'] = []
    data_store['ambient_history'] = []
    data_store['motion_history'] = []
    return jsonify({'success': True})

# Serve React app
@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def serve(path):
    if path != "" and os.path.exists(app.static_folder + '/' + path):
        return send_from_directory(app.static_folder, path)
    else:
        return send_from_directory(app.static_folder, 'index.html')

# Initialize MQTT client on startup
init_mqtt()

if __name__ == '__main__':
    # Get port from environment variable or use default
    port = int(os.environ.get('PORT', 5000))

    # Run the app
    app.run(host='0.0.0.0', port=port, debug=True)
