use anyhow::{anyhow, Result};
use crate::serial::serial_connection::SerialConnection;
use serde_json::{json, Value};
use std::sync::Arc;
use tokio::sync::Mutex;

pub struct SerialCommands {
    connection: Arc<Mutex<SerialConnection>>,
}

impl SerialCommands {
    pub fn new(connection: Arc<Mutex<SerialConnection>>) -> Self {
        Self { connection }
    }

    pub async fn connect_to_port(&self, port_name: &str, baud_rate: u32) -> Result<()> {
        let mut conn = self.connection.lock().await;
        conn.connect(port_name, baud_rate).await
    }

    pub async fn auto_connect(&self) -> Result<String> {
        let mut conn = self.connection.lock().await;
        conn.auto_connect().await
    }

    pub async fn disconnect(&self) -> Result<()> {
        let mut conn = self.connection.lock().await;
        conn.disconnect();
        Ok(())
    }

    pub async fn is_connected(&self) -> Result<bool> {
        let conn = self.connection.lock().await;
        Ok(conn.is_connected())
    }

    pub async fn get_port_name(&self) -> Result<String> {
        let conn = self.connection.lock().await;
        Ok(conn.get_port_name().to_string())
    }

    pub async fn list_ports(&self, output_format: &str) -> Result<()> {
        let ports = SerialConnection::list_available_ports()?;
        
        if ports.is_empty() {
            println!("No serial ports found");
            return Ok(());
        }

        if output_format == "json" {
            let mut port_list = Vec::new();
            for port in ports {
                let port_info = json!({
                    "name": port.port_name,
                    "type": format!("{:?}", port.port_type),
                });
                port_list.push(port_info);
            }
            println!("{}", serde_json::to_string_pretty(&port_list)?);
        } else {
            println!("Available serial ports:");
            for port in ports {
                println!("  {} - {:?}", port.port_name, port.port_type);
            }
        }

        Ok(())
    }

    pub async fn send_command(&self, command: &str) -> Result<String> {
        let conn = self.connection.lock().await;
        if !conn.is_connected() {
            return Err(anyhow!("Not connected to a serial port"));
        }
        conn.send_command(command).await
    }

    // SECoT specific commands
    pub async fn scan_wifi(&self, output_format: &str) -> Result<()> {
        let response = self.send_command("scan wifi").await?;
        
        if output_format == "json" {
            // Try to parse the response as JSON
            if let Ok(json_value) = serde_json::from_str::<Value>(&response) {
                println!("{}", serde_json::to_string_pretty(&json_value)?);
            } else {
                println!("{}", response);
            }
        } else {
            println!("{}", response);
        }
        
        Ok(())
    }

    pub async fn scan_mqtt(&self, output_format: &str) -> Result<()> {
        let response = self.send_command("scan mqtt").await?;
        
        if output_format == "json" {
            // Try to parse the response as JSON
            if let Ok(json_value) = serde_json::from_str::<Value>(&response) {
                println!("{}", serde_json::to_string_pretty(&json_value)?);
            } else {
                println!("{}", response);
            }
        } else {
            println!("{}", response);
        }
        
        Ok(())
    }

    pub async fn start_attack(&self, attack_type: &str, duration: Option<u32>) -> Result<()> {
        let command = match duration {
            Some(dur) => format!("attack {} {}", attack_type, dur),
            None => format!("attack {}", attack_type),
        };
        
        let response = self.send_command(&command).await?;
        println!("{}", response);
        
        Ok(())
    }

    pub async fn stop_attack(&self, attack_type: Option<&str>) -> Result<()> {
        let command = match attack_type {
            Some(attack) => format!("stop {}", attack),
            None => "stop".to_string(),
        };
        
        let response = self.send_command(&command).await?;
        println!("{}", response);
        
        Ok(())
    }

    pub async fn get_status(&self) -> Result<()> {
        let response = self.send_command("status").await?;
        println!("{}", response);
        
        Ok(())
    }

    pub async fn set_parameter(&self, attack: &str, param: &str, value: &str) -> Result<()> {
        let command = format!("set {} {} {}", attack, param, value);
        let response = self.send_command(&command).await?;
        println!("{}", response);
        
        Ok(())
    }

    pub async fn get_parameter(&self, attack: &str, param: &str) -> Result<()> {
        let command = format!("get {} {}", attack, param);
        let response = self.send_command(&command).await?;
        println!("{}", response);
        
        Ok(())
    }
}
