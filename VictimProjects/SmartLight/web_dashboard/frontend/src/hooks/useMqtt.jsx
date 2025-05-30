import { createContext, useContext, useState, useEffect, useCallback } from 'react';
import mqtt from 'mqtt';

// Create context
const MqttContext = createContext(null);

// MQTT topics
const LIGHT_STATE_TOPIC = 'home/smartlight/state';
const LIGHT_COMMAND_TOPIC = 'home/smartlight/command';
const LIGHT_BRIGHTNESS_TOPIC = 'home/smartlight/brightness';
const LIGHT_COLOR_TOPIC = 'home/smartlight/color';
const LIGHT_AMBIENT_TOPIC = 'home/smartlight/ambient';
const LIGHT_MOTION_TOPIC = 'home/smartlight/motion';
const STATUS_TOPIC = 'home/smartlight/status';

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
  
  // Light state
  const [lightState, setLightState] = useState({
    state: 'OFF',
    brightness: 255,
    color: { r: 255, g: 255, b: 255 },
    ambient: 0,
    motion: false,
    timestamp: 0
  });
  
  // History data
  const [brightnessHistory, setBrightnessHistory] = useState([]);
  const [ambientHistory, setAmbientHistory] = useState([]);
  const [motionHistory, setMotionHistory] = useState([]);
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
          LIGHT_STATE_TOPIC,
          LIGHT_AMBIENT_TOPIC,
          LIGHT_MOTION_TOPIC,
          STATUS_TOPIC,
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
          if (topic === LIGHT_STATE_TOPIC) {
            const state = JSON.parse(payload);
            setLightState(state);
            
            // Add to history
            if (state.brightness !== undefined) {
              setBrightnessHistory(prev => {
                const newHistory = [...prev, { value: state.brightness, timestamp: new Date() }];
                // Keep only the last 50 readings
                return newHistory.slice(-50);
              });
            }
          } else if (topic === LIGHT_AMBIENT_TOPIC) {
            const ambient = parseInt(payload);
            setLightState(prev => ({ ...prev, ambient }));
            
            // Add to history
            setAmbientHistory(prev => {
              const newHistory = [...prev, { value: ambient, timestamp: new Date() }];
              // Keep only the last 50 readings
              return newHistory.slice(-50);
            });
          } else if (topic === LIGHT_MOTION_TOPIC) {
            const motion = payload === '1';
            setLightState(prev => ({ ...prev, motion }));
            
            // Add to history
            setMotionHistory(prev => {
              const newHistory = [...prev, { value: motion, timestamp: new Date() }];
              // Keep only the last 50 readings
              return newHistory.slice(-50);
            });
          } else if (topic === STATUS_TOPIC) {
            const status = JSON.parse(payload);
            setDeviceStatus(status);
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
  
  // Toggle light state
  const toggleLight = useCallback(() => {
    if (client && connectionState === CONNECTION_STATE.CONNECTED) {
      const command = lightState.state === 'ON' ? 'OFF' : 'ON';
      client.publish(LIGHT_COMMAND_TOPIC, command);
    }
  }, [client, connectionState, lightState.state]);
  
  // Set light state
  const setLight = useCallback((state) => {
    if (client && connectionState === CONNECTION_STATE.CONNECTED) {
      client.publish(LIGHT_COMMAND_TOPIC, state ? 'ON' : 'OFF');
    }
  }, [client, connectionState]);
  
  // Set brightness
  const setBrightness = useCallback((brightness) => {
    if (client && connectionState === CONNECTION_STATE.CONNECTED) {
      client.publish(LIGHT_BRIGHTNESS_TOPIC, brightness.toString());
    }
  }, [client, connectionState]);
  
  // Set color
  const setColor = useCallback((color) => {
    if (client && connectionState === CONNECTION_STATE.CONNECTED) {
      const colorJson = JSON.stringify(color);
      client.publish(LIGHT_COLOR_TOPIC, colorJson);
    }
  }, [client, connectionState]);
  
  // Update connection settings
  const updateSettings = useCallback((newSettings) => {
    setSettings(prev => ({ ...prev, ...newSettings }));
  }, []);
  
  // Clear history data
  const clearHistory = useCallback(() => {
    setBrightnessHistory([]);
    setAmbientHistory([]);
    setMotionHistory([]);
  }, []);
  
  // Context value
  const value = {
    client,
    connectionState,
    error,
    lightState,
    brightnessHistory,
    ambientHistory,
    motionHistory,
    deviceStatus,
    settings,
    connect,
    disconnect,
    toggleLight,
    setLight,
    setBrightness,
    setColor,
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
