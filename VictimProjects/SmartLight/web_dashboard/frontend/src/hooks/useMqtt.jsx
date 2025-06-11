import { createContext, useContext, useState, useEffect, useCallback } from 'react';
import axios from 'axios';

// Create context
const MqttContext = createContext(null);

// API endpoints
const API_BASE_URL = 'http://localhost:5000/api';

// Connection states
export const CONNECTION_STATE = {
  DISCONNECTED: 'disconnected',
  CONNECTING: 'connecting',
  CONNECTED: 'connected',
};

// Provider component
export const MqttProvider = ({ children }) => {
  const [connectionStatus, setConnectionStatus] = useState('disconnected');
  const [lightState, setLightState] = useState({
    state: 'OFF',
    brightness: 0,
    color: { r: 0, g: 0, b: 0 },
    motion: false
  });

  // Poll for updates
  useEffect(() => {
    const pollInterval = setInterval(async () => {
      try {
        const response = await axios.get(`${API_BASE_URL}/status`);
        setConnectionStatus(response.data.connection_status);
        setLightState(response.data.light_state);
      } catch (error) {
        console.error('Error polling status:', error);
        setConnectionStatus('error');
      }
    }, 1000);

    return () => clearInterval(pollInterval);
  }, []);

  const toggleLight = useCallback(async () => {
    try {
      await axios.post(`${API_BASE_URL}/light/toggle`);
    } catch (error) {
      console.error('Error toggling light:', error);
    }
  }, []);

  const setBrightness = useCallback(async (brightness) => {
    try {
      await axios.post(`${API_BASE_URL}/brightness`, { brightness });
    } catch (error) {
      console.error('Error setting brightness:', error);
    }
  }, []);

  const setColor = useCallback(async (color) => {
    try {
      await axios.post(`${API_BASE_URL}/color`, { color });
    } catch (error) {
      console.error('Error setting color:', error);
    }
  }, []);

  const reconnect = useCallback(async () => {
    try {
      await axios.post(`${API_BASE_URL}/reconnect`);
    } catch (error) {
      console.error('Error reconnecting:', error);
    }
  }, []);

  // Context value
  const value = {
    connectionStatus,
    lightState,
    toggleLight,
    setBrightness,
    setColor,
    reconnect
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
