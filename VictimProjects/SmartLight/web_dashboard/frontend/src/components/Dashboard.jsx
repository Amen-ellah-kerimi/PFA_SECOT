import React from 'react';
import { useMqtt } from '../hooks/useMqtt';
import {
  Box,
  Container,
  Grid,
  Paper,
  Typography,
  Switch,
  Slider,
  IconButton,
  CircularProgress,
  Alert,
  Snackbar
} from '@mui/material';
import {
  Brightness4 as BrightnessIcon,
  Refresh as RefreshIcon,
  Power as PowerIcon
} from '@mui/icons-material';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

export default function Dashboard() {
  const {
    connectionStatus,
    lightState,
    toggleLight,
    setBrightness,
    setColor,
    reconnect
  } = useMqtt();

  const [showSnackbar, setShowSnackbar] = React.useState(false);
  const [snackbarMessage, setSnackbarMessage] = React.useState('');

  const handleReconnect = () => {
    reconnect();
    setSnackbarMessage('Attempting to reconnect...');
    setShowSnackbar(true);
  };

  const handleBrightnessChange = (event, newValue) => {
    setBrightness(newValue);
  };

  const handleColorChange = (event, newValue) => {
    setColor({ r: newValue, g: newValue, b: newValue });
  };

  return (
    <Container maxWidth="lg" sx={{ mt: 4, mb: 4 }}>
      <Grid container spacing={3}>
        {/* Status Bar */}
        <Grid item xs={12}>
          <Paper sx={{ p: 2, display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
            <Typography variant="h6" component="div">
              Smart Light Dashboard
            </Typography>
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
              {connectionStatus === 'connecting' && (
                <CircularProgress size={20} />
              )}
              <Typography
                variant="body2"
                color={connectionStatus === 'connected' ? 'success.main' : 'error.main'}
              >
                {connectionStatus.toUpperCase()}
              </Typography>
              <IconButton onClick={handleReconnect} color="primary">
                <RefreshIcon />
              </IconButton>
            </Box>
          </Paper>
        </Grid>

        {/* Main Control Panel */}
        <Grid item xs={12} md={8}>
          <Paper sx={{ p: 2 }}>
            <Grid container spacing={2}>
              <Grid item xs={12}>
                <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                  <Typography variant="h6">Light Control</Typography>
                  <Switch
                    checked={lightState.state === 'ON'}
                    onChange={toggleLight}
                    color="primary"
                  />
                </Box>
              </Grid>
              <Grid item xs={12}>
                <Typography gutterBottom>Brightness</Typography>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
                  <BrightnessIcon />
                  <Slider
                    value={lightState.brightness}
                    onChange={handleBrightnessChange}
                    min={0}
                    max={255}
                    disabled={lightState.state === 'OFF'}
                  />
                </Box>
              </Grid>
              <Grid item xs={12}>
                <Typography gutterBottom>Color Temperature</Typography>
                <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
                  <PowerIcon />
                  <Slider
                    value={lightState.color.r}
                    onChange={handleColorChange}
                    min={0}
                    max={255}
                    disabled={lightState.state === 'OFF'}
                  />
                </Box>
              </Grid>
            </Grid>
          </Paper>
        </Grid>

        {/* Status Panel */}
        <Grid item xs={12} md={4}>
          <Paper sx={{ p: 2 }}>
            <Typography variant="h6" gutterBottom>Status</Typography>
            <Box sx={{ display: 'flex', flexDirection: 'column', gap: 2 }}>
              <Typography>
                Ambient Light: {lightState.ambient} lux
              </Typography>
              <Typography>
                Motion Detected: {lightState.motion ? 'Yes' : 'No'}
              </Typography>
              <Typography>
                Current Brightness: {lightState.brightness}
              </Typography>
              <Typography>
                Color: RGB({lightState.color.r}, {lightState.color.g}, {lightState.color.b})
              </Typography>
            </Box>
          </Paper>
        </Grid>
      </Grid>

      <Snackbar
        open={showSnackbar}
        autoHideDuration={3000}
        onClose={() => setShowSnackbar(false)}
      >
        <Alert severity="info" onClose={() => setShowSnackbar(false)}>
          {snackbarMessage}
        </Alert>
      </Snackbar>
    </Container>
  );
}
