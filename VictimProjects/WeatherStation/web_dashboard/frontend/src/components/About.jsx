import styled from 'styled-components';
import { FaInfoCircle, FaGithub, FaCode, FaShieldAlt } from 'react-icons/fa';

const AboutContainer = styled.div`
  max-width: 800px;
  margin: 0 auto;
`;

const AboutHeader = styled.div`
  margin-bottom: 20px;
  
  h1 {
    font-size: 1.8rem;
    color: var(--text-color);
    margin-bottom: 10px;
    display: flex;
    align-items: center;
    
    svg {
      margin-right: 10px;
      color: var(--primary-color);
    }
  }
  
  p {
    color: var(--text-secondary);
  }
`;

const AboutCard = styled.div`
  background-color: white;
  border-radius: var(--card-border-radius);
  padding: 20px;
  box-shadow: var(--shadow);
  margin-bottom: 20px;
`;

const Section = styled.div`
  margin-bottom: 30px;
  
  &:last-child {
    margin-bottom: 0;
  }
  
  h2 {
    font-size: 1.2rem;
    margin-bottom: 15px;
    padding-bottom: 10px;
    border-bottom: 1px solid var(--border-color);
    display: flex;
    align-items: center;
    
    svg {
      margin-right: 10px;
      color: var(--primary-color);
    }
  }
  
  p {
    margin-bottom: 15px;
    line-height: 1.6;
  }
  
  ul {
    margin-left: 20px;
    margin-bottom: 15px;
    
    li {
      margin-bottom: 5px;
    }
  }
`;

function About() {
  return (
    <AboutContainer>
      <AboutHeader>
        <h1><FaInfoCircle /> About</h1>
        <p>Information about the Weather Station project</p>
      </AboutHeader>
      
      <AboutCard>
        <Section>
          <h2><FaCode /> Weather Station Dashboard</h2>
          <p>
            This dashboard is part of the IoT Weather Station project, which demonstrates a complete
            IoT system with hardware, mobile app, web dashboard, and MQTT communication.
          </p>
          <p>
            The dashboard allows you to monitor temperature and humidity data from your IoT weather
            station in real-time, view historical data, and configure connection settings.
          </p>
        </Section>
        
        <Section>
          <h2><FaShieldAlt /> Security Considerations</h2>
          <p>
            This project is designed for educational purposes and includes both secure and insecure
            deployment options:
          </p>
          <ul>
            <li><strong>Basic Deployment:</strong> No encryption or authentication (for development only)</li>
            <li><strong>Secure Deployment:</strong> TLS/SSL encryption and user authentication</li>
          </ul>
          <p>
            In a production environment, consider implementing:
          </p>
          <ul>
            <li>Proper certificate management</li>
            <li>Strong authentication mechanisms</li>
            <li>Secure storage of credentials</li>
            <li>Regular security updates</li>
            <li>Network segmentation</li>
          </ul>
        </Section>
        
        <Section>
          <h2><FaGithub /> Project Information</h2>
          <p>
            This project is part of the SECoT (Security Evaluation and Compromise Toolkit) and is
            designed to demonstrate:
          </p>
          <ul>
            <li>How real IoT systems are built and deployed</li>
            <li>Communication between IoT devices and applications</li>
            <li>Security vulnerabilities in IoT systems</li>
            <li>Best practices for securing IoT deployments</li>
          </ul>
          <p>
            <strong>Version:</strong> 1.0.0
          </p>
          <p>
            <strong>License:</strong> This code is provided for educational purposes as part of the
            SECoT (Security Evaluation and Compromise Toolkit) project.
          </p>
        </Section>
      </AboutCard>
    </AboutContainer>
  );
}

export default About;
