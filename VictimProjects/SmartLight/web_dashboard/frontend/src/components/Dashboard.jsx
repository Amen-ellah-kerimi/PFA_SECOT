import { useState } from 'react';
import { 
  Box, 
  Grid, 
  Paper, 
  Typography, 
  Switch, 
  Slider, 
  Button,
  Card,
  CardContent,
  CardActions,
  IconButton,
  Divider,
  Chip,
  Stack,
  Alert
} from '@mui/material';
import { 
  Lightbulb as LightbulbIcon,
  WbSunny as SunnyIcon,
  Brightness3 as MoonIcon,
  Brightness5 as BrightnessIcon,
  Brightness6 as BrightnessLowIcon,
  ColorLens as ColorLensIcon,
  BarChart as BarChartIcon,
  ExpandMore as ExpandMoreIcon,
  ExpandLess as ExpandLessIcon,
  Sensors as SensorsIcon,
  AccessTime as TimeIcon
} from '@mui/icons-material';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';
import { Link } from 'react-router-dom';

const Dashboard = () => {
  const { 
    connectionState, 
    lightState, 
    connect, 
    toggleLight, 
    setBrightness, 
    setColor 
  } = useMqtt();
  
  const [expanded, setExpanded] = useState(false);
  
  // Format RGB color as CSS
  const getRgbColor = () => {
    const { r, g, b } = lightState.color;
    return `rgb(${r}, ${g}, ${b})`;
  };
  
  // Format timestamp
  const formatTime = (timestamp) => {
    if (!timestamp) return '--';
    const date = new Date(timestamp);
    return date.toLocaleTimeString();
  };
  
  // If not connected, show connection prompt
  if (connectionState !== CONNECTION_STATE.CONNECTED) {
    return (
      <Box sx={{ mt: 4 }}>
        <Alert 
          severity="info" 
          sx={{ mb: 2 }}
        >
          Connect to the MQTT broker to control your smart light
        </Alert>
        
        <Paper sx={{ p: 3, textAlign: 'center' }}>
          <LightbulbIcon sx={{ fontSize: 60, color: 'text.secondary', mb: 2 }} />
          <Typography variant="h5" gutterBottom>
            Not Connected
          </Typography>
          <Typography variant="body1" color="text.secondary" paragraph>
            You need to connect to the MQTT broker to view and control your smart light.
          </Typography>
          <Button 
            variant="contained" 
            onClick={connect}
            disabled={connectionState === CONNECTION_STATE.CONNECTING}
          >
            {connectionState === CONNECTION_STATE.CONNECTING ? 'Connecting...' : 'Connect Now'}
          </Button>
        </Paper>
      </Box>
    );
  }
  
  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        Dashboard
      </Typography>
      
      <Grid container spacing={3}>
        {/* Light Control Card */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
              <Box sx={{ mr: 2 }}>
                {lightState.state === 'ON' ? (
                  <LightbulbIcon 
                    sx={{ 
                      fontSize: 48, 
                      color: getRgbColor(),
                      filter: 'drop-shadow(0 0 8px rgba(0,0,0,0.3))'
                    }} 
                  />
                ) : (
                  <LightbulbIcon sx={{ fontSize: 48, color: 'text.disabled' }} />
                )}
              </Box>
              <Box sx={{ flexGrow: 1 }}>
                <Typography variant="h6">
                  Smart Light
                </Typography>
                <Typography variant="body2" color="text.secondary">
                  {lightState.state === 'ON' ? 'Currently On' : 'Currently Off'}
                </Typography>
              </Box>
              <Switch 
                checked={lightState.state === 'ON'} 
                onChange={toggleLight}
                color="primary"
              />
            </Box>
            
            <Divider sx={{ my: 2 }} />
            
            <Typography variant="subtitle2" gutterBottom>
              Brightness
            </Typography>
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
              <BrightnessLowIcon sx={{ mr: 1, color: 'text.secondary' }} />
              <Slider
                value={lightState.brightness}
                min={0}
                max={255}
                onChange={(e, newValue) => setBrightness(newValue)}
                disabled={lightState.state !== 'ON'}
                sx={{ mx: 2 }}
              />
              <BrightnessIcon sx={{ ml: 1, color: 'text.secondary' }} />
            </Box>
            
            <Typography variant="subtitle2" gutterBottom>
              Color
            </Typography>
            <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1, mb: 2 }}>
              {/* Quick color buttons */}
              {[
                { name: 'White', color: { r: 255, g: 255, b: 255 } },
                { name: 'Red', color: { r: 255, g: 0, b: 0 } },
                { name: 'Green', color: { r: 0, g: 255, b: 0 } },
                { name: 'Blue', color: { r: 0, g: 0, b: 255 } },
                { name: 'Warm', color: { r: 255, g: 200, b: 120 } },
                { name: 'Cool', color: { r: 200, g: 220, b: 255 } },
              ].map((item) => (
                <Chip
                  key={item.name}
                  label={item.name}
                  onClick={() => setColor(item.color)}
                  disabled={lightState.state !== 'ON'}
                  sx={{
                    bgcolor: `rgb(${item.color.r}, ${item.color.g}, ${item.color.b})`,
                    color: item.name === 'White' || item.name === 'Warm' || item.name === 'Cool' || item.name === 'Green' ? 'black' : 'white',
                    '&:hover': {
                      bgcolor: `rgb(${item.color.r * 0.9}, ${item.color.g * 0.9}, ${item.color.b * 0.9})`,
                    }
                  }}
                />
              ))}
            </Box>
            
            <Button 
              variant="outlined" 
              component={Link} 
              to="/control"
              startIcon={<ColorLensIcon />}
              fullWidth
              sx={{ mt: 1 }}
            >
              Advanced Control
            </Button>
          </Paper>
        </Grid>
        
        {/* Status Card */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom>
              Status Information
            </Typography>
            
            <Box sx={{ mb: 2 }}>
              <Grid container spacing={2}>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Current State
                  </Typography>
                  <Typography variant="body1">
                    {lightState.state}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Brightness
                  </Typography>
                  <Typography variant="body1">
                    {lightState.brightness} / 255
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Color
                  </Typography>
                  <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <Box 
                      sx={{ 
                        width: 16, 
                        height: 16, 
                        borderRadius: '50%', 
                        bgcolor: getRgbColor(),
                        mr: 1,
                        border: '1px solid #ddd'
                      }} 
                    />
                    <Typography variant="body1">
                      RGB({lightState.color.r}, {lightState.color.g}, {lightState.color.b})
                    </Typography>
                  </Box>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Ambient Light
                  </Typography>
                  <Typography variant="body1">
                    {lightState.ambient}%
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Motion Detected
                  </Typography>
                  <Typography variant="body1">
                    {lightState.motion ? 'Yes' : 'No'}
                  </Typography>
                </Grid>
                <Grid item xs={6}>
                  <Typography variant="subtitle2" color="text.secondary">
                    Last Update
                  </Typography>
                  <Typography variant="body1">
                    {formatTime(lightState.timestamp)}
                  </Typography>
                </Grid>
              </Grid>
            </Box>
            
            <Divider sx={{ my: 2 }} />
            
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
              <Typography variant="subtitle2">
                Device Information
              </Typography>
              <IconButton 
                size="small" 
                onClick={() => setExpanded(!expanded)}
                aria-expanded={expanded}
                aria-label="show more"
              >
                {expanded ? <ExpandLessIcon /> : <ExpandMoreIcon />}
              </IconButton>
            </Box>
            
            {expanded && (
              <Box sx={{ mt: 2 }}>
                <Grid container spacing={2}>
                  <Grid item xs={6}>
                    <Typography variant="subtitle2" color="text.secondary">
                      Device ID
                    </Typography>
                    <Typography variant="body2">
                      {lightState.device_id || 'smart_light_1'}
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="subtitle2" color="text.secondary">
                      IP Address
                    </Typography>
                    <Typography variant="body2">
                      {lightState.ip || '192.168.1.100'}
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="subtitle2" color="text.secondary">
                      Firmware Version
                    </Typography>
                    <Typography variant="body2">
                      1.0.0
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="subtitle2" color="text.secondary">
                      Connection Type
                    </Typography>
                    <Typography variant="body2">
                      WiFi
                    </Typography>
                  </Grid>
                </Grid>
              </Box>
            )}
            
            <Box sx={{ mt: 3, display: 'flex', justifyContent: 'space-between' }}>
              <Button 
                variant="outlined" 
                component={Link} 
                to="/analytics"
                startIcon={<BarChartIcon />}
              >
                View Analytics
              </Button>
              <Button 
                variant="outlined" 
                component={Link} 
                to="/scenes"
                startIcon={<ColorLensIcon />}
              >
                Light Scenes
              </Button>
            </Box>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Dashboard;
