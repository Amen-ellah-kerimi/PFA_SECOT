use thiserror::Error;

#[derive(Error, Debug)]
pub enum CliError {
    #[error("Command error: {0}")]
    CommandError(String),
    
    #[error("MQTT error: {0}")]
    MqttError(String),
    
    #[error("Serial error: {0}")]
    SerialError(String),
    
    #[error("Network error: {0}")]
    NetworkError(String),
    
    #[error("Configuration error: {0}")]
    ConfigError(String),
    
    #[error("I/O error: {0}")]
    IoError(#[from] std::io::Error),
    
    #[error("JSON error: {0}")]
    JsonError(#[from] serde_json::Error),
    
    #[error("Unknown error: {0}")]
    Unknown(String),
}

pub type CliResult<T> = Result<T, CliError>;
