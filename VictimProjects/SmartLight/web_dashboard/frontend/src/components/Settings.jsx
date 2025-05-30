import { useState, useEffect } from 'react';
import { 
  Box, 
  Paper, 
  Typography, 
  TextField, 
  Button, 
  Grid,
  FormControlLabel,
  Switch,
  Divider,
  Alert,
  Snackbar,
  Card,
  CardContent,
  CardActions,
  FormControl,
  InputLabel,
  Select,
  MenuItem
} from '@mui/material';
import { 
  Settings as SettingsIcon,
  Save as SaveIcon,
  Refresh as RefreshIcon,
  WifiTethering as WifiIcon
} from '@mui/icons-material';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

const Settings = () => {
  const { 
    connectionState, 
    settings, 
    updateSettings, 
    disconnect 
  } = useMqtt();
  
  const [formData, setFormData] = useState({
    host: '',
    port: '',
    clientId: '',
    username: '',
    password: '',
    useSSL: false,
  });
  
  const [snackbar, setSnackbar] = useState({
    open: false,
    message: '',
    severity: 'success'
  });
  
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
  };
  
  // Save settings
  const handleSubmit = (e) => {
    e.preventDefault();
    
    // Validate form
    if (!formData.host) {
      setSnackbar({
        open: true,
        message: 'Broker host is required',
        severity: 'error'
      });
      return;
    }
    
    if (!formData.port || isNaN(formData.port) || formData.port < 1 || formData.port > 65535) {
      setSnackbar({
        open: true,
        message: 'Port must be a valid number between 1 and 65535',
        severity: 'error'
      });
      return;
    }
    
    if (!formData.clientId) {
      setSnackbar({
        open: true,
        message: 'Client ID is required',
        severity: 'error'
      });
      return;
    }
    
    // If connected, disconnect first
    if (connectionState === CONNECTION_STATE.CONNECTED) {
      disconnect();
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
    
    setSnackbar({
      open: true,
      message: 'Settings saved successfully',
      severity: 'success'
    });
  };
  
  // Close snackbar
  const handleCloseSnackbar = () => {
    setSnackbar(prev => ({ ...prev, open: false }));
  };
  
  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        Settings
      </Typography>
      
      <Grid container spacing={3}>
        {/* MQTT Settings */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <WifiIcon sx={{ mr: 1 }} />
              MQTT Connection Settings
            </Typography>
            
            {connectionState === CONNECTION_STATE.CONNECTED && (
              <Alert severity="warning" sx={{ mb: 3 }}>
                You are currently connected to the MQTT broker. Changes will take effect after reconnecting.
              </Alert>
            )}
            
            <form onSubmit={handleSubmit}>
              <Grid container spacing={2}>
                <Grid item xs={12}>
                  <TextField
                    label="Broker Host"
                    name="host"
                    value={formData.host}
                    onChange={handleChange}
                    fullWidth
                    margin="normal"
                    helperText="Hostname or IP address of the MQTT broker"
                    required
                  />
                </Grid>
                
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Broker Port"
                    name="port"
                    type="number"
                    value={formData.port}
                    onChange={handleChange}
                    fullWidth
                    margin="normal"
                    helperText="WebSocket port (usually 9001)"
                    required
                    inputProps={{ min: 1, max: 65535 }}
                  />
                </Grid>
                
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Client ID"
                    name="clientId"
                    value={formData.clientId}
                    onChange={handleChange}
                    fullWidth
                    margin="normal"
                    helperText="Unique identifier for this client"
                    required
                  />
                </Grid>
                
                <Grid item xs={12}>
                  <FormControlLabel
                    control={
                      <Switch
                        name="useSSL"
                        checked={formData.useSSL}
                        onChange={handleChange}
                        color="primary"
                      />
                    }
                    label="Use Secure Connection (WSS)"
                  />
                </Grid>
                
                <Grid item xs={12}>
                  <Divider sx={{ my: 2 }} />
                  <Typography variant="subtitle2" gutterBottom>
                    Authentication (Optional)
                  </Typography>
                </Grid>
                
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Username"
                    name="username"
                    value={formData.username}
                    onChange={handleChange}
                    fullWidth
                    margin="normal"
                    helperText="Leave empty if not required"
                  />
                </Grid>
                
                <Grid item xs={12} sm={6}>
                  <TextField
                    label="Password"
                    name="password"
                    type="password"
                    value={formData.password}
                    onChange={handleChange}
                    fullWidth
                    margin="normal"
                    helperText="Leave empty if not required"
                  />
                </Grid>
                
                <Grid item xs={12}>
                  <Box sx={{ display: 'flex', justifyContent: 'flex-end', gap: 2, mt: 2 }}>
                    <Button
                      type="button"
                      variant="outlined"
                      startIcon={<RefreshIcon />}
                      onClick={handleReset}
                    >
                      Reset
                    </Button>
                    <Button
                      type="submit"
                      variant="contained"
                      startIcon={<SaveIcon />}
                    >
                      Save Settings
                    </Button>
                  </Box>
                </Grid>
              </Grid>
            </form>
          </Paper>
        </Grid>
        
        {/* App Settings */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <SettingsIcon sx={{ mr: 1 }} />
              Application Settings
            </Typography>
            
            <Grid container spacing={2}>
              <Grid item xs={12}>
                <FormControl fullWidth margin="normal">
                  <InputLabel id="theme-label">Theme</InputLabel>
                  <Select
                    labelId="theme-label"
                    value="light"
                    label="Theme"
                  >
                    <MenuItem value="light">Light</MenuItem>
                    <MenuItem value="dark">Dark</MenuItem>
                    <MenuItem value="system">System Default</MenuItem>
                  </Select>
                </FormControl>
              </Grid>
              
              <Grid item xs={12}>
                <FormControl fullWidth margin="normal">
                  <InputLabel id="language-label">Language</InputLabel>
                  <Select
                    labelId="language-label"
                    value="en"
                    label="Language"
                  >
                    <MenuItem value="en">English</MenuItem>
                    <MenuItem value="fr">Français</MenuItem>
                    <MenuItem value="es">Español</MenuItem>
                    <MenuItem value="de">Deutsch</MenuItem>
                  </Select>
                </FormControl>
              </Grid>
              
              <Grid item xs={12}>
                <FormControlLabel
                  control={<Switch color="primary" />}
                  label="Enable Notifications"
                />
              </Grid>
              
              <Grid item xs={12}>
                <FormControlLabel
                  control={<Switch color="primary" />}
                  label="Auto-connect on Startup"
                />
              </Grid>
              
              <Grid item xs={12}>
                <FormControlLabel
                  control={<Switch color="primary" />}
                  label="Show Advanced Features"
                />
              </Grid>
              
              <Grid item xs={12}>
                <Box sx={{ display: 'flex', justifyContent: 'flex-end', mt: 2 }}>
                  <Button
                    variant="contained"
                    startIcon={<SaveIcon />}
                  >
                    Save App Settings
                  </Button>
                </Box>
              </Grid>
            </Grid>
          </Paper>
        </Grid>
      </Grid>
      
      <Snackbar
        open={snackbar.open}
        autoHideDuration={6000}
        onClose={handleCloseSnackbar}
      >
        <Alert 
          onClose={handleCloseSnackbar} 
          severity={snackbar.severity}
          sx={{ width: '100%' }}
        >
          {snackbar.message}
        </Alert>
      </Snackbar>
    </Box>
  );
};

export default Settings;
