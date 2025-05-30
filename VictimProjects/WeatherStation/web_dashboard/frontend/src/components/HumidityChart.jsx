import { useEffect, useRef } from 'react';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler,
} from 'chart.js';

// Register ChartJS components
ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

function HumidityChart({ data }) {
  const chartRef = useRef(null);
  
  // Format time for display
  const formatTime = (date) => {
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  };
  
  // Prepare chart data
  const chartData = {
    labels: data.map(item => formatTime(item.timestamp)),
    datasets: [
      {
        label: 'Humidity (%)',
        data: data.map(item => item.value),
        borderColor: '#2ecc71',
        backgroundColor: 'rgba(46, 204, 113, 0.2)',
        borderWidth: 2,
        tension: 0.4,
        fill: true,
        pointRadius: data.length > 20 ? 0 : 3,
        pointHoverRadius: 5,
      },
    ],
  };
  
  // Chart options
  const options = {
    responsive: true,
    maintainAspectRatio: false,
    animation: {
      duration: 500,
    },
    scales: {
      y: {
        beginAtZero: false,
        min: 0,
        max: 100,
        title: {
          display: true,
          text: 'Humidity (%)',
        },
        ticks: {
          callback: (value) => `${value}%`,
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
        display: false,
      },
      tooltip: {
        callbacks: {
          label: (context) => `Humidity: ${context.parsed.y.toFixed(1)}%`,
        },
      },
    },
  };
  
  // If no data, show a message
  if (data.length === 0) {
    return (
      <div style={{ 
        height: '100%', 
        display: 'flex', 
        alignItems: 'center', 
        justifyContent: 'center',
        color: 'var(--text-secondary)',
      }}>
        No humidity data available
      </div>
    );
  }
  
  return <Line ref={chartRef} data={chartData} options={options} />;
}

export default HumidityChart;
