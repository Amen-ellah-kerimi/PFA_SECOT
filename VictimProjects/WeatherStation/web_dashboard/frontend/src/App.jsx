import { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import Header from './components/Header';
import Sidebar from './components/Sidebar';
import Dashboard from './components/Dashboard';
import Settings from './components/Settings';
import About from './components/About';
import { MqttProvider } from './hooks/useMqtt';
import styled from 'styled-components';

const AppContainer = styled.div`
  display: flex;
  flex-direction: column;
  min-height: 100vh;
`;

const MainContainer = styled.div`
  display: flex;
  flex: 1;
`;

const ContentContainer = styled.main`
  flex: 1;
  padding: 20px;
  overflow-y: auto;
`;

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
        <AppContainer>
          <Header toggleSidebar={toggleSidebar} />
          <MainContainer>
            <Sidebar isOpen={sidebarOpen} />
            <ContentContainer>
              <Routes>
                <Route path="/" element={<Dashboard />} />
                <Route path="/settings" element={<Settings />} />
                <Route path="/about" element={<About />} />
              </Routes>
            </ContentContainer>
          </MainContainer>
        </AppContainer>
      </MqttProvider>
    </Router>
  );
}

export default App;
