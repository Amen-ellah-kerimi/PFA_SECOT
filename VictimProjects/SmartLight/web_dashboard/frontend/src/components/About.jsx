import { 
  Box, 
  Paper, 
  Typography, 
  Grid,
  Card,
  CardContent,
  Divider,
  Link,
  List,
  ListItem,
  ListItemIcon,
  ListItemText,
  Chip
} from '@mui/material';
import { 
  Info as InfoIcon,
  Security as SecurityIcon,
  Code as CodeIcon,
  GitHub as GitHubIcon,
  Lightbulb as LightbulbIcon,
  BugReport as BugIcon,
  CheckCircle as CheckIcon,
  Warning as WarningIcon
} from '@mui/icons-material';

const About = () => {
  return (
    <Box>
      <Typography variant="h4" gutterBottom>
        About
      </Typography>
      
      <Grid container spacing={3}>
        <Grid item xs={12} md={8}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h5" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <LightbulbIcon sx={{ mr: 1 }} />
              Smart Light Control System
            </Typography>
            
            <Typography variant="body1" paragraph>
              This dashboard is part of the IoT Smart Light Control System, which demonstrates a complete
              IoT system with hardware, mobile app, web dashboard, and MQTT communication.
            </Typography>
            
            <Typography variant="body1" paragraph>
              The system allows you to control RGB LEDs, monitor ambient light levels, detect motion,
              and create custom lighting scenes. It's designed to showcase both the functionality and
              security aspects of IoT systems.
            </Typography>
            
            <Divider sx={{ my: 3 }} />
            
            <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <SecurityIcon sx={{ mr: 1 }} />
              Security Considerations
            </Typography>
            
            <Typography variant="body1" paragraph>
              This project is designed for educational purposes and includes both secure and insecure
              deployment options:
            </Typography>
            
            <Grid container spacing={2} sx={{ mb: 3 }}>
              <Grid item xs={12} sm={6}>
                <Card sx={{ height: '100%', bgcolor: '#ffebee' }}>
                  <CardContent>
                    <Typography variant="subtitle1" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
                      <WarningIcon sx={{ mr: 1, color: '#f44336' }} />
                      Insecure Deployment
                    </Typography>
                    <List dense>
                      <ListItem>
                        <ListItemText primary="No encryption (plain MQTT)" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="No authentication" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Plaintext credentials" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="No input validation" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Insecure default settings" />
                      </ListItem>
                    </List>
                  </CardContent>
                </Card>
              </Grid>
              
              <Grid item xs={12} sm={6}>
                <Card sx={{ height: '100%', bgcolor: '#e8f5e9' }}>
                  <CardContent>
                    <Typography variant="subtitle1" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
                      <CheckIcon sx={{ mr: 1, color: '#4caf50' }} />
                      Secure Deployment
                    </Typography>
                    <List dense>
                      <ListItem>
                        <ListItemText primary="TLS/SSL encryption" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Strong authentication" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Secure credential storage" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Input validation" />
                      </ListItem>
                      <ListItem>
                        <ListItemText primary="Secure default settings" />
                      </ListItem>
                    </List>
                  </CardContent>
                </Card>
              </Grid>
            </Grid>
            
            <Typography variant="body1" paragraph>
              In a production environment, always implement proper security measures including:
            </Typography>
            
            <Box sx={{ display: 'flex', flexWrap: 'wrap', gap: 1, mb: 3 }}>
              <Chip label="Proper certificate management" color="primary" />
              <Chip label="Strong authentication" color="primary" />
              <Chip label="Secure credential storage" color="primary" />
              <Chip label="Regular security updates" color="primary" />
              <Chip label="Network segmentation" color="primary" />
              <Chip label="Input validation" color="primary" />
              <Chip label="Logging and monitoring" color="primary" />
            </Box>
            
            <Divider sx={{ my: 3 }} />
            
            <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <CodeIcon sx={{ mr: 1 }} />
              Technical Details
            </Typography>
            
            <Typography variant="body1" paragraph>
              This project is built using the following technologies:
            </Typography>
            
            <Grid container spacing={2}>
              <Grid item xs={12} sm={6}>
                <Typography variant="subtitle2" gutterBottom>
                  Hardware:
                </Typography>
                <List dense>
                  <ListItem>
                    <ListItemText primary="NodeMCU ESP8266" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="RGB LEDs" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="LDR (Light Dependent Resistor)" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="PIR Motion Sensor" />
                  </ListItem>
                </List>
              </Grid>
              
              <Grid item xs={12} sm={6}>
                <Typography variant="subtitle2" gutterBottom>
                  Software:
                </Typography>
                <List dense>
                  <ListItem>
                    <ListItemText primary="React with Vite (Frontend)" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="Flask (Backend)" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="MQTT Protocol" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="Docker (Deployment)" />
                  </ListItem>
                  <ListItem>
                    <ListItemText primary="Flutter (Mobile App)" />
                  </ListItem>
                </List>
              </Grid>
            </Grid>
          </Paper>
        </Grid>
        
        <Grid item xs={12} md={4}>
          <Paper sx={{ p: 3 }}>
            <Typography variant="h6" gutterBottom sx={{ display: 'flex', alignItems: 'center' }}>
              <InfoIcon sx={{ mr: 1 }} />
              Project Information
            </Typography>
            
            <List>
              <ListItem>
                <ListItemText 
                  primary="Version" 
                  secondary="1.0.0"
                />
              </ListItem>
              <ListItem>
                <ListItemText 
                  primary="Released" 
                  secondary="November 2023"
                />
              </ListItem>
              <ListItem>
                <ListItemText 
                  primary="License" 
                  secondary="Educational Use Only"
                />
              </ListItem>
            </List>
            
            <Divider sx={{ my: 2 }} />
            
            <Typography variant="subtitle2" gutterBottom>
              Part of SECoT Project
            </Typography>
            
            <Typography variant="body2" paragraph>
              This project is part of the SECoT (Security Evaluation and Compromise Toolkit) and is
              designed to demonstrate:
            </Typography>
            
            <List dense>
              <ListItem>
                <ListItemIcon>
                  <CheckIcon color="primary" fontSize="small" />
                </ListItemIcon>
                <ListItemText primary="How real IoT systems are built and deployed" />
              </ListItem>
              <ListItem>
                <ListItemIcon>
                  <CheckIcon color="primary" fontSize="small" />
                </ListItemIcon>
                <ListItemText primary="Communication between IoT devices and applications" />
              </ListItem>
              <ListItem>
                <ListItemIcon>
                  <CheckIcon color="primary" fontSize="small" />
                </ListItemIcon>
                <ListItemText primary="Security vulnerabilities in IoT systems" />
              </ListItem>
              <ListItem>
                <ListItemIcon>
                  <CheckIcon color="primary" fontSize="small" />
                </ListItemIcon>
                <ListItemText primary="Best practices for securing IoT deployments" />
              </ListItem>
            </List>
            
            <Divider sx={{ my: 2 }} />
            
            <Typography variant="subtitle2" gutterBottom>
              Reporting Issues
            </Typography>
            
            <Typography variant="body2" paragraph>
              If you find any bugs or have suggestions for improvements, please report them through the appropriate channels.
            </Typography>
            
            <Box sx={{ display: 'flex', justifyContent: 'center', mt: 3 }}>
              <Chip 
                icon={<BugIcon />} 
                label="Report an Issue" 
                color="primary" 
                variant="outlined"
                clickable
              />
            </Box>
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
};

export default About;
