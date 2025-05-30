mod command;
mod config;
mod error;
mod models;
mod mqtt;
mod output;
mod serial;

use anyhow::Result;
use command::cmd_handler::handle_command;
use config::Config;
use mqtt::broker::{start_broker, stop_broker};
use output::formatter::{print_info, print_success, print_error, print_section};
use serial::serial_connection::SerialConnection;
use serial::serial_commands::SerialCommands;
use std::io::{self, Write};
use std::sync::Arc;
use tokio::runtime::Runtime;
use tokio::sync::Mutex;

fn main() -> Result<()> {
    // Load configuration
    let config = match Config::load("config.json") {
        Ok(config) => config,
        Err(_) => {
            print_info("No configuration found. Using default settings.");
            Config::default()
        }
    };

    // Initialize runtime
    let runtime = Runtime::new()?;

    // Start MQTT broker
    let broker_process = start_broker();

    // Print welcome message
    print_section("SECoT CLI Tool");
    print_info("Secure Command Tool for IoT Security Testing");
    println!("Type 'help' for available commands\n");

    // Initialize serial connection
    let (serial_connection, _tx, _rx) = SerialConnection::new();
    let serial_connection = Arc::new(Mutex::new(serial_connection));
    let serial_commands = Arc::new(SerialCommands::new(serial_connection.clone()));

    // Try to auto-connect to SECoT device if enabled in config
    if config.serial.auto_connect {
        print_info("Attempting to auto-connect to SECoT device...");
        match runtime.block_on(serial_commands.auto_connect()) {
            Ok(port) => print_success(&format!("Connected to SECoT device on port {}", port)),
            Err(_) => print_info("No SECoT device found. Use 'serial connect <port>' to connect manually."),
        }
    }

    // Set initial output format from config
    let mut output_format = config.output.default_format.clone();
    print_info(&format!("Output format set to '{}'", output_format));

    // Main command loop
    loop {
        print!("\nSECoT> ");
        io::stdout().flush()?;

        let mut input = String::new();
        io::stdin().read_line(&mut input)?;
        let trimmed = input.trim();

        if trimmed == "exit" {
            print_info("Exiting SECoT CLI Tool...");
            break;
        } else if trimmed.starts_with("set output ") {
            let parts: Vec<&str> = trimmed.split_whitespace().collect();
            if let Some(fmt) = parts.get(2) {
                if fmt == &"table" || fmt == &"json" {
                    output_format = fmt.to_string();
                    print_success(&format!("Output format set to '{}'", output_format));
                } else {
                    print_error("Invalid format. Use 'set output <table|json>'");
                }
            } else {
                print_error("Invalid format. Use 'set output <table|json>'");
            }
        } else {
            match runtime.block_on(handle_command(trimmed, &output_format, &serial_commands)) {
                Ok(_) => {},
                Err(e) => print_error(&format!("Error: {}", e)),
            }
        }
    }

    // Clean up
    stop_broker(broker_process);
    print_success("Goodbye!");
    Ok(())
}