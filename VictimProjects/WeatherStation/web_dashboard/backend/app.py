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
    'topics': {
        'temperature': 'home/weatherstation/temperature',
        'humidity': 'home/weatherstation/humidity',
        'status': 'home/weatherstation/status',
        'data': 'home/weatherstation/data'
    }
}

# In-memory data store
data_store = {
    'temperature': [],
    'humidity': [],
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
        if topic == mqtt_settings['topics']['temperature']:
            temperature = float(payload)
            data_store['temperature'].append({
                'value': temperature,
                'timestamp': datetime.now().isoformat()
            })
            # Limit the number of readings
            if len(data_store['temperature']) > MAX_READINGS:
                data_store['temperature'] = data_store['temperature'][-MAX_READINGS:]

        elif topic == mqtt_settings['topics']['humidity']:
            humidity = float(payload)
            data_store['humidity'].append({
                'value': humidity,
                'timestamp': datetime.now().isoformat()
            })
            # Limit the number of readings
            if len(data_store['humidity']) > MAX_READINGS:
                data_store['humidity'] = data_store['humidity'][-MAX_READINGS:]

        elif topic == mqtt_settings['topics']['status']:
            data_store['device_status'] = json.loads(payload)

        elif topic == mqtt_settings['topics']['data']:
            data = json.loads(payload)

            if 'temperature' in data:
                data_store['temperature'].append({
                    'value': float(data['temperature']),
                    'timestamp': datetime.now().isoformat()
                })
                # Limit the number of readings
                if len(data_store['temperature']) > MAX_READINGS:
                    data_store['temperature'] = data_store['temperature'][-MAX_READINGS:]

            if 'humidity' in data:
                data_store['humidity'].append({
                    'value': float(data['humidity']),
                    'timestamp': datetime.now().isoformat()
                })
                # Limit the number of readings
                if len(data_store['humidity']) > MAX_READINGS:
                    data_store['humidity'] = data_store['humidity'][-MAX_READINGS:]

            if 'device_id' in data or 'timestamp' in data:
                data_store['device_status'].update({k: v for k, v in data.items() if k in ['device_id', 'timestamp', 'ip', 'status']})

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
            'temperature': len(data_store['temperature']),
            'humidity': len(data_store['humidity'])
        },
        'device_status': data_store['device_status']
    })

@app.route('/api/data', methods=['GET'])
def get_data():
    return jsonify({
        'temperature': data_store['temperature'],
        'humidity': data_store['humidity'],
        'device_status': data_store['device_status'],
        'last_update': data_store['last_update']
    })

@app.route('/api/temperature', methods=['GET'])
def get_temperature():
    return jsonify(data_store['temperature'])

@app.route('/api/humidity', methods=['GET'])
def get_humidity():
    return jsonify(data_store['humidity'])

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

@app.route('/api/clear', methods=['POST'])
def clear_data():
    data_store['temperature'] = []
    data_store['humidity'] = []
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
