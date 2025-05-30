import { useState } from 'react';
import { 
  AppBar, 
  Toolbar, 
  Typography, 
  IconButton, 
  Button, 
  Box, 
  Menu, 
  MenuItem,
  Tooltip,
  Badge
} from '@mui/material';
import { 
  Menu as MenuIcon, 
  Lightbulb as LightbulbIcon,
  WbSunny as SunnyIcon,
  Brightness3 as MoonIcon,
  WifiOff as WifiOffIcon,
  Wifi as WifiIcon,
  Settings as SettingsIcon,
  AccountCircle as AccountIcon
} from '@mui/icons-material';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

const Header = ({ toggleSidebar }) => {
  const { connectionState, lightState, connect, disconnect, toggleLight } = useMqtt();
  const [anchorEl, setAnchorEl] = useState(null);
  
  const handleMenu = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleClose = () => {
    setAnchorEl(null);
  };
  
  const handleConnectionToggle = () => {
    if (connectionState === CONNECTION_STATE.CONNECTED) {
      disconnect();
    } else {
      connect();
    }
  };
  
  return (
    <AppBar position="fixed" sx={{ zIndex: (theme) => theme.zIndex.drawer + 1 }}>
      <Toolbar>
        <IconButton
          color="inherit"
          aria-label="open drawer"
          edge="start"
          onClick={toggleSidebar}
          sx={{ mr: 2 }}
        >
          <MenuIcon />
        </IconButton>
        
        <Typography variant="h6" component="div" sx={{ display: 'flex', alignItems: 'center' }}>
          <LightbulbIcon sx={{ mr: 1 }} />
          Smart Light Control
        </Typography>
        
        <Box sx={{ flexGrow: 1 }} />
        
        {/* Connection Status */}
        <Box sx={{ display: 'flex', alignItems: 'center', mr: 2 }}>
          <Tooltip title={
            connectionState === CONNECTION_STATE.CONNECTED 
              ? "Connected to MQTT broker" 
              : connectionState === CONNECTION_STATE.CONNECTING 
                ? "Connecting to MQTT broker" 
                : "Disconnected from MQTT broker"
          }>
            <IconButton 
              color="inherit" 
              onClick={handleConnectionToggle}
              disabled={connectionState === CONNECTION_STATE.CONNECTING}
            >
              {connectionState === CONNECTION_STATE.CONNECTED ? (
                <WifiIcon />
              ) : (
                <WifiOffIcon />
              )}
            </IconButton>
          </Tooltip>
        </Box>
        
        {/* Light Toggle Button */}
        <Box sx={{ mr: 2 }}>
          <Tooltip title={lightState.state === 'ON' ? "Turn Off Light" : "Turn On Light"}>
            <IconButton 
              color="inherit" 
              onClick={toggleLight}
              disabled={connectionState !== CONNECTION_STATE.CONNECTED}
            >
              {lightState.state === 'ON' ? (
                <SunnyIcon sx={{ color: 'yellow' }} />
              ) : (
                <MoonIcon />
              )}
            </IconButton>
          </Tooltip>
        </Box>
        
        {/* User Menu */}
        <Box>
          <IconButton
            size="large"
            aria-label="account of current user"
            aria-controls="menu-appbar"
            aria-haspopup="true"
            onClick={handleMenu}
            color="inherit"
          >
            <AccountIcon />
          </IconButton>
          <Menu
            id="menu-appbar"
            anchorEl={anchorEl}
            anchorOrigin={{
              vertical: 'bottom',
              horizontal: 'right',
            }}
            keepMounted
            transformOrigin={{
              vertical: 'top',
              horizontal: 'right',
            }}
            open={Boolean(anchorEl)}
            onClose={handleClose}
          >
            <MenuItem onClick={handleClose}>Profile</MenuItem>
            <MenuItem onClick={handleClose}>My account</MenuItem>
            <MenuItem onClick={handleClose}>Logout</MenuItem>
          </Menu>
        </Box>
      </Toolbar>
    </AppBar>
  );
};

export default Header;
