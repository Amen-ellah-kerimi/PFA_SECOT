import { useState } from 'react';
import { Link } from 'react-router-dom';
import styled from 'styled-components';
import { FaBars, FaThermometerHalf, FaSignOutAlt, FaSignInAlt } from 'react-icons/fa';
import { useMqtt, CONNECTION_STATE } from '../hooks/useMqtt';

const HeaderContainer = styled.header`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 20px;
  height: var(--header-height);
  background-color: white;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  position: relative;
  z-index: 100;
`;

const Logo = styled(Link)`
  display: flex;
  align-items: center;
  font-size: 1.5rem;
  font-weight: bold;
  color: var(--text-color);
  
  svg {
    margin-right: 10px;
    color: var(--primary-color);
  }
`;

const MenuButton = styled.button`
  background: none;
  border: none;
  font-size: 1.5rem;
  color: var(--text-color);
  cursor: pointer;
  display: flex;
  align-items: center;
  padding: 0;
  
  @media (min-width: 769px) {
    display: none;
  }
`;

const ConnectionStatus = styled.div`
  display: flex;
  align-items: center;
  margin-right: 20px;
  font-size: 0.9rem;
`;

const StatusIndicator = styled.span`
  display: inline-block;
  width: 10px;
  height: 10px;
  border-radius: 50%;
  margin-right: 8px;
  background-color: ${props => {
    switch (props.status) {
      case CONNECTION_STATE.CONNECTED:
        return 'var(--secondary-color)';
      case CONNECTION_STATE.CONNECTING:
        return '#f39c12';
      default:
        return '#e74c3c';
    }
  }};
`;

const Actions = styled.div`
  display: flex;
  align-items: center;
`;

const ConnectButton = styled.button`
  display: flex;
  align-items: center;
  background-color: ${props => props.isConnected ? '#e74c3c' : 'var(--primary-color)'};
  color: white;
  border: none;
  border-radius: 4px;
  padding: 8px 12px;
  cursor: pointer;
  font-size: 0.9rem;
  
  svg {
    margin-right: 5px;
  }
  
  &:hover {
    background-color: ${props => props.isConnected ? '#c0392b' : '#2980b9'};
  }
`;

function Header({ toggleSidebar }) {
  const { connectionState, connect, disconnect } = useMqtt();
  
  const handleConnectionToggle = () => {
    if (connectionState === CONNECTION_STATE.CONNECTED) {
      disconnect();
    } else {
      connect();
    }
  };
  
  return (
    <HeaderContainer>
      <MenuButton onClick={toggleSidebar}>
        <FaBars />
      </MenuButton>
      
      <Logo to="/">
        <FaThermometerHalf />
        <span>Weather Station</span>
      </Logo>
      
      <Actions>
        <ConnectionStatus>
          <StatusIndicator status={connectionState} />
          <span>
            {connectionState === CONNECTION_STATE.CONNECTED ? 'Connected' : 
             connectionState === CONNECTION_STATE.CONNECTING ? 'Connecting...' : 'Disconnected'}
          </span>
        </ConnectionStatus>
        
        <ConnectButton 
          isConnected={connectionState === CONNECTION_STATE.CONNECTED}
          onClick={handleConnectionToggle}
        >
          {connectionState === CONNECTION_STATE.CONNECTED ? (
            <>
              <FaSignOutAlt />
              <span>Disconnect</span>
            </>
          ) : (
            <>
              <FaSignInAlt />
              <span>Connect</span>
            </>
          )}
        </ConnectButton>
      </Actions>
    </HeaderContainer>
  );
}

export default Header;
