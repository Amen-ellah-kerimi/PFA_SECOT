import { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { Box, CssBaseline } from '@mui/material';
import Header from './components/Header';
import Sidebar from './components/Sidebar';
import Dashboard from './components/Dashboard';
import LightControl from './components/LightControl';
import Scenes from './components/Scenes';
import Analytics from './components/Analytics';
import Settings from './components/Settings';
import About from './components/About';
import { MqttProvider } from './hooks/useMqtt';

function App() {
  const [sidebarOpen, setSidebarOpen] = useState(window.innerWidth > 768);

  useEffect(() => {
    const handleResize = () => {
      setSidebarOpen(window.innerWidth > 768);
    };

    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, []);

  const toggleSidebar = () => {
    setSidebarOpen(!sidebarOpen);
  };

  return (
    <Router>
      <MqttProvider>
        <Box sx={{ display: 'flex', height: '100vh' }}>
          <CssBaseline />
          <Header toggleSidebar={toggleSidebar} />
          <Sidebar isOpen={sidebarOpen} />
          <Box
            component="main"
            sx={{
              flexGrow: 1,
              p: 3,
              mt: '64px',
              ml: { sm: sidebarOpen ? '240px' : 0 },
              transition: 'margin 0.2s',
              overflow: 'auto'
            }}
          >
            <Routes>
              <Route path="/" element={<Dashboard />} />
              <Route path="/control" element={<LightControl />} />
              <Route path="/scenes" element={<Scenes />} />
              <Route path="/analytics" element={<Analytics />} />
              <Route path="/settings" element={<Settings />} />
              <Route path="/about" element={<About />} />
            </Routes>
          </Box>
        </Box>
      </MqttProvider>
    </Router>
  );
}

export default App;
