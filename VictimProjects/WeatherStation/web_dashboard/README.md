# Weather Station - Web Dashboard

This directory contains a modern web-based dashboard for monitoring the IoT Weather Station, built with React (frontend) and Flask (backend).

## Features

- Real-time temperature and humidity monitoring
- Historical data visualization with charts
- MQTT connection management
- Device status monitoring
- Responsive design for desktop and mobile devices
- Collapsible chart sections for better UI experience

## Architecture

The dashboard consists of two main components:

1. **Frontend**: React application built with Vite
   - Modern component-based architecture
   - Real-time data visualization with Chart.js
   - MQTT over WebSockets for live updates
   - Responsive UI with styled-components

2. **Backend**: Python Flask API
   - Handles MQTT communication with the broker
   - Provides REST API endpoints for data access
   - Maintains data history
   - Serves the React frontend

## Prerequisites

- Node.js and npm (for frontend development)
- Python 3.7+ (for backend development)
- Docker and Docker Compose (for deployment)
- An MQTT broker with WebSocket support (e.g., Mosquitto)
- The NodeMCU Weather Station device running and publishing data

## Project Structure

```bash
web_dashboard/
├── frontend/                # React frontend
│   ├── src/                 # Source code
│   │   ├── components/      # React components
│   │   ├── hooks/           # Custom React hooks
│   │   └── services/        # API services
│   ├── public/              # Static assets
│   ├── package.json         # Dependencies
│   └── vite.config.js       # Vite configuration
├── backend/                 # Flask backend
│   ├── app.py               # Main application
│   └── requirements.txt     # Python dependencies
└── README.md                # This file
```

## Development

### Frontend

```bash
cd frontend
npm install
npm run dev
```

The development server will start at `http://localhost:5173` with hot module replacement.

### Backend

```bash
cd backend
pip install -r requirements.txt
python app.py
```

The Flask server will start at `http://localhost:5000`.

## Building for Production

The project can be built and deployed using Docker Compose:

```bash
# From the deployment directory
docker-compose up -d
```

This will:

1. Build the React frontend
2. Build the Flask backend
3. Start the MQTT broker
4. Start the backend API
5. Start the frontend server

## Configuration

The dashboard connects to an MQTT broker to receive temperature and humidity data. Default configuration:

- MQTT Broker: localhost (configurable via environment variables)
- MQTT Port: 1883 (standard MQTT), 9001 (WebSockets)
- API Endpoint: `http://localhost:5000/api`
- Topics:
  - Temperature: home/weatherstation/temperature
  - Humidity: home/weatherstation/humidity
  - Status: home/weatherstation/status
  - Combined data: home/weatherstation/data

## Security Considerations

This dashboard is designed for educational purposes and includes both secure and insecure deployment options:

- **Basic Deployment**: No encryption or authentication (for development only)
- **Secure Deployment**: TLS/SSL encryption and user authentication

In a production environment, consider implementing:

- Proper certificate management
- Strong authentication mechanisms
- Secure storage of credentials
- Regular security updates
- Input validation and sanitization
- Cross-site scripting (XSS) protection

## Troubleshooting

- **Cannot connect to MQTT broker**: Verify the broker address and port. Ensure the broker has WebSocket support enabled and is accessible from your network.
- **No data received**: Check that the NodeMCU is publishing to the correct topics.
- **API errors**: Check the Flask logs for backend issues.
- **Frontend not loading**: Ensure the backend API is running and accessible.

## License

This code is provided for educational purposes as part of the SECoT (Security Evaluation and Compromise Toolkit) project.
