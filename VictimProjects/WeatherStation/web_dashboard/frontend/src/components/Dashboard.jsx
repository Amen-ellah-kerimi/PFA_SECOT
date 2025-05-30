import { useState } from 'react';
import styled from 'styled-components';
import { 
  FaThermometerHalf, 
  FaTint, 
  FaServer, 
  FaChartLine,
  FaChevronDown,
  FaChevronUp,
  FaTrash
} from 'react-icons/fa';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';
import TemperatureChart from './TemperatureChart';
import HumidityChart from './HumidityChart';

const DashboardContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;
`;

const DashboardHeader = styled.div`
  margin-bottom: 20px;
  
  h1 {
    font-size: 1.8rem;
    color: var(--text-color);
    margin-bottom: 10px;
  }
  
  p {
    color: var(--text-secondary);
  }
`;

const CardGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
  margin-bottom: 20px;
`;

const Card = styled.div`
  background-color: white;
  border-radius: var(--card-border-radius);
  padding: 20px;
  box-shadow: var(--shadow);
`;

const SensorCard = styled(Card)`
  display: flex;
  flex-direction: column;
  
  .icon {
    font-size: 2rem;
    margin-bottom: 15px;
    color: ${props => props.color || 'var(--primary-color)'};
  }
  
  .title {
    font-size: 1rem;
    color: var(--text-secondary);
    margin-bottom: 5px;
  }
  
  .value {
    font-size: 2.5rem;
    font-weight: bold;
    color: var(--text-color);
    margin-bottom: 10px;
  }
  
  .min-max {
    display: flex;
    justify-content: space-between;
    color: var(--text-secondary);
    font-size: 0.9rem;
    
    span {
      font-weight: 500;
      color: var(--text-color);
    }
  }
`;

const DeviceCard = styled(Card)`
  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 15px;
    
    h2 {
      font-size: 1.2rem;
      display: flex;
      align-items: center;
      
      svg {
        margin-right: 8px;
        color: var(--primary-color);
      }
    }
  }
  
  .status-item {
    display: flex;
    justify-content: space-between;
    padding: 8px 0;
    border-bottom: 1px solid var(--border-color);
    
    &:last-child {
      border-bottom: none;
    }
    
    .label {
      color: var(--text-secondary);
    }
    
    .value {
      font-weight: 500;
    }
  }
`;

const ChartCard = styled(Card)`
  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 15px;
    
    h2 {
      font-size: 1.2rem;
      display: flex;
      align-items: center;
      
      svg {
        margin-right: 8px;
        color: ${props => props.color || 'var(--primary-color)'};
      }
    }
    
    .actions {
      display: flex;
      gap: 10px;
    }
  }
  
  .chart-container {
    height: ${props => props.expanded ? '400px' : '250px'};
    transition: height var(--transition-speed);
  }
`;

const ToggleButton = styled.button`
  background: none;
  border: none;
  color: var(--text-secondary);
  cursor: pointer;
  display: flex;
  align-items: center;
  padding: 5px;
  
  &:hover {
    color: var(--text-color);
  }
`;

const ClearButton = styled.button`
  background: none;
  border: none;
  color: #e74c3c;
  cursor: pointer;
  display: flex;
  align-items: center;
  padding: 5px;
  
  &:hover {
    color: #c0392b;
  }
`;

const NoDataMessage = styled.div`
  text-align: center;
  padding: 40px 20px;
  color: var(--text-secondary);
  
  h2 {
    margin-bottom: 10px;
    font-size: 1.5rem;
  }
  
  p {
    margin-bottom: 20px;
  }
  
  button {
    background-color: var(--primary-color);
    color: white;
    border: none;
    border-radius: 4px;
    padding: 10px 20px;
    font-size: 1rem;
    cursor: pointer;
    
    &:hover {
      background-color: #2980b9;
    }
  }
`;

function Dashboard() {
  const { 
    connectionState, 
    temperature, 
    humidity, 
    temperatureHistory, 
    humidityHistory,
    deviceStatus,
    connect,
    clearHistory
  } = useMqtt();
  
  const [tempChartExpanded, setTempChartExpanded] = useState(false);
  const [humidityChartExpanded, setHumidityChartExpanded] = useState(false);
  
  // Calculate min/max values
  const minTemp = temperatureHistory.length > 0 
    ? Math.min(...temperatureHistory.map(item => item.value))
    : null;
    
  const maxTemp = temperatureHistory.length > 0 
    ? Math.max(...temperatureHistory.map(item => item.value))
    : null;
    
  const minHumidity = humidityHistory.length > 0 
    ? Math.min(...humidityHistory.map(item => item.value))
    : null;
    
  const maxHumidity = humidityHistory.length > 0 
    ? Math.max(...humidityHistory.map(item => item.value))
    : null;
  
  // Format timestamp
  const formatTime = (timestamp) => {
    if (!timestamp) return '--';
    return new Date(timestamp).toLocaleTimeString();
  };
  
  // Format uptime
  const formatUptime = (milliseconds) => {
    if (!milliseconds) return '--';
    
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);
    
    if (days > 0) {
      return `${days}d ${hours % 24}h ${minutes % 60}m`;
    } else if (hours > 0) {
      return `${hours}h ${minutes % 60}m ${seconds % 60}s`;
    } else if (minutes > 0) {
      return `${minutes}m ${seconds % 60}s`;
    } else {
      return `${seconds}s`;
    }
  };
  
  // If not connected, show connection prompt
  if (connectionState !== CONNECTION_STATE.CONNECTED) {
    return (
      <DashboardContainer>
        <DashboardHeader>
          <h1>Weather Station Dashboard</h1>
          <p>Monitor your IoT weather station in real-time</p>
        </DashboardHeader>
        
        <NoDataMessage>
          <h2>Not Connected</h2>
          <p>Connect to the MQTT broker to view sensor data</p>
          <button onClick={connect}>Connect Now</button>
        </NoDataMessage>
      </DashboardContainer>
    );
  }
  
  return (
    <DashboardContainer>
      <DashboardHeader>
        <h1>Weather Station Dashboard</h1>
        <p>Monitor your IoT weather station in real-time</p>
      </DashboardHeader>
      
      <CardGrid>
        <SensorCard color="#3498db">
          <FaThermometerHalf className="icon" />
          <div className="title">Temperature</div>
          <div className="value">
            {temperature !== null ? `${temperature.toFixed(1)}°C` : '--'}
          </div>
          <div className="min-max">
            <div>Min: <span>{minTemp !== null ? `${minTemp.toFixed(1)}°C` : '--'}</span></div>
            <div>Max: <span>{maxTemp !== null ? `${maxTemp.toFixed(1)}°C` : '--'}</span></div>
          </div>
        </SensorCard>
        
        <SensorCard color="#2ecc71">
          <FaTint className="icon" />
          <div className="title">Humidity</div>
          <div className="value">
            {humidity !== null ? `${humidity.toFixed(1)}%` : '--'}
          </div>
          <div className="min-max">
            <div>Min: <span>{minHumidity !== null ? `${minHumidity.toFixed(1)}%` : '--'}</span></div>
            <div>Max: <span>{maxHumidity !== null ? `${maxHumidity.toFixed(1)}%` : '--'}</span></div>
          </div>
        </SensorCard>
        
        <DeviceCard>
          <div className="header">
            <h2><FaServer /> Device Status</h2>
          </div>
          <div className="status-item">
            <div className="label">Device ID</div>
            <div className="value">{deviceStatus.device_id || '--'}</div>
          </div>
          <div className="status-item">
            <div className="label">IP Address</div>
            <div className="value">{deviceStatus.ip || '--'}</div>
          </div>
          <div className="status-item">
            <div className="label">Status</div>
            <div className="value">{deviceStatus.status || '--'}</div>
          </div>
          <div className="status-item">
            <div className="label">Uptime</div>
            <div className="value">{formatUptime(deviceStatus.timestamp)}</div>
          </div>
          <div className="status-item">
            <div className="label">Last Update</div>
            <div className="value">{formatTime(temperatureHistory[temperatureHistory.length - 1]?.timestamp)}</div>
          </div>
        </DeviceCard>
      </CardGrid>
      
      <ChartCard color="#3498db" expanded={tempChartExpanded}>
        <div className="header">
          <h2><FaChartLine /> Temperature History</h2>
          <div className="actions">
            <ClearButton onClick={clearHistory} title="Clear history">
              <FaTrash />
            </ClearButton>
            <ToggleButton 
              onClick={() => setTempChartExpanded(!tempChartExpanded)}
              title={tempChartExpanded ? "Collapse" : "Expand"}
            >
              {tempChartExpanded ? <FaChevronUp /> : <FaChevronDown />}
            </ToggleButton>
          </div>
        </div>
        <div className="chart-container">
          <TemperatureChart data={temperatureHistory} />
        </div>
      </ChartCard>
      
      <ChartCard color="#2ecc71" expanded={humidityChartExpanded}>
        <div className="header">
          <h2><FaChartLine /> Humidity History</h2>
          <div className="actions">
            <ClearButton onClick={clearHistory} title="Clear history">
              <FaTrash />
            </ClearButton>
            <ToggleButton 
              onClick={() => setHumidityChartExpanded(!humidityChartExpanded)}
              title={humidityChartExpanded ? "Collapse" : "Expand"}
            >
              {humidityChartExpanded ? <FaChevronUp /> : <FaChevronDown />}
            </ToggleButton>
          </div>
        </div>
        <div className="chart-container">
          <HumidityChart data={humidityHistory} />
        </div>
      </ChartCard>
    </DashboardContainer>
  );
}

export default Dashboard;
