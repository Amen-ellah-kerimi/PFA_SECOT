use anyhow::{anyhow, Result};
use serialport::{SerialPort, SerialPortInfo, SerialPortType};
use std::io::{Read, Write};
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::sync::mpsc;

// Global static for response handling
static mut LAST_RESPONSE_TX: Option<tokio::sync::oneshot::Sender<String>> = None;

const DEFAULT_BAUD_RATE: u32 = 115200;
const DEFAULT_TIMEOUT: Duration = Duration::from_millis(1000);
const SECOT_IDENTIFIER: &str = "SECoT";

pub struct SerialConnection {
    port: Option<Arc<Mutex<Box<dyn SerialPort>>>>,
    port_name: String,
    baud_rate: u32,
    connected: bool,
    rx_sender: mpsc::Sender<String>,  // For sending commands to the device
    tx_sender: mpsc::Sender<String>,  // For sending responses back to the application
}

impl SerialConnection {
    pub fn new() -> (Self, mpsc::Sender<String>, mpsc::Receiver<String>) {
        let (tx_to_serial, _rx_from_app) = mpsc::channel::<String>(100);
        let (tx_to_app, rx_from_serial) = mpsc::channel::<String>(100);

        (
            Self {
                port: None,
                port_name: String::new(),
                baud_rate: DEFAULT_BAUD_RATE,
                connected: false,
                rx_sender: tx_to_serial.clone(),
                tx_sender: tx_to_app.clone(),
            },
            tx_to_serial,
            rx_from_serial,
        )
    }

    pub async fn connect(&mut self, port_name: &str, baud_rate: u32) -> Result<()> {
        self.port_name = port_name.to_string();
        self.baud_rate = baud_rate;

        let port = serialport::new(port_name, baud_rate)
            .timeout(DEFAULT_TIMEOUT)
            .open()?;

        self.port = Some(Arc::new(Mutex::new(port)));
        self.connected = true;

        // Start the read and write tasks
        self.start_read_task();
        self.start_write_task();

        // Send a ping to verify it's a SECoT device
        self.send_command("ping").await?;

        Ok(())
    }

    pub async fn auto_connect(&mut self) -> Result<String> {
        let available_ports = serialport::available_ports()?;

        if available_ports.is_empty() {
            return Err(anyhow!("No serial ports found"));
        }

        for port_info in available_ports {
            match &port_info.port_type {
                SerialPortType::UsbPort(_) => {
                    // Try to connect to this port
                    if let Ok(()) = self.connect(&port_info.port_name, DEFAULT_BAUD_RATE).await {
                        // Wait a moment for the device to respond
                        tokio::time::sleep(Duration::from_millis(500)).await;

                        // Send a ping and check if it's a SECoT device
                        if let Ok(response) = self.send_command("ping").await {
                            if response.contains(SECOT_IDENTIFIER) {
                                return Ok(port_info.port_name);
                            }
                        }

                        // Not a SECoT device, disconnect
                        self.disconnect();
                    }
                }
                _ => continue,
            }
        }

        Err(anyhow!("No SECoT device found"))
    }

    pub fn disconnect(&mut self) {
        self.port = None;
        self.connected = false;
    }

    pub fn is_connected(&self) -> bool {
        self.connected
    }

    pub fn get_port_name(&self) -> &str {
        &self.port_name
    }

    pub async fn send_command(&self, command: &str) -> Result<String> {
        if !self.connected {
            return Err(anyhow!("Not connected to a serial port"));
        }

        // Create a one-shot channel for the response
        let (response_tx, response_rx) = tokio::sync::oneshot::channel::<String>();

        // Store the response channel in a static map
        // This is a simplified approach - in a real implementation, you would use a proper
        // request/response tracking mechanism
        unsafe {
            LAST_RESPONSE_TX = Some(response_tx);
        }

        // Send the command
        let command_with_newline = format!("{}\n", command);
        self.rx_sender.send(command_with_newline).await?;

        // Wait for response with timeout
        match tokio::time::timeout(Duration::from_secs(5), response_rx).await {
            Ok(Ok(response)) => Ok(response),
            Ok(Err(_)) => Err(anyhow!("Channel closed")),
            Err(_) => Err(anyhow!("Timeout waiting for response")),
        }
    }

    fn start_read_task(&self) {
        let port_clone = self.port.as_ref().unwrap().clone();
        let tx_sender = self.tx_sender.clone();

        // Spawn a thread for serial reading to avoid Send issues with MutexGuard
        std::thread::spawn(move || {
            let mut buffer = [0u8; 1024];
            let mut response = String::new();

            // Create a runtime for async operations within this thread
            let rt = tokio::runtime::Builder::new_current_thread()
                .enable_all()
                .build()
                .unwrap();

            loop {
                // Use a separate block to limit the lifetime of the MutexGuard
                {
                    let mut port_guard = match port_clone.lock() {
                        Ok(guard) => guard,
                        Err(_) => {
                            eprintln!("Failed to acquire lock on serial port");
                            std::thread::sleep(Duration::from_millis(100));
                            continue;
                        }
                    };

                    match port_guard.read(&mut buffer) {
                        Ok(bytes_read) if bytes_read > 0 => {
                            if let Ok(data) = String::from_utf8(buffer[..bytes_read].to_vec()) {
                                response.push_str(&data);
                            }
                        }
                        Err(e) => {
                            eprintln!("Error reading from serial port: {}", e);
                            break;
                        }
                        _ => {}
                    }
                } // MutexGuard is dropped here

                // Check if we have a complete response (ending with newline)
                if response.ends_with('\n') {
                    // Send the response to the channel
                    let response_clone = response.clone();
                    rt.block_on(async {
                        if let Err(e) = tx_sender.send(response_clone).await {
                            eprintln!("Error sending response: {}", e);
                        }
                    });

                    // Also check if there's a waiting oneshot channel
                    unsafe {
                        if let Some(tx) = LAST_RESPONSE_TX.take() {
                            let _ = tx.send(response.clone());
                        }
                    }

                    response.clear();
                }

                std::thread::sleep(Duration::from_millis(10));
            }
        });
    }

    fn start_write_task(&self) {
        let port_clone = self.port.as_ref().unwrap().clone();

        // Create a channel for receiving commands to send to the device
        let (tx, mut rx) = mpsc::channel::<String>(100);

        // Store the sender in a thread-local static
        thread_local! {
            static COMMAND_SENDER: std::cell::RefCell<Option<mpsc::Sender<String>>> = std::cell::RefCell::new(None);
        }

        COMMAND_SENDER.with(|cell| {
            *cell.borrow_mut() = Some(tx);
        });

        // Spawn a thread for serial writing to avoid Send issues with MutexGuard
        std::thread::spawn(move || {
            // Create a runtime for async operations within this thread
            let rt = tokio::runtime::Builder::new_current_thread()
                .enable_all()
                .build()
                .unwrap();

            rt.block_on(async {
                while let Some(data) = rx.recv().await {
                    // Use a separate block to limit the lifetime of the MutexGuard
                    {
                        let mut port_guard = match port_clone.lock() {
                            Ok(guard) => guard,
                            Err(_) => {
                                eprintln!("Failed to acquire lock on serial port");
                                continue;
                            }
                        };

                        if let Err(e) = port_guard.write_all(data.as_bytes()) {
                            eprintln!("Error writing to serial port: {}", e);
                        }
                    } // MutexGuard is dropped here
                }
            });
        });
    }

    pub fn list_available_ports() -> Result<Vec<SerialPortInfo>> {
        Ok(serialport::available_ports()?)
    }
}
