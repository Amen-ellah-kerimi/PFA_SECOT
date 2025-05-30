import { useState, useEffect } from 'react';
import { 
  Box, 
  Grid, 
  Paper, 
  Typography, 
  Switch, 
  Slider, 
  Button,
  Divider,
  Alert,
  TextField,
  InputAdornment,
  Card,
  CardContent,
  CardActions,
  IconButton,
  Tabs,
  Tab
} from '@mui/material';
import { 
  Lightbulb as LightbulbIcon,
  WbSunny as SunnyIcon,
  Brightness3 as MoonIcon,
  Brightness5 as BrightnessIcon,
  Brightness6 as BrightnessLowIcon,
  ColorLens as ColorLensIcon,
  Save as SaveIcon,
  Refresh as RefreshIcon,
  Timer as TimerIcon
} from '@mui/icons-material';
import { SketchPicker } from 'react-color';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

const LightControl = () => {
  const { 
    connectionState, 
    lightState, 
    connect, 
    toggleLight, 
    setBrightness, 
    setColor 
  } = useMqtt();
  
  const [tabValue, setTabValue] = useState(0);
  const [colorPickerColor, setColorPickerColor] = useState({ 
    r: 255, 
    g: 255, 
    b: 255, 
    a: 1 
  });
  const [redValue, setRedValue] = useState(255);
  const [greenValue, setGreenValue] = useState(255);
  const [blueValue, setBlueValue] = useState(255);
  
  // Update local state when lightState changes
  useEffect(() => {
    if (lightState.color) {
      setColorPickerColor({ 
        r: lightState.color.r, 
        g: lightState.color.g, 
        b: lightState.color.b, 
        a: 1 
      });
      setRedValue(lightState.color.r);
      setGreenValue(lightState.color.g);
      setBlueValue(lightState.color.b);
    }
  }, [lightState]);
  
  // Handle color picker change
  const handleColorChange = (color) => {
    setColorPickerColor(color.rgb);
    setRedValue(color.rgb.r);
    setGreenValue(color.rgb.g);
    setBlueValue(color.rgb.b);
  };
  
  // Handle color apply
  const handleColorApply = () => {
    setColor({
      r: colorPickerColor.r,
      g: colorPickerColor.g,
      b: colorPickerColor.b
    });
  };
  
  // Handle RGB input change
  const handleRgbChange = (color, value) => {
    let newValue = parseInt(value);
    
    // Validate input
    if (isNaN(newValue)) {
      newValue = 0;
    } else if (newValue < 0) {
      newValue = 0;
    } else if (newValue > 255) {
      newValue = 255;
    }
    
    // Update the specific color value
    if (color === 'r') {
      setRedValue(newValue);
      setColorPickerColor(prev => ({ ...prev, r: newValue }));
    } else if (color === 'g') {
      setGreenValue(newValue);
      setColorPickerColor(prev => ({ ...prev, g: newValue }));
    } else if (color === 'b') {
      setBlueValue(newValue);
      setColorPickerColor(prev => ({ ...prev, b: newValue }));
    }
  };
  
  // Handle RGB apply
  const handleRgbApply = () => {
    setColor({
      r: redValue,
      g: greenValue,
      b: blueValue
    });
  };
  
  // Handle tab change
  const handleTabChange = (event, newValue) => {
    setTabValue(newValue);
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
        Light Control
      </Typography>
      
      <Grid container spacing={3}>
        {/* Main Control Card */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
              <Box sx={{ mr: 2 }}>
                {lightState.state === 'ON' ? (
                  <LightbulbIcon 
                    sx={{ 
                      fontSize: 48, 
                      color: `rgb(${lightState.color.r}, ${lightState.color.g}, ${lightState.color.b})`,
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
              <Typography variant="body2" sx={{ ml: 2, minWidth: 40 }}>
                {Math.round(lightState.brightness / 255 * 100)}%
              </Typography>
            </Box>
            
            <Divider sx={{ my: 2 }} />
            
            <Typography variant="subtitle2" gutterBottom>
              Timer Control
            </Typography>
            <Box sx={{ display: 'flex', gap: 1, flexWrap: 'wrap', mb: 2 }}>
              <Button 
                variant="outlined" 
                size="small"
                startIcon={<TimerIcon />}
                disabled={lightState.state !== 'ON'}
              >
                15 min
              </Button>
              <Button 
                variant="outlined" 
                size="small"
                startIcon={<TimerIcon />}
                disabled={lightState.state !== 'ON'}
              >
                30 min
              </Button>
              <Button 
                variant="outlined" 
                size="small"
                startIcon={<TimerIcon />}
                disabled={lightState.state !== 'ON'}
              >
                1 hour
              </Button>
              <Button 
                variant="outlined" 
                size="small"
                startIcon={<TimerIcon />}
                disabled={lightState.state !== 'ON'}
              >
                Custom
              </Button>
            </Box>
          </Paper>
        </Grid>
        
        {/* Color Control Card */}
        <Grid item xs={12} md={6}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom>
              Color Control
            </Typography>
            
            <Box sx={{ borderBottom: 1, borderColor: 'divider', mb: 2 }}>
              <Tabs value={tabValue} onChange={handleTabChange} aria-label="color control tabs">
                <Tab label="Color Picker" />
                <Tab label="RGB Values" />
              </Tabs>
            </Box>
            
            {tabValue === 0 && (
              <Box>
                <Box sx={{ mb: 2, display: 'flex', justifyContent: 'center' }}>
                  <SketchPicker 
                    color={colorPickerColor}
                    onChange={handleColorChange}
                    disableAlpha={true}
                    presetColors={[
                      '#FFFFFF', // White
                      '#FF0000', // Red
                      '#00FF00', // Green
                      '#0000FF', // Blue
                      '#FFFF00', // Yellow
                      '#00FFFF', // Cyan
                      '#FF00FF', // Magenta
                      '#FFA500', // Orange
                      '#800080', // Purple
                      '#008000', // Dark Green
                      '#FFC0CB', // Pink
                      '#FFD700', // Gold
                    ]}
                  />
                </Box>
                <Button 
                  variant="contained" 
                  onClick={handleColorApply}
                  disabled={lightState.state !== 'ON'}
                  fullWidth
                >
                  Apply Color
                </Button>
              </Box>
            )}
            
            {tabValue === 1 && (
              <Box>
                <Grid container spacing={2} sx={{ mb: 3 }}>
                  <Grid item xs={4}>
                    <TextField
                      label="Red"
                      type="number"
                      value={redValue}
                      onChange={(e) => handleRgbChange('r', e.target.value)}
                      InputProps={{
                        endAdornment: <InputAdornment position="end">R</InputAdornment>,
                      }}
                      inputProps={{
                        min: 0,
                        max: 255,
                      }}
                      fullWidth
                      disabled={lightState.state !== 'ON'}
                    />
                  </Grid>
                  <Grid item xs={4}>
                    <TextField
                      label="Green"
                      type="number"
                      value={greenValue}
                      onChange={(e) => handleRgbChange('g', e.target.value)}
                      InputProps={{
                        endAdornment: <InputAdornment position="end">G</InputAdornment>,
                      }}
                      inputProps={{
                        min: 0,
                        max: 255,
                      }}
                      fullWidth
                      disabled={lightState.state !== 'ON'}
                    />
                  </Grid>
                  <Grid item xs={4}>
                    <TextField
                      label="Blue"
                      type="number"
                      value={blueValue}
                      onChange={(e) => handleRgbChange('b', e.target.value)}
                      InputProps={{
                        endAdornment: <InputAdornment position="end">B</InputAdornment>,
                      }}
                      inputProps={{
                        min: 0,
                        max: 255,
                      }}
                      fullWidth
                      disabled={lightState.state !== 'ON'}
                    />
                  </Grid>
                </Grid>
                
                <Box sx={{ display: 'flex', alignItems: 'center', mb: 3 }}>
                  <Typography variant="body2" sx={{ mr: 2 }}>
                    Preview:
                  </Typography>
                  <Box 
                    sx={{ 
                      width: 40, 
                      height: 40, 
                      borderRadius: '50%', 
                      bgcolor: `rgb(${redValue}, ${greenValue}, ${blueValue})`,
                      border: '1px solid #ddd'
                    }} 
                  />
                </Box>
                
                <Button 
                  variant="contained" 
                  onClick={handleRgbApply}
                  disabled={lightState.state !== 'ON'}
                  fullWidth
                >
                  Apply RGB Values
                </Button>
              </Box>
            )}
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default LightControl;
