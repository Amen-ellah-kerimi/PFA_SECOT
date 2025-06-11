import os
import json
import time
from datetime import datetime
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
import paho.mqtt.client as mqtt
import threading
import logging
from threading import Lock

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__, static_folder='../frontend/dist')
CORS(app)  # Enable CORS for all routes

# MQTT Broker configurations
MQTT_CONFIGS = [
    # No security
    {
        'host': '192.168.1.100',
        'port': 1883,
        'username': '',
        'password': '',
        'use_tls': False
    },
    # Basic security
    {
        'host': '192.168.1.100',
        'port': 1884,
        'username': 'smart_light',
        'password': '1234',
        'use_tls': False
    },
    # Secure TLS
    {
        'host': '192.168.1.100',
        'port': 8883,
        'username': 'smart_light',
        'password': 'SmartLight2024!',
        'use_tls': True
    }
]

# MQTT Topics
TOPICS = {
    'state': 'home/smartlight/state',
    'command': 'home/smartlight/command',
    'brightness': 'home/smartlight/brightness',
    'color': 'home/smartlight/color',
    'motion': 'home/smartlight/motion',
    'status': 'home/smartlight/status'
}

# Global state
mqtt_client = None
current_config_index = 0
connection_status = 'disconnected'
light_state = {
    'state': 'OFF',
    'brightness': 0,
    'color': {'r': 0, 'g': 0, 'b': 0},
    'motion': False
}
state_lock = Lock()

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    global connection_status
    if rc == 0:
        logger.info("Connected to MQTT broker")
        connection_status = 'connected'
        # Subscribe to all topics
        for topic in TOPICS.values():
            client.subscribe(topic)
    else:
        logger.error(f"Failed to connect to MQTT broker with code: {rc}")
        connection_status = 'error'
        try_next_config()

def on_disconnect(client, userdata, rc):
    global connection_status
    logger.info("Disconnected from MQTT broker")
    connection_status = 'disconnected'

def on_message(client, userdata, msg):
    global light_state
    try:
        payload = msg.payload.decode()
        if msg.topic == TOPICS['state']:
            with state_lock:
                data = json.loads(payload)
                light_state.update(data)
        elif msg.topic == TOPICS['motion']:
            with state_lock:
                light_state['motion'] = payload == '1'
    except Exception as e:
        logger.error(f"Error processing message: {e}")

# Initialize MQTT client
def init_mqtt():
    global mqtt_client

    # Create MQTT client with proper API version
    mqtt_client = mqtt.Client()
    mqtt_client._client_id = f'flask_backend_{int(time.time())}'

    # Set callbacks
    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_message = on_message

    # Set credentials if provided
    if MQTT_CONFIGS[current_config_index]['username'] and MQTT_CONFIGS[current_config_index]['password']:
        mqtt_client.username_pw_set(MQTT_CONFIGS[current_config_index]['username'], MQTT_CONFIGS[current_config_index]['password'])

    # Set TLS if enabled
    if MQTT_CONFIGS[current_config_index]['use_tls']:
        mqtt_client.tls_set()

    # Connect to broker
    try:
        mqtt_client.connect(MQTT_CONFIGS[current_config_index]['host'], MQTT_CONFIGS[current_config_index]['port'], 60)
        mqtt_client.loop_start()
        logger.info(f"Connecting to MQTT broker at {MQTT_CONFIGS[current_config_index]['host']}:{MQTT_CONFIGS[current_config_index]['port']}")
    except Exception as e:
        logger.error(f"Failed to connect to MQTT broker: {e}")

# API routes
@app.route('/api/status', methods=['GET'])
def get_status():
    with state_lock:
        return jsonify({
            'connection_status': connection_status,
            'light_state': light_state
        })

@app.route('/api/data', methods=['GET'])
def get_data():
    return jsonify({
        'light_state': light_state,
        'brightness_history': [],
        'motion_history': [],
        'device_status': {},
        'last_update': None
    })

@app.route('/api/light', methods=['GET'])
def get_light():
    return jsonify(light_state)

@app.route('/api/brightness', methods=['GET'])
def get_brightness_history():
    return jsonify([])

@app.route('/api/motion', methods=['GET'])
def get_motion_history():
    return jsonify([])

@app.route('/api/device', methods=['GET'])
def get_device():
    return jsonify({})

@app.route('/api/publish', methods=['POST'])
def publish_message():
    if connection_status != 'connected':
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
    if connection_status != 'connected':
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'command' not in data:
        return jsonify({'success': False, 'error': 'Missing command'}), 400

    command = data['command']

    try:
        mqtt_client.publish(TOPICS['command'], command)
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error sending command: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/brightness', methods=['POST'])
def set_brightness():
    if connection_status != 'connected':
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'brightness' not in data:
        return jsonify({'success': False, 'error': 'Missing brightness value'}), 400

    brightness = data['brightness']

    try:
        mqtt_client.publish(TOPICS['brightness'], str(brightness))
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error setting brightness: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/color', methods=['POST'])
def set_color():
    if connection_status != 'connected':
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    data = request.json
    if not data or 'color' not in data:
        return jsonify({'success': False, 'error': 'Missing color value'}), 400

    color = data['color']

    try:
        mqtt_client.publish(TOPICS['color'], json.dumps(color))
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error setting color: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/clear', methods=['POST'])
def clear_data():
    return jsonify({'success': True})

@app.route('/api/light/toggle', methods=['POST'])
def toggle_light():
    if connection_status != 'connected':
        return jsonify({'success': False, 'error': 'Not connected to MQTT broker'}), 503

    try:
        new_state = 'OFF' if light_state['state'] == 'ON' else 'ON'
        mqtt_client.publish(TOPICS['command'], new_state)
        return jsonify({'success': True})
    except Exception as e:
        logger.error(f"Error toggling light: {e}")
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/api/reconnect', methods=['POST'])
def reconnect():
    init_mqtt()
    return jsonify({'success': True})

@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def serve(path):
    if path != "" and os.path.exists(app.static_folder + '/' + path):
        return send_from_directory(app.static_folder, path)
    else:
        return send_from_directory(app.static_folder, 'index.html')

if __name__ == '__main__':
    init_mqtt()
    app.run(host='0.0.0.0', port=5000, debug=True)
