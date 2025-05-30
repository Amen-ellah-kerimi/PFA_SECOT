use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;
use anyhow::{Result, Context};

#[derive(Debug, Serialize, Deserialize)]
pub struct Config {
    pub mqtt: MqttConfig,
    pub serial: SerialConfig,
    pub output: OutputConfig,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct MqttConfig {
    pub broker_host: String,
    pub broker_port: u16,
    pub client_id: String,
    pub username: Option<String>,
    pub password: Option<String>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct SerialConfig {
    pub baud_rate: u32,
    pub auto_connect: bool,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct OutputConfig {
    pub default_format: String,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            mqtt: MqttConfig {
                broker_host: "localhost".to_string(),
                broker_port: 1883,
                client_id: "secot_cli_tool".to_string(),
                username: None,
                password: None,
            },
            serial: SerialConfig {
                baud_rate: 115200,
                auto_connect: true,
            },
            output: OutputConfig {
                default_format: "table".to_string(),
            },
        }
    }
}

impl Config {
    pub fn load(path: &str) -> Result<Self> {
        let config_path = Path::new(path);
        
        if !config_path.exists() {
            let default_config = Config::default();
            default_config.save(path)?;
            return Ok(default_config);
        }
        
        let config_str = fs::read_to_string(config_path)
            .with_context(|| format!("Failed to read config file: {}", path))?;
            
        let config: Config = serde_json::from_str(&config_str)
            .with_context(|| format!("Failed to parse config file: {}", path))?;
            
        Ok(config)
    }
    
    pub fn save(&self, path: &str) -> Result<()> {
        let config_str = serde_json::to_string_pretty(self)
            .with_context(|| "Failed to serialize config")?;
            
        fs::write(path, config_str)
            .with_context(|| format!("Failed to write config file: {}", path))?;
            
        Ok(())
    }
}
