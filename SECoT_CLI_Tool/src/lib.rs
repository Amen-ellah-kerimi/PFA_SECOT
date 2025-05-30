pub mod command;
pub mod config;
pub mod error;
pub mod models;
pub mod mqtt;
pub mod output;
pub mod serial;

// Re-export commonly used types
pub use config::Config;
pub use error::{CliError, CliResult};
pub use models::network::{DeviceInfo, MqttBroker, WiFiNetwork};
pub use models::port::{PortScanResults, PortStatus};
pub use output::formatter::{format_output, print_success, print_error, print_info, print_section};
pub use serial::serial_commands::SerialCommands;
