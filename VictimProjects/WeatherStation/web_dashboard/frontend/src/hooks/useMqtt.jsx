import { createContext, useContext, useState, useEffect, useCallback } from 'react';
import mqtt from 'mqtt';

// Create context
const MqttContext = createContext(null);

// MQTT topics
const TEMPERATURE_TOPIC = 'home/weatherstation/temperature';
const HUMIDITY_TOPIC = 'home/weatherstation/humidity';
const STATUS_TOPIC = 'home/weatherstation/status';
const DATA_TOPIC = 'home/weatherstation/data';

// Connection states
export const CONNECTION_STATE = {
  DISCONNECTED: 'disconnected',
  CONNECTING: 'connecting',
  CONNECTED: 'connected',
};

// Provider component
export const MqttProvider = ({ children }) => {
  const [client, setClient] = useState(null);
  const [connectionState, setConnectionState] = useState(CONNECTION_STATE.DISCONNECTED);
  const [error, setError] = useState(null);
  
  // Sensor data
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [temperatureHistory, setTemperatureHistory] = useState([]);
  const [humidityHistory, setHumidityHistory] = useState([]);
  const [deviceStatus, setDeviceStatus] = useState({});
  
  // Connection settings
  const [settings, setSettings] = useState(() => {
    const savedSettings = localStorage.getItem('mqtt_settings');
    return savedSettings ? JSON.parse(savedSettings) : {
      host: 'localhost',
      port: 9001,
      clientId: `web_dashboard_${Math.random().toString(16).substring(2, 8)}`,
      username: '',
      password: '',
      useSSL: false,
    };
  });
  
  // Save settings to localStorage
  useEffect(() => {
    localStorage.setItem('mqtt_settings', JSON.stringify(settings));
  }, [settings]);
  
  // Connect to MQTT broker
  const connect = useCallback(() => {
    if (client) {
      client.end();
    }
    
    setConnectionState(CONNECTION_STATE.CONNECTING);
    setError(null);
    
    const { host, port, clientId, username, password, useSSL } = settings;
    const protocol = useSSL ? 'wss' : 'ws';
    const url = `${protocol}://${host}:${port}/mqtt`;
    
    try {
      const mqttClient = mqtt.connect(url, {
        clientId,
        username: username || undefined,
        password: password || undefined,
        clean: true,
        reconnectPeriod: 5000,
        connectTimeout: 30 * 1000,
      });
      
      mqttClient.on('connect', () => {
        console.log('Connected to MQTT broker');
        setConnectionState(CONNECTION_STATE.CONNECTED);
        
        // Subscribe to topics
        mqttClient.subscribe([
          TEMPERATURE_TOPIC,
          HUMIDITY_TOPIC,
          STATUS_TOPIC,
          DATA_TOPIC,
        ]);
      });
      
      mqttClient.on('error', (err) => {
        console.error('MQTT error:', err);
        setError(err.message);
        setConnectionState(CONNECTION_STATE.DISCONNECTED);
      });
      
      mqttClient.on('offline', () => {
        console.log('MQTT client offline');
        setConnectionState(CONNECTION_STATE.DISCONNECTED);
      });
      
      mqttClient.on('message', (topic, message) => {
        const payload = message.toString();
        console.log(`Received message on ${topic}: ${payload}`);
        
        try {
          if (topic === TEMPERATURE_TOPIC) {
            const temp = parseFloat(payload);
            if (!isNaN(temp)) {
              setTemperature(temp);
              setTemperatureHistory(prev => {
                const newHistory = [...prev, { value: temp, timestamp: new Date() }];
                // Keep only the last 50 readings
                return newHistory.slice(-50);
              });
            }
          } else if (topic === HUMIDITY_TOPIC) {
            const hum = parseFloat(payload);
            if (!isNaN(hum)) {
              setHumidity(hum);
              setHumidityHistory(prev => {
                const newHistory = [...prev, { value: hum, timestamp: new Date() }];
                // Keep only the last 50 readings
                return newHistory.slice(-50);
              });
            }
          } else if (topic === STATUS_TOPIC) {
            const status = JSON.parse(payload);
            setDeviceStatus(status);
          } else if (topic === DATA_TOPIC) {
            const data = JSON.parse(payload);
            
            if (data.temperature !== undefined) {
              const temp = parseFloat(data.temperature);
              if (!isNaN(temp)) {
                setTemperature(temp);
                setTemperatureHistory(prev => {
                  const newHistory = [...prev, { value: temp, timestamp: new Date() }];
                  return newHistory.slice(-50);
                });
              }
            }
            
            if (data.humidity !== undefined) {
              const hum = parseFloat(data.humidity);
              if (!isNaN(hum)) {
                setHumidity(hum);
                setHumidityHistory(prev => {
                  const newHistory = [...prev, { value: hum, timestamp: new Date() }];
                  return newHistory.slice(-50);
                });
              }
            }
            
            if (data.device_id || data.timestamp) {
              setDeviceStatus(prev => ({ ...prev, ...data }));
            }
          }
        } catch (err) {
          console.error('Error processing message:', err);
        }
      });
      
      setClient(mqttClient);
    } catch (err) {
      console.error('MQTT connection error:', err);
      setError(err.message);
      setConnectionState(CONNECTION_STATE.DISCONNECTED);
    }
  }, [client, settings]);
  
  // Disconnect from MQTT broker
  const disconnect = useCallback(() => {
    if (client) {
      client.end();
      setClient(null);
      setConnectionState(CONNECTION_STATE.DISCONNECTED);
    }
  }, [client]);
  
  // Publish message to topic
  const publish = useCallback((topic, message) => {
    if (client && connectionState === CONNECTION_STATE.CONNECTED) {
      client.publish(topic, message);
    }
  }, [client, connectionState]);
  
  // Update connection settings
  const updateSettings = useCallback((newSettings) => {
    setSettings(prev => ({ ...prev, ...newSettings }));
  }, []);
  
  // Clear history data
  const clearHistory = useCallback(() => {
    setTemperatureHistory([]);
    setHumidityHistory([]);
  }, []);
  
  // Context value
  const value = {
    client,
    connectionState,
    error,
    temperature,
    humidity,
    temperatureHistory,
    humidityHistory,
    deviceStatus,
    settings,
    connect,
    disconnect,
    publish,
    updateSettings,
    clearHistory,
  };
  
  return (
    <MqttContext.Provider value={value}>
      {children}
    </MqttContext.Provider>
  );
};

// Custom hook to use the MQTT context
export const useMqtt = () => {
  const context = useContext(MqttContext);
  if (!context) {
    throw new Error('useMqtt must be used within a MqttProvider');
  }
  return context;
};
