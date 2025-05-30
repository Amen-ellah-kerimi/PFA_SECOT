import { useState, useEffect } from 'react';
import styled from 'styled-components';
import { FaCog, FaSave, FaUndo } from 'react-icons/fa';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

const SettingsContainer = styled.div`
  max-width: 800px;
  margin: 0 auto;
`;

const SettingsHeader = styled.div`
  margin-bottom: 20px;
  
  h1 {
    font-size: 1.8rem;
    color: var(--text-color);
    margin-bottom: 10px;
    display: flex;
    align-items: center;
    
    svg {
      margin-right: 10px;
      color: var(--primary-color);
    }
  }
  
  p {
    color: var(--text-secondary);
  }
`;

const SettingsCard = styled.div`
  background-color: white;
  border-radius: var(--card-border-radius);
  padding: 20px;
  box-shadow: var(--shadow);
  margin-bottom: 20px;
`;

const SettingsForm = styled.form`
  display: grid;
  grid-template-columns: 1fr;
  gap: 20px;
`;

const FormSection = styled.div`
  h2 {
    font-size: 1.2rem;
    margin-bottom: 15px;
    padding-bottom: 10px;
    border-bottom: 1px solid var(--border-color);
  }
`;

const FormGroup = styled.div`
  margin-bottom: 15px;
  
  label {
    display: block;
    margin-bottom: 5px;
    font-weight: 500;
  }
  
  input, select {
    width: 100%;
    padding: 10px;
    border: 1px solid var(--border-color);
    border-radius: 4px;
    font-size: 1rem;
  }
  
  .hint {
    font-size: 0.8rem;
    color: var(--text-secondary);
    margin-top: 5px;
  }
  
  &.checkbox {
    display: flex;
    align-items: center;
    
    input {
      width: auto;
      margin-right: 10px;
    }
    
    label {
      margin-bottom: 0;
    }
  }
`;

const ButtonGroup = styled.div`
  display: flex;
  justify-content: flex-end;
  gap: 10px;
  margin-top: 20px;
`;

const Button = styled.button`
  display: flex;
  align-items: center;
  padding: 10px 15px;
  border-radius: 4px;
  font-size: 1rem;
  cursor: pointer;
  
  svg {
    margin-right: 8px;
  }
  
  &.primary {
    background-color: var(--primary-color);
    color: white;
    
    &:hover {
      background-color: #2980b9;
    }
  }
  
  &.secondary {
    background-color: #95a5a6;
    color: white;
    
    &:hover {
      background-color: #7f8c8d;
    }
  }
  
  &:disabled {
    background-color: #bdc3c7;
    cursor: not-allowed;
  }
`;

const Alert = styled.div`
  padding: 15px;
  margin-bottom: 20px;
  border-radius: 4px;
  
  &.success {
    background-color: #d5f5e3;
    color: #27ae60;
    border: 1px solid #2ecc71;
  }
  
  &.error {
    background-color: #fadbd8;
    color: #c0392b;
    border: 1px solid #e74c3c;
  }
  
  &.warning {
    background-color: #fef9e7;
    color: #f39c12;
    border: 1px solid #f1c40f;
  }
`;

function Settings() {
  const { settings, updateSettings, connectionState, disconnect } = useMqtt();
  
  const [formData, setFormData] = useState({
    host: '',
    port: '',
    clientId: '',
    username: '',
    password: '',
    useSSL: false,
  });
  
  const [alert, setAlert] = useState(null);
  
  // Initialize form with current settings
  useEffect(() => {
    setFormData({
      host: settings.host || '',
      port: settings.port || 9001,
      clientId: settings.clientId || '',
      username: settings.username || '',
      password: settings.password || '',
      useSSL: settings.useSSL || false,
    });
  }, [settings]);
  
  // Handle form input changes
  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setFormData(prev => ({
      ...prev,
      [name]: type === 'checkbox' ? checked : value,
    }));
  };
  
  // Reset form to current settings
  const handleReset = () => {
    setFormData({
      host: settings.host || '',
      port: settings.port || 9001,
      clientId: settings.clientId || '',
      username: settings.username || '',
      password: settings.password || '',
      useSSL: settings.useSSL || false,
    });
    setAlert(null);
  };
  
  // Save settings
  const handleSubmit = (e) => {
    e.preventDefault();
    
    // Validate form
    if (!formData.host) {
      setAlert({ type: 'error', message: 'Broker host is required' });
      return;
    }
    
    if (!formData.port || isNaN(formData.port) || formData.port < 1 || formData.port > 65535) {
      setAlert({ type: 'error', message: 'Port must be a valid number between 1 and 65535' });
      return;
    }
    
    if (!formData.clientId) {
      setAlert({ type: 'error', message: 'Client ID is required' });
      return;
    }
    
    // If connected, show warning
    if (connectionState === CONNECTION_STATE.CONNECTED) {
      setAlert({ 
        type: 'warning', 
        message: 'Changing settings will disconnect you from the current broker. Please disconnect first.' 
      });
      return;
    }
    
    // Update settings
    updateSettings({
      host: formData.host,
      port: parseInt(formData.port),
      clientId: formData.clientId,
      username: formData.username,
      password: formData.password,
      useSSL: formData.useSSL,
    });
    
    setAlert({ type: 'success', message: 'Settings saved successfully' });
    
    // Clear alert after 3 seconds
    setTimeout(() => {
      setAlert(null);
    }, 3000);
  };
  
  return (
    <SettingsContainer>
      <SettingsHeader>
        <h1><FaCog /> Connection Settings</h1>
        <p>Configure MQTT broker connection settings</p>
      </SettingsHeader>
      
      {alert && (
        <Alert className={alert.type}>
          {alert.message}
        </Alert>
      )}
      
      <SettingsCard>
        <SettingsForm onSubmit={handleSubmit}>
          <FormSection>
            <h2>MQTT Broker</h2>
            
            <FormGroup>
              <label htmlFor="host">Broker Host</label>
              <input
                type="text"
                id="host"
                name="host"
                value={formData.host}
                onChange={handleChange}
                placeholder="e.g., localhost or 192.168.1.100"
              />
              <div className="hint">The hostname or IP address of the MQTT broker</div>
            </FormGroup>
            
            <FormGroup>
              <label htmlFor="port">Broker Port</label>
              <input
                type="number"
                id="port"
                name="port"
                value={formData.port}
                onChange={handleChange}
                placeholder="e.g., 9001"
                min="1"
                max="65535"
              />
              <div className="hint">The WebSocket port of the MQTT broker (usually 9001)</div>
            </FormGroup>
            
            <FormGroup className="checkbox">
              <input
                type="checkbox"
                id="useSSL"
                name="useSSL"
                checked={formData.useSSL}
                onChange={handleChange}
              />
              <label htmlFor="useSSL">Use Secure Connection (WSS)</label>
            </FormGroup>
          </FormSection>
          
          <FormSection>
            <h2>Client Settings</h2>
            
            <FormGroup>
              <label htmlFor="clientId">Client ID</label>
              <input
                type="text"
                id="clientId"
                name="clientId"
                value={formData.clientId}
                onChange={handleChange}
                placeholder="e.g., web_dashboard"
              />
              <div className="hint">A unique identifier for this client</div>
            </FormGroup>
          </FormSection>
          
          <FormSection>
            <h2>Authentication (Optional)</h2>
            
            <FormGroup>
              <label htmlFor="username">Username</label>
              <input
                type="text"
                id="username"
                name="username"
                value={formData.username}
                onChange={handleChange}
                placeholder="Leave empty if not required"
              />
            </FormGroup>
            
            <FormGroup>
              <label htmlFor="password">Password</label>
              <input
                type="password"
                id="password"
                name="password"
                value={formData.password}
                onChange={handleChange}
                placeholder="Leave empty if not required"
              />
            </FormGroup>
          </FormSection>
          
          <ButtonGroup>
            <Button 
              type="button" 
              className="secondary"
              onClick={handleReset}
            >
              <FaUndo />
              Reset
            </Button>
            <Button 
              type="submit" 
              className="primary"
              disabled={connectionState === CONNECTION_STATE.CONNECTED}
            >
              <FaSave />
              Save Settings
            </Button>
          </ButtonGroup>
        </SettingsForm>
      </SettingsCard>
      
      {connectionState === CONNECTION_STATE.CONNECTED && (
        <Alert className="warning">
          You are currently connected to the MQTT broker. Please disconnect before changing settings.
        </Alert>
      )}
    </SettingsContainer>
  );
}

export default Settings;
