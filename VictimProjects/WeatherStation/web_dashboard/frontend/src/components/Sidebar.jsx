import { NavLink } from 'react-router-dom';
import styled from 'styled-components';
import { 
  FaHome, 
  FaCog, 
  FaInfoCircle,
  FaThermometerHalf,
  FaTint
} from 'react-icons/fa';
import { useMqtt } from '../hooks/useMqtt';

const SidebarContainer = styled.aside`
  width: ${props => props.isOpen ? 'var(--sidebar-width)' : '0'};
  height: calc(100vh - var(--header-height));
  background-color: white;
  box-shadow: 2px 0 5px rgba(0, 0, 0, 0.1);
  overflow-x: hidden;
  transition: width var(--transition-speed);
  position: relative;
  z-index: 90;
`;

const NavMenu = styled.nav`
  padding: 20px 0;
`;

const NavItem = styled(NavLink)`
  display: flex;
  align-items: center;
  padding: 12px 20px;
  color: var(--text-color);
  transition: background-color var(--transition-speed);
  white-space: nowrap;
  
  &:hover {
    background-color: var(--background-color);
  }
  
  &.active {
    background-color: var(--background-color);
    border-left: 4px solid var(--primary-color);
    font-weight: 500;
  }
  
  svg {
    margin-right: 10px;
    font-size: 1.2rem;
  }
`;

const SensorStatus = styled.div`
  padding: 20px;
  border-top: 1px solid var(--border-color);
  margin-top: auto;
`;

const SensorItem = styled.div`
  display: flex;
  align-items: center;
  margin-bottom: 15px;
  
  svg {
    margin-right: 10px;
    font-size: 1.2rem;
    color: ${props => props.color || 'var(--text-color)'};
  }
  
  .value {
    font-weight: 500;
    font-size: 1.1rem;
  }
  
  .unit {
    font-size: 0.9rem;
    color: var(--text-secondary);
    margin-left: 5px;
  }
`;

function Sidebar({ isOpen }) {
  const { temperature, humidity } = useMqtt();
  
  return (
    <SidebarContainer isOpen={isOpen}>
      <NavMenu>
        <NavItem to="/" end>
          <FaHome />
          <span>Dashboard</span>
        </NavItem>
        <NavItem to="/settings">
          <FaCog />
          <span>Settings</span>
        </NavItem>
        <NavItem to="/about">
          <FaInfoCircle />
          <span>About</span>
        </NavItem>
      </NavMenu>
      
      <SensorStatus>
        <SensorItem color="#3498db">
          <FaThermometerHalf />
          <div>
            <span className="value">
              {temperature !== null ? temperature.toFixed(1) : '--'}
            </span>
            <span className="unit">°C</span>
          </div>
        </SensorItem>
        
        <SensorItem color="#2ecc71">
          <FaTint />
          <div>
            <span className="value">
              {humidity !== null ? humidity.toFixed(1) : '--'}
            </span>
            <span className="unit">%</span>
          </div>
        </SensorItem>
      </SensorStatus>
    </SidebarContainer>
  );
}

export default Sidebar;
