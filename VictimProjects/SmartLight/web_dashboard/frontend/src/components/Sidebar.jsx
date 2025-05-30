import { useLocation } from 'react-router-dom';
import { 
  Drawer, 
  List, 
  ListItem, 
  ListItemButton, 
  ListItemIcon, 
  ListItemText,
  Divider,
  Box,
  Typography,
  Toolbar
} from '@mui/material';
import { 
  Dashboard as DashboardIcon,
  Lightbulb as LightbulbIcon,
  ColorLens as ColorLensIcon,
  BarChart as BarChartIcon,
  Settings as SettingsIcon,
  Info as InfoIcon,
  WbSunny as SunnyIcon,
  Brightness3 as MoonIcon,
  Speed as SpeedIcon
} from '@mui/icons-material';
import { Link } from 'react-router-dom';
import { useMqtt } from '../hooks/useMqtt';

const drawerWidth = 240;

const menuItems = [
  { text: 'Dashboard', icon: <DashboardIcon />, path: '/' },
  { text: 'Light Control', icon: <LightbulbIcon />, path: '/control' },
  { text: 'Scenes', icon: <ColorLensIcon />, path: '/scenes' },
  { text: 'Analytics', icon: <BarChartIcon />, path: '/analytics' },
  { text: 'Settings', icon: <SettingsIcon />, path: '/settings' },
  { text: 'About', icon: <InfoIcon />, path: '/about' },
];

const Sidebar = ({ isOpen }) => {
  const location = useLocation();
  const { lightState } = useMqtt();
  
  // Format RGB color as CSS
  const getRgbColor = () => {
    const { r, g, b } = lightState.color;
    return `rgb(${r}, ${g}, ${b})`;
  };
  
  return (
    <Drawer
      variant="persistent"
      open={isOpen}
      sx={{
        width: drawerWidth,
        flexShrink: 0,
        '& .MuiDrawer-paper': {
          width: drawerWidth,
          boxSizing: 'border-box',
        },
      }}
    >
      <Toolbar />
      <Box sx={{ overflow: 'auto' }}>
        <List>
          {menuItems.map((item) => (
            <ListItem key={item.text} disablePadding>
              <ListItemButton 
                component={Link} 
                to={item.path}
                selected={location.pathname === item.path}
              >
                <ListItemIcon>
                  {item.icon}
                </ListItemIcon>
                <ListItemText primary={item.text} />
              </ListItemButton>
            </ListItem>
          ))}
        </List>
        
        <Divider sx={{ my: 2 }} />
        
        {/* Light Status */}
        <Box sx={{ p: 2 }}>
          <Typography variant="subtitle2" color="text.secondary" gutterBottom>
            LIGHT STATUS
          </Typography>
          
          <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
            <Box sx={{ mr: 2 }}>
              {lightState.state === 'ON' ? (
                <SunnyIcon sx={{ color: 'orange' }} />
              ) : (
                <MoonIcon color="disabled" />
              )}
            </Box>
            <Box>
              <Typography variant="body2">
                State: <strong>{lightState.state}</strong>
              </Typography>
              <Typography variant="body2">
                Brightness: <strong>{lightState.brightness}</strong>
              </Typography>
            </Box>
          </Box>
          
          <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
            <Box 
              sx={{ 
                width: 24, 
                height: 24, 
                borderRadius: '50%', 
                bgcolor: getRgbColor(),
                mr: 2,
                border: '1px solid #ddd'
              }} 
            />
            <Typography variant="body2">
              Color: <strong>RGB({lightState.color.r}, {lightState.color.g}, {lightState.color.b})</strong>
            </Typography>
          </Box>
          
          <Box sx={{ display: 'flex', alignItems: 'center' }}>
            <SpeedIcon sx={{ mr: 2 }} />
            <Typography variant="body2">
              Ambient Light: <strong>{lightState.ambient}%</strong>
            </Typography>
          </Box>
        </Box>
      </Box>
    </Drawer>
  );
};

export default Sidebar;
