// Global variables
let websocket;
let selectedAttackType = null;
let attackRunning = false;
let currentPage = 'dashboard';

// Attack type constants
const ATTACK_TYPE_DEAUTH = 0;
const ATTACK_TYPE_BEACON_FLOOD = 1;
const ATTACK_TYPE_PROBE_SPAM = 2;
const ATTACK_TYPE_ARP_SPOOF = 3;
const ATTACK_TYPE_MQTT_SPOOF = 4;
const ATTACK_TYPE_EVIL_TWIN = 5;
const ATTACK_TYPE_PASSIVE_SNIFF = 6;

// Network type constants
const NETWORK_TYPE_WIFI = 0;
const NETWORK_TYPE_MQTT = 1;
const NETWORK_TYPE_BLE = 2;

// DOM elements
const navLinks = document.querySelectorAll('nav a');
const pages = document.querySelectorAll('.page');
const attackTypeSelect = document.getElementById('attack-type');
const attackStartBtn = document.getElementById('attack-start');
const attackStopBtn = document.getElementById('attack-stop');
const attackParamsDiv = document.getElementById('attack-params');
const scanTypeSelect = document.getElementById('scan-type');
const scanStartBtn = document.getElementById('scan-start');
const scanResultsContainer = document.getElementById('scan-results-container');
const saveSettingsBtn = document.getElementById('save-settings');

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    initNavigation();
    initWebSocket();
    initAttackControls();
    initScanControls();
    initSettingsControls();
});

// Initialize navigation
function initNavigation() {
    navLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const targetPage = link.getAttribute('data-page');
            
            // Update active states
            navLinks.forEach(l => l.classList.remove('active'));
            link.classList.add('active');
            
            pages.forEach(p => p.classList.remove('active'));
            document.getElementById(targetPage).classList.add('active');
            
            currentPage = targetPage;
        });
    });
}

// Initialize WebSocket connection
function initWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws`;
    
    websocket = new WebSocket(wsUrl);
    
    websocket.onopen = () => {
        console.log('WebSocket connection established');
    };
    
    websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        updateDashboard(data);
    };
    
    websocket.onclose = () => {
        console.log('WebSocket connection closed');
        // Try to reconnect after 5 seconds
        setTimeout(initWebSocket, 5000);
    };
    
    websocket.onerror = (error) => {
        console.error('WebSocket error:', error);
    };
}

// Initialize attack controls
function initAttackControls() {
    // Fetch available attacks
    fetch('/api/attacks')
        .then(response => response.json())
        .then(data => {
            populateAttackSelect(data.attacks);
        })
        .catch(error => {
            console.error('Error fetching attacks:', error);
        });
    
    // Attack type selection
    attackTypeSelect.addEventListener('change', () => {
        selectedAttackType = parseInt(attackTypeSelect.value);
        
        if (selectedAttackType !== null) {
            fetchAttackParams(selectedAttackType);
        } else {
            attackParamsDiv.innerHTML = '<p>Select an attack to configure parameters</p>';
        }
    });
    
    // Start attack button
    attackStartBtn.addEventListener('click', () => {
        if (selectedAttackType !== null) {
            startAttack(selectedAttackType);
        }
    });
    
    // Stop attack button
    attackStopBtn.addEventListener('click', () => {
        if (selectedAttackType !== null) {
            stopAttack(selectedAttackType);
        }
    });
}

// Initialize scan controls
function initScanControls() {
    // Start scan button
    scanStartBtn.addEventListener('click', () => {
        const scanType = parseInt(scanTypeSelect.value);
        startScan(scanType);
    });
}

// Initialize settings controls
function initSettingsControls() {
    // Save settings button
    saveSettingsBtn.addEventListener('click', () => {
        const settings = {
            ap_ssid: document.getElementById('ap-ssid').value,
            ap_password: document.getElementById('ap-password').value,
            ap_channel: parseInt(document.getElementById('ap-channel').value)
        };
        
        saveSettings(settings);
    });
}

// Populate attack select dropdown
function populateAttackSelect(attacks) {
    attackTypeSelect.innerHTML = '<option value="" disabled selected>Select an attack</option>';
    
    attacks.forEach(attack => {
        const option = document.createElement('option');
        option.value = attack.type;
        option.textContent = attack.name;
        attackTypeSelect.appendChild(option);
    });
}

// Fetch attack parameters
function fetchAttackParams(attackType) {
    fetch(`/api/attack/params?type=${attackType}`)
        .then(response => response.json())
        .then(data => {
            displayAttackParams(data);
        })
        .catch(error => {
            console.error('Error fetching attack parameters:', error);
        });
}

// Display attack parameters
function displayAttackParams(data) {
    attackParamsDiv.innerHTML = '';
    
    const heading = document.createElement('h3');
    heading.textContent = data.name + ' Parameters';
    attackParamsDiv.appendChild(heading);
    
    // Create form for parameters
    const form = document.createElement('form');
    form.id = 'attack-params-form';
    form.addEventListener('submit', (e) => {
        e.preventDefault();
    });
    
    // Add parameters
    for (const [key, value] of Object.entries(data.params)) {
        const paramItem = document.createElement('div');
        paramItem.className = 'param-item';
        
        const label = document.createElement('label');
        label.textContent = key;
        label.setAttribute('for', `param-${key}`);
        
        const input = document.createElement('input');
        input.type = typeof value === 'boolean' ? 'checkbox' : 'text';
        input.id = `param-${key}`;
        input.name = key;
        
        if (typeof value === 'boolean') {
            input.checked = value;
        } else {
            input.value = value;
        }
        
        input.addEventListener('change', () => {
            const paramValue = input.type === 'checkbox' ? input.checked : input.value;
            setAttackParameter(selectedAttackType, key, paramValue);
        });
        
        paramItem.appendChild(label);
        paramItem.appendChild(input);
        form.appendChild(paramItem);
    }
    
    attackParamsDiv.appendChild(form);
}

// Set attack parameter
function setAttackParameter(attackType, paramName, paramValue) {
    fetch('/api/attack/setparam', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `type=${attackType}&name=${paramName}&value=${paramValue}`
    })
    .then(response => response.json())
    .then(data => {
        if (!data.success) {
            console.error('Error setting parameter:', data.error);
        }
    })
    .catch(error => {
        console.error('Error setting parameter:', error);
    });
}

// Start attack
function startAttack(attackType) {
    fetch('/api/attack/start', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `type=${attackType}`
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            attackRunning = true;
            attackStartBtn.disabled = true;
            attackStopBtn.disabled = false;
        } else {
            console.error('Error starting attack:', data.error);
        }
    })
    .catch(error => {
        console.error('Error starting attack:', error);
    });
}

// Stop attack
function stopAttack(attackType) {
    fetch('/api/attack/stop', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `type=${attackType}`
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            attackRunning = false;
            attackStartBtn.disabled = false;
            attackStopBtn.disabled = true;
        } else {
            console.error('Error stopping attack:', data.error);
        }
    })
    .catch(error => {
        console.error('Error stopping attack:', error);
    });
}

// Start scan
function startScan(scanType) {
    scanResultsContainer.innerHTML = '<p>Scanning...</p>';
    
    fetch('/api/scan', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `type=${scanType}`
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            // Wait a bit for scan to complete
            setTimeout(() => {
                fetchScanResults(scanType);
            }, 5000);
        } else {
            scanResultsContainer.innerHTML = '<p>Scan failed</p>';
            console.error('Error starting scan:', data.error);
        }
    })
    .catch(error => {
        scanResultsContainer.innerHTML = '<p>Scan failed</p>';
        console.error('Error starting scan:', error);
    });
}

// Fetch scan results
function fetchScanResults(scanType) {
    fetch(`/api/scan/results?type=${scanType}`)
        .then(response => response.json())
        .then(data => {
            displayScanResults(scanType, data);
        })
        .catch(error => {
            console.error('Error fetching scan results:', error);
        });
}

// Display scan results
function displayScanResults(scanType, data) {
    scanResultsContainer.innerHTML = '';
    
    if (scanType === NETWORK_TYPE_WIFI && data.wifi_networks) {
        if (data.wifi_networks.length === 0) {
            scanResultsContainer.innerHTML = '<p>No WiFi networks found</p>';
            return;
        }
        
        data.wifi_networks.forEach(network => {
            const networkItem = document.createElement('div');
            networkItem.className = 'network-item';
            
            const heading = document.createElement('h4');
            heading.textContent = network.ssid || '(Hidden Network)';
            
            const details = document.createElement('div');
            details.className = 'network-details';
            
            // BSSID
            const bssidDetail = document.createElement('div');
            bssidDetail.className = 'network-detail';
            bssidDetail.innerHTML = `<span>BSSID</span><span>${network.bssid}</span>`;
            
            // Channel
            const channelDetail = document.createElement('div');
            channelDetail.className = 'network-detail';
            channelDetail.innerHTML = `<span>Channel</span><span>${network.channel}</span>`;
            
            // RSSI
            const rssiDetail = document.createElement('div');
            rssiDetail.className = 'network-detail';
            rssiDetail.innerHTML = `<span>Signal</span><span>${network.rssi} dBm</span>`;
            
            // Encryption
            const encryptionDetail = document.createElement('div');
            encryptionDetail.className = 'network-detail';
            encryptionDetail.innerHTML = `<span>Security</span><span>${network.encryption}</span>`;
            
            details.appendChild(bssidDetail);
            details.appendChild(channelDetail);
            details.appendChild(rssiDetail);
            details.appendChild(encryptionDetail);
            
            networkItem.appendChild(heading);
            networkItem.appendChild(details);
            
            scanResultsContainer.appendChild(networkItem);
        });
    } else if (scanType === NETWORK_TYPE_MQTT && data.mqtt_brokers) {
        if (data.mqtt_brokers.length === 0) {
            scanResultsContainer.innerHTML = '<p>No MQTT brokers found</p>';
            return;
        }
        
        data.mqtt_brokers.forEach(broker => {
            const brokerItem = document.createElement('div');
            brokerItem.className = 'network-item';
            
            const heading = document.createElement('h4');
            heading.textContent = `MQTT Broker: ${broker.ip}:${broker.port}`;
            
            const details = document.createElement('div');
            details.className = 'network-details';
            
            // Authentication
            const authDetail = document.createElement('div');
            authDetail.className = 'network-detail';
            authDetail.innerHTML = `<span>Auth Required</span><span>${broker.requires_auth ? 'Yes' : 'No'}</span>`;
            
            // TLS
            const tlsDetail = document.createElement('div');
            tlsDetail.className = 'network-detail';
            tlsDetail.innerHTML = `<span>TLS Support</span><span>${broker.supports_tls ? 'Yes' : 'No'}</span>`;
            
            // Accessible
            const accessDetail = document.createElement('div');
            accessDetail.className = 'network-detail';
            accessDetail.innerHTML = `<span>Accessible</span><span>${broker.accessible ? 'Yes' : 'No'}</span>`;
            
            details.appendChild(authDetail);
            details.appendChild(tlsDetail);
            details.appendChild(accessDetail);
            
            brokerItem.appendChild(heading);
            brokerItem.appendChild(details);
            
            scanResultsContainer.appendChild(brokerItem);
        });
    } else if (scanType === NETWORK_TYPE_BLE && data.ble_devices) {
        if (data.ble_devices.length === 0) {
            scanResultsContainer.innerHTML = '<p>No BLE devices found</p>';
            return;
        }
        
        data.ble_devices.forEach(device => {
            const deviceItem = document.createElement('div');
            deviceItem.className = 'network-item';
            
            const heading = document.createElement('h4');
            heading.textContent = device.name || '(Unnamed Device)';
            
            const details = document.createElement('div');
            details.className = 'network-details';
            
            // Address
            const addressDetail = document.createElement('div');
            addressDetail.className = 'network-detail';
            addressDetail.innerHTML = `<span>Address</span><span>${device.address}</span>`;
            
            // RSSI
            const rssiDetail = document.createElement('div');
            rssiDetail.className = 'network-detail';
            rssiDetail.innerHTML = `<span>Signal</span><span>${device.rssi} dBm</span>`;
            
            // Connectable
            const connectableDetail = document.createElement('div');
            connectableDetail.className = 'network-detail';
            connectableDetail.innerHTML = `<span>Connectable</span><span>${device.connectable ? 'Yes' : 'No'}</span>`;
            
            details.appendChild(addressDetail);
            details.appendChild(rssiDetail);
            details.appendChild(connectableDetail);
            
            deviceItem.appendChild(heading);
            deviceItem.appendChild(details);
            
            scanResultsContainer.appendChild(deviceItem);
        });
    } else {
        scanResultsContainer.innerHTML = '<p>No results available</p>';
    }
}

// Save settings
function saveSettings(settings) {
    // This would normally send settings to the server
    // For now, just show an alert
    alert('Settings saved (not actually implemented yet)');
}

// Update dashboard with status data
function updateDashboard(data) {
    // Update system status
    document.getElementById('uptime').textContent = formatUptime(data.system.uptime);
    document.getElementById('free-memory').textContent = formatBytes(data.system.free_heap);
    
    // Update WiFi status
    document.getElementById('wifi-mode').textContent = data.wifi.ap_mode ? 'Access Point' : 'Station';
    document.getElementById('wifi-ssid').textContent = data.wifi.ap_mode ? data.wifi.ap_ssid : data.wifi.sta_ssid;
    document.getElementById('wifi-ip').textContent = data.wifi.ap_mode ? data.wifi.ap_ip : data.wifi.sta_ip;
    
    // Update scan results
    document.getElementById('wifi-networks').textContent = data.scan.wifi_networks;
    document.getElementById('mqtt-brokers').textContent = data.scan.mqtt_brokers;
    document.getElementById('ble-devices').textContent = data.scan.ble_devices || 0;
    
    // Update active attacks
    const activeAttacksDiv = document.getElementById('active-attacks');
    activeAttacksDiv.innerHTML = '';
    
    let hasActiveAttacks = false;
    
    data.attacks.forEach(attack => {
        if (attack.status === 1) { // ATTACK_STATUS_RUNNING
            hasActiveAttacks = true;
            
            const attackItem = document.createElement('div');
            attackItem.className = 'status-item';
            attackItem.innerHTML = `<span>${attack.name}</span><span class="status-running">Running</span>`;
            
            activeAttacksDiv.appendChild(attackItem);
        }
    });
    
    if (!hasActiveAttacks) {
        activeAttacksDiv.innerHTML = '<p>No active attacks</p>';
    }
    
    // Update attack controls if we're on the attacks page
    if (currentPage === 'attacks' && selectedAttackType !== null) {
        const attack = data.attacks.find(a => a.type === selectedAttackType);
        if (attack) {
            attackRunning = (attack.status === 1); // ATTACK_STATUS_RUNNING
            attackStartBtn.disabled = attackRunning;
            attackStopBtn.disabled = !attackRunning;
        }
    }
}

// Format uptime in a human-readable way
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m ${secs}s`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

// Format bytes in a human-readable way
function formatBytes(bytes) {
    if (bytes < 1024) {
        return bytes + ' bytes';
    } else if (bytes < 1048576) {
        return (bytes / 1024).toFixed(2) + ' KB';
    } else {
        return (bytes / 1048576).toFixed(2) + ' MB';
    }
}
