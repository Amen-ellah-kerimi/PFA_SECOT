import mqtt from 'mqtt';

class MqttService {
  constructor() {
    this.client = null;
    this.broker = process.env.MQTT_BROKER || 'localhost';
    this.port = process.env.MQTT_PORT || 1884;  // Different port for SmartLight
    this.username = process.env.MQTT_USERNAME || '';
    this.password = process.env.MQTT_PASSWORD || '';
    this.topics = {
      state: 'home/smartlight/state',
      command: 'home/smartlight/command',
      brightness: 'home/smartlight/brightness',
      color: 'home/smartlight/color',
      ambient: 'home/smartlight/ambient',
      motion: 'home/smartlight/motion',
      status: 'home/smartlight/status'
    };
  }

  connect() {
    const options = {
      clientId: `web_${Math.random().toString(16).slice(3)}`,
      clean: true,
      connectTimeout: 4000,
      reconnectPeriod: 1000,
    };

    if (this.username && this.password) {
      options.username = this.username;
      options.password = this.password;
    }

    this.client = mqtt.connect(`ws://${this.broker}:${this.port}`, options);

    this.client.on('connect', () => {
      console.log('Connected to MQTT broker');
      this.subscribeToTopics();
    });

    this.client.on('error', (error) => {
      console.error('MQTT Error:', error);
    });

    this.client.on('close', () => {
      console.log('MQTT connection closed');
    });

    return this.client;
  }

  subscribeToTopics() {
    Object.values(this.topics).forEach(topic => {
      this.client.subscribe(topic, (err) => {
        if (err) {
          console.error(`Error subscribing to ${topic}:`, err);
        } else {
          console.log(`Subscribed to ${topic}`);
        }
      });
    });
  }

  publish(topic, message) {
    if (this.client && this.client.connected) {
      this.client.publish(topic, JSON.stringify(message));
    }
  }

  disconnect() {
    if (this.client) {
      this.client.end();
    }
  }
}

export default new MqttService(); 