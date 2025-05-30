import { useState } from 'react';
import { 
  Box, 
  Grid, 
  Paper, 
  Typography, 
  Card, 
  CardContent,
  Divider,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  ToggleButtonGroup,
  ToggleButton,
  Alert,
  Button,
  IconButton
} from '@mui/material';
import { 
  Lightbulb as LightbulbIcon,
  BarChart as BarChartIcon,
  Timeline as TimelineIcon,
  AccessTime as TimeIcon,
  CalendarMonth as CalendarIcon,
  ExpandMore as ExpandMoreIcon,
  ExpandLess as ExpandLessIcon,
  Refresh as RefreshIcon
} from '@mui/icons-material';
import { Line, Bar } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  BarElement,
  Title,
  Tooltip,
  Legend,
  Filler,
} from 'chart.js';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

// Register ChartJS components
ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  BarElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

const Analytics = () => {
  const { 
    connectionState, 
    lightState, 
    brightnessHistory, 
    ambientHistory, 
    motionHistory,
    connect, 
    clearHistory 
  } = useMqtt();
  
  const [timeRange, setTimeRange] = useState('hour');
  const [chartType, setChartType] = useState('line');
  const [dataType, setDataType] = useState('brightness');
  const [expanded, setExpanded] = useState(false);
  
  // Format time for display
  const formatTime = (date) => {
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };
  
  // Get data based on selected type
  const getChartData = () => {
    let data = [];
    let label = '';
    let color = '';
    
    switch (dataType) {
      case 'brightness':
        data = brightnessHistory;
        label = 'Brightness';
        color = '#ff9800';
        break;
      case 'ambient':
        data = ambientHistory;
        label = 'Ambient Light';
        color = '#2196f3';
        break;
      case 'motion':
        data = motionHistory;
        label = 'Motion Detected';
        color = '#4caf50';
        break;
      default:
        data = brightnessHistory;
        label = 'Brightness';
        color = '#ff9800';
    }
    
    return {
      labels: data.map(item => formatTime(item.timestamp)),
      datasets: [
        {
          label: label,
          data: data.map(item => item.value),
          borderColor: color,
          backgroundColor: `${color}33`,
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: data.length > 20 ? 0 : 3,
          pointHoverRadius: 5,
        },
      ],
    };
  };
  
  // Chart options
  const getChartOptions = () => {
    return {
      responsive: true,
      maintainAspectRatio: false,
      animation: {
        duration: 500,
      },
      scales: {
        y: {
          beginAtZero: true,
          title: {
            display: true,
            text: dataType === 'brightness' ? 'Brightness (0-255)' : 
                 dataType === 'ambient' ? 'Ambient Light (%)' : 
                 'Motion Detected',
          },
          max: dataType === 'motion' ? 1 : undefined,
          ticks: {
            callback: (value) => {
              if (dataType === 'motion') {
                return value === 1 ? 'Yes' : 'No';
              }
              return value;
            },
          },
        },
        x: {
          title: {
            display: true,
            text: 'Time',
          },
          ticks: {
            maxRotation: 45,
            minRotation: 45,
          },
        },
      },
      plugins: {
        legend: {
          display: true,
          position: 'top',
        },
        tooltip: {
          callbacks: {
            label: (context) => {
              if (dataType === 'motion') {
                return `Motion: ${context.parsed.y === 1 ? 'Detected' : 'None'}`;
              }
              return `${context.dataset.label}: ${context.parsed.y}`;
            },
          },
        },
      },
    };
  };
  
  // Calculate usage statistics
  const calculateStats = () => {
    // For demonstration purposes, we'll generate some stats
    // In a real app, these would be calculated from actual data
    
    return {
      totalOnTime: '12h 34m',
      averageBrightness: '75%',
      motionDetections: 42,
      energySaved: '1.2 kWh',
      mostUsedColor: 'Warm White',
      peakUsageTime: '19:00 - 22:00',
    };
  };
  
  const stats = calculateStats();
  
  // If not connected, show connection prompt
  if (connectionState !== CONNECTION_STATE.CONNECTED) {
    return (
      <Box sx={{ mt: 4 }}>
        <Alert 
          severity="info" 
          sx={{ mb: 2 }}
        >
          Connect to the MQTT broker to view analytics data
        </Alert>
        
        <Paper sx={{ p: 3, textAlign: 'center' }}>
          <BarChartIcon sx={{ fontSize: 60, color: 'text.secondary', mb: 2 }} />
          <Typography variant="h5" gutterBottom>
            Not Connected
          </Typography>
          <Typography variant="body1" color="text.secondary" paragraph>
            You need to connect to the MQTT broker to view analytics data.
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
        Analytics
      </Typography>
      
      <Grid container spacing={3}>
        {/* Chart Card */}
        <Grid item xs={12}>
          <Paper sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
              <Typography variant="h6">
                Light Usage Data
              </Typography>
              <Box>
                <IconButton onClick={clearHistory} title="Clear history data">
                  <RefreshIcon />
                </IconButton>
              </Box>
            </Box>
            
            <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 2, mb: 3 }}>
              {/* Data Type Selector */}
              <FormControl sx={{ minWidth: 150 }}>
                <InputLabel id="data-type-label">Data Type</InputLabel>
                <Select
                  labelId="data-type-label"
                  value={dataType}
                  label="Data Type"
                  onChange={(e) => setDataType(e.target.value)}
                >
                  <MenuItem value="brightness">Brightness</MenuItem>
                  <MenuItem value="ambient">Ambient Light</MenuItem>
                  <MenuItem value="motion">Motion Detection</MenuItem>
                </Select>
              </FormControl>
              
              {/* Time Range Selector */}
              <FormControl sx={{ minWidth: 150 }}>
                <InputLabel id="time-range-label">Time Range</InputLabel>
                <Select
                  labelId="time-range-label"
                  value={timeRange}
                  label="Time Range"
                  onChange={(e) => setTimeRange(e.target.value)}
                >
                  <MenuItem value="hour">Last Hour</MenuItem>
                  <MenuItem value="day">Last 24 Hours</MenuItem>
                  <MenuItem value="week">Last Week</MenuItem>
                  <MenuItem value="month">Last Month</MenuItem>
                </Select>
              </FormControl>
              
              {/* Chart Type Selector */}
              <ToggleButtonGroup
                value={chartType}
                exclusive
                onChange={(e, newValue) => {
                  if (newValue !== null) {
                    setChartType(newValue);
                  }
                }}
                aria-label="chart type"
              >
                <ToggleButton value="line" aria-label="line chart">
                  <TimelineIcon />
                </ToggleButton>
                <ToggleButton value="bar" aria-label="bar chart">
                  <BarChartIcon />
                </ToggleButton>
              </ToggleButtonGroup>
            </Box>
            
            {/* Chart */}
            <Box sx={{ height: 400, mb: 3 }}>
              {(dataType === 'brightness' && brightnessHistory.length === 0) ||
               (dataType === 'ambient' && ambientHistory.length === 0) ||
               (dataType === 'motion' && motionHistory.length === 0) ? (
                <Box sx={{ 
                  height: '100%', 
                  display: 'flex', 
                  alignItems: 'center', 
                  justifyContent: 'center',
                  flexDirection: 'column',
                  color: 'text.secondary',
                }}>
                  <BarChartIcon sx={{ fontSize: 48, mb: 2 }} />
                  <Typography variant="h6" gutterBottom>
                    No Data Available
                  </Typography>
                  <Typography variant="body2">
                    There is no {dataType} data to display. Use the light to generate data.
                  </Typography>
                </Box>
              ) : chartType === 'line' ? (
                <Line data={getChartData()} options={getChartOptions()} />
              ) : (
                <Bar data={getChartData()} options={getChartOptions()} />
              )}
            </Box>
          </Paper>
        </Grid>
        
        {/* Stats Cards */}
        <Grid item xs={12}>
          <Paper sx={{ p: 3 }}>
            <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
              <Typography variant="h6">
                Usage Statistics
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
            
            <Grid container spacing={3}>
              <Grid item xs={12} sm={6} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                      Total On Time
                    </Typography>
                    <Typography variant="h5">
                      {stats.totalOnTime}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
              
              <Grid item xs={12} sm={6} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                      Average Brightness
                    </Typography>
                    <Typography variant="h5">
                      {stats.averageBrightness}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
              
              <Grid item xs={12} sm={6} md={4}>
                <Card>
                  <CardContent>
                    <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                      Motion Detections
                    </Typography>
                    <Typography variant="h5">
                      {stats.motionDetections}
                    </Typography>
                  </CardContent>
                </Card>
              </Grid>
              
              {expanded && (
                <>
                  <Grid item xs={12} sm={6} md={4}>
                    <Card>
                      <CardContent>
                        <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                          Energy Saved
                        </Typography>
                        <Typography variant="h5">
                          {stats.energySaved}
                        </Typography>
                      </CardContent>
                    </Card>
                  </Grid>
                  
                  <Grid item xs={12} sm={6} md={4}>
                    <Card>
                      <CardContent>
                        <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                          Most Used Color
                        </Typography>
                        <Typography variant="h5">
                          {stats.mostUsedColor}
                        </Typography>
                      </CardContent>
                    </Card>
                  </Grid>
                  
                  <Grid item xs={12} sm={6} md={4}>
                    <Card>
                      <CardContent>
                        <Typography variant="subtitle2" color="text.secondary" gutterBottom>
                          Peak Usage Time
                        </Typography>
                        <Typography variant="h5">
                          {stats.peakUsageTime}
                        </Typography>
                      </CardContent>
                    </Card>
                  </Grid>
                </>
              )}
            </Grid>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default Analytics;
