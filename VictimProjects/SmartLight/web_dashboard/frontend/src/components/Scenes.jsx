import { useState } from 'react';
import { 
  Box, 
  Grid, 
  Paper, 
  Typography, 
  Card, 
  CardContent, 
  CardActionArea,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TextField,
  Slider,
  Alert,
  Divider,
  IconButton
} from '@mui/material';
import { 
  Lightbulb as LightbulbIcon,
  WbSunny as SunnyIcon,
  NightsStay as NightIcon,
  LocalLibrary as ReadingIcon,
  Theaters as MovieIcon,
  Celebration as PartyIcon,
  Spa as RelaxIcon,
  Add as AddIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  Save as SaveIcon
} from '@mui/icons-material';
import { SketchPicker } from 'react-color';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

// Predefined scenes
const predefinedScenes = [
  { 
    id: 'daylight', 
    name: 'Daylight', 
    icon: <SunnyIcon />, 
    color: { r: 255, g: 255, b: 255 }, 
    brightness: 255,
    description: 'Bright white light for daytime activities'
  },
  { 
    id: 'night', 
    name: 'Night Light', 
    icon: <NightIcon />, 
    color: { r: 255, g: 200, b: 120 }, 
    brightness: 50,
    description: 'Dim warm light for nighttime'
  },
  { 
    id: 'reading', 
    name: 'Reading', 
    icon: <ReadingIcon />, 
    color: { r: 255, g: 240, b: 200 }, 
    brightness: 200,
    description: 'Comfortable light for reading'
  },
  { 
    id: 'movie', 
    name: 'Movie', 
    icon: <MovieIcon />, 
    color: { r: 100, g: 50, b: 255 }, 
    brightness: 100,
    description: 'Ambient light for watching movies'
  },
  { 
    id: 'party', 
    name: 'Party', 
    icon: <PartyIcon />, 
    color: { r: 255, g: 0, b: 200 }, 
    brightness: 200,
    description: 'Vibrant light for parties'
  },
  { 
    id: 'relax', 
    name: 'Relax', 
    icon: <RelaxIcon />, 
    color: { r: 200, g: 100, b: 255 }, 
    brightness: 150,
    description: 'Calming light for relaxation'
  },
];

const Scenes = () => {
  const { 
    connectionState, 
    lightState, 
    connect, 
    setLight, 
    setBrightness, 
    setColor 
  } = useMqtt();
  
  const [scenes, setScenes] = useState(() => {
    const savedScenes = localStorage.getItem('custom_scenes');
    return savedScenes ? JSON.parse(savedScenes) : [];
  });
  
  const [openDialog, setOpenDialog] = useState(false);
  const [editingScene, setEditingScene] = useState(null);
  const [sceneName, setSceneName] = useState('');
  const [sceneColor, setSceneColor] = useState({ r: 255, g: 255, b: 255, a: 1 });
  const [sceneBrightness, setSceneBrightness] = useState(255);
  const [sceneDescription, setSceneDescription] = useState('');
  
  // Apply a scene
  const applyScene = (scene) => {
    if (connectionState !== CONNECTION_STATE.CONNECTED) return;
    
    // Turn on the light if it's off
    if (lightState.state !== 'ON') {
      setLight(true);
    }
    
    // Set brightness and color
    setBrightness(scene.brightness);
    setColor(scene.color);
  };
  
  // Open dialog to add a new scene
  const handleAddScene = () => {
    setEditingScene(null);
    setSceneName('');
    setSceneColor({ r: 255, g: 255, b: 255, a: 1 });
    setSceneBrightness(255);
    setSceneDescription('');
    setOpenDialog(true);
  };
  
  // Open dialog to edit a scene
  const handleEditScene = (scene) => {
    setEditingScene(scene);
    setSceneName(scene.name);
    setSceneColor({ ...scene.color, a: 1 });
    setSceneBrightness(scene.brightness);
    setSceneDescription(scene.description || '');
    setOpenDialog(true);
  };
  
  // Delete a scene
  const handleDeleteScene = (sceneId) => {
    const updatedScenes = scenes.filter(scene => scene.id !== sceneId);
    setScenes(updatedScenes);
    localStorage.setItem('custom_scenes', JSON.stringify(updatedScenes));
  };
  
  // Save a scene
  const handleSaveScene = () => {
    if (!sceneName.trim()) return;
    
    const newScene = {
      id: editingScene ? editingScene.id : `custom_${Date.now()}`,
      name: sceneName.trim(),
      color: {
        r: sceneColor.r,
        g: sceneColor.g,
        b: sceneColor.b
      },
      brightness: sceneBrightness,
      description: sceneDescription.trim(),
      custom: true
    };
    
    let updatedScenes;
    
    if (editingScene) {
      // Update existing scene
      updatedScenes = scenes.map(scene => 
        scene.id === editingScene.id ? newScene : scene
      );
    } else {
      // Add new scene
      updatedScenes = [...scenes, newScene];
    }
    
    setScenes(updatedScenes);
    localStorage.setItem('custom_scenes', JSON.stringify(updatedScenes));
    setOpenDialog(false);
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
        Light Scenes
      </Typography>
      
      <Typography variant="body1" paragraph>
        Select a scene to quickly set your light to a predefined color and brightness.
      </Typography>
      
      <Typography variant="h6" gutterBottom sx={{ mt: 3 }}>
        Predefined Scenes
      </Typography>
      
      <Grid container spacing={3}>
        {predefinedScenes.map((scene) => (
          <Grid item xs={12} sm={6} md={4} key={scene.id}>
            <Card 
              sx={{ 
                height: '100%',
                transition: 'transform 0.2s, box-shadow 0.2s',
                '&:hover': {
                  transform: 'translateY(-4px)',
                  boxShadow: 4
                }
              }}
            >
              <CardActionArea 
                onClick={() => applyScene(scene)}
                sx={{ height: '100%' }}
              >
                <CardContent>
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                    <Box 
                      sx={{ 
                        mr: 2,
                        color: `rgb(${scene.color.r}, ${scene.color.g}, ${scene.color.b})`,
                        filter: 'drop-shadow(0 0 3px rgba(0,0,0,0.2))'
                      }}
                    >
                      {scene.icon}
                    </Box>
                    <Typography variant="h6">
                      {scene.name}
                    </Typography>
                  </Box>
                  
                  <Typography variant="body2" color="text.secondary" paragraph>
                    {scene.description}
                  </Typography>
                  
                  <Box sx={{ display: 'flex', alignItems: 'center' }}>
                    <Box 
                      sx={{ 
                        width: 24, 
                        height: 24, 
                        borderRadius: '50%', 
                        bgcolor: `rgb(${scene.color.r}, ${scene.color.g}, ${scene.color.b})`,
                        mr: 1,
                        border: '1px solid #ddd'
                      }} 
                    />
                    <Typography variant="body2" color="text.secondary">
                      Brightness: {Math.round(scene.brightness / 255 * 100)}%
                    </Typography>
                  </Box>
                </CardContent>
              </CardActionArea>
            </Card>
          </Grid>
        ))}
      </Grid>
      
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mt: 4, mb: 2 }}>
        <Typography variant="h6">
          Custom Scenes
        </Typography>
        <Button 
          variant="contained" 
          startIcon={<AddIcon />}
          onClick={handleAddScene}
        >
          Add Scene
        </Button>
      </Box>
      
      {scenes.length === 0 ? (
        <Paper sx={{ p: 3, textAlign: 'center', bgcolor: 'background.default' }}>
          <Typography variant="body1" color="text.secondary">
            You haven't created any custom scenes yet.
          </Typography>
        </Paper>
      ) : (
        <Grid container spacing={3}>
          {scenes.map((scene) => (
            <Grid item xs={12} sm={6} md={4} key={scene.id}>
              <Card sx={{ height: '100%' }}>
                <CardContent>
                  <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
                    <Typography variant="h6">
                      {scene.name}
                    </Typography>
                    <Box>
                      <IconButton 
                        size="small" 
                        onClick={() => handleEditScene(scene)}
                        sx={{ mr: 1 }}
                      >
                        <EditIcon fontSize="small" />
                      </IconButton>
                      <IconButton 
                        size="small" 
                        onClick={() => handleDeleteScene(scene.id)}
                      >
                        <DeleteIcon fontSize="small" />
                      </IconButton>
                    </Box>
                  </Box>
                  
                  <Typography variant="body2" color="text.secondary" paragraph>
                    {scene.description || 'No description'}
                  </Typography>
                  
                  <Box sx={{ display: 'flex', alignItems: 'center', mb: 2 }}>
                    <Box 
                      sx={{ 
                        width: 24, 
                        height: 24, 
                        borderRadius: '50%', 
                        bgcolor: `rgb(${scene.color.r}, ${scene.color.g}, ${scene.color.b})`,
                        mr: 1,
                        border: '1px solid #ddd'
                      }} 
                    />
                    <Typography variant="body2" color="text.secondary">
                      Brightness: {Math.round(scene.brightness / 255 * 100)}%
                    </Typography>
                  </Box>
                  
                  <Button 
                    variant="outlined" 
                    fullWidth
                    onClick={() => applyScene(scene)}
                  >
                    Apply Scene
                  </Button>
                </CardContent>
              </Card>
            </Grid>
          ))}
        </Grid>
      )}
      
      {/* Add/Edit Scene Dialog */}
      <Dialog open={openDialog} onClose={() => setOpenDialog(false)} maxWidth="sm" fullWidth>
        <DialogTitle>
          {editingScene ? 'Edit Scene' : 'Add New Scene'}
        </DialogTitle>
        <DialogContent>
          <TextField
            autoFocus
            margin="dense"
            label="Scene Name"
            fullWidth
            value={sceneName}
            onChange={(e) => setSceneName(e.target.value)}
            sx={{ mb: 2 }}
          />
          
          <TextField
            margin="dense"
            label="Description"
            fullWidth
            multiline
            rows={2}
            value={sceneDescription}
            onChange={(e) => setSceneDescription(e.target.value)}
            sx={{ mb: 3 }}
          />
          
          <Typography variant="subtitle2" gutterBottom>
            Brightness
          </Typography>
          <Slider
            value={sceneBrightness}
            min={0}
            max={255}
            onChange={(e, newValue) => setSceneBrightness(newValue)}
            valueLabelDisplay="auto"
            valueLabelFormat={(value) => `${Math.round(value / 255 * 100)}%`}
            sx={{ mb: 3 }}
          />
          
          <Typography variant="subtitle2" gutterBottom>
            Color
          </Typography>
          <Box sx={{ display: 'flex', justifyContent: 'center', mb: 2 }}>
            <SketchPicker 
              color={sceneColor}
              onChange={(color) => setSceneColor(color.rgb)}
              disableAlpha={true}
            />
          </Box>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setOpenDialog(false)}>Cancel</Button>
          <Button 
            onClick={handleSaveScene}
            variant="contained"
            startIcon={<SaveIcon />}
            disabled={!sceneName.trim()}
          >
            Save Scene
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
};

export default Scenes;
