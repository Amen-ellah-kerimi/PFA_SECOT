use anyhow::{anyhow, Result};
use super::scan_ports::run_port_scan;
use super::scan_networks::run_network_scan;
use super::broker_test::run_broker_test;
use crate::serial::serial_commands::SerialCommands;
use crate::output::formatter::{print_info, print_error, print_success, print_section};
use std::sync::Arc;

pub async fn handle_command(
    cmd: &str,
    output_format: &str,
    serial_commands: &Arc<SerialCommands>
) -> Result<()> {
    let parts: Vec<&str> = cmd.trim().split_whitespace().collect();

    match parts.as_slice() {
        ["help"] => {
            print_section("Available Commands");

            print_section("Network Commands");
            println!("  scan ports <ip>              - Scan ports on a device");
            println!("  scan network <cidr>          - Scan local network for devices");
            println!("  broker test <ip>             - Test MQTT broker accessibility");

            print_section("Serial Port Commands");
            println!("  serial list                  - List available serial ports");
            println!("  serial connect <port> [baud] - Connect to a serial port");
            println!("  serial disconnect            - Disconnect from serial port");
            println!("  serial status                - Show serial connection status");

            print_section("SECoT Commands");
            println!("  secot scan wifi              - Scan for WiFi networks using SECoT");
            println!("  secot scan mqtt              - Scan for MQTT brokers using SECoT");
            println!("  secot attack <type> [dur]    - Start an attack using SECoT");
            println!("  secot stop [type]            - Stop an attack using SECoT");
            println!("  secot status                 - Show status of SECoT");
            println!("  secot set <attack> <param> <value> - Set attack parameter");
            println!("  secot get <attack> <param>   - Get attack parameter");

            print_section("General Commands");
            println!("  set output <fmt>             - Set output format to table/json");
            println!("  exit                         - Exit the tool");
        },
        ["scan", "ports", ip] => {
            print_info(&format!("Scanning ports on {}...", ip));
            run_port_scan(ip, output_format).await?;
        },
        ["scan", "network", cidr] => {
            print_info(&format!("Scanning network {}...", cidr));
            run_network_scan(cidr, output_format).await?;
        },
        ["broker", "test", ip] => {
            print_info(&format!("Testing MQTT broker at {}...", ip));
            run_broker_test(ip, output_format).await?;
        },

        // Serial port commands
        ["serial", "list"] => {
            print_info("Listing available serial ports...");
            serial_commands.list_ports(output_format).await?;
        },
        ["serial", "connect", port] => {
            print_info(&format!("Connecting to {}...", port));
            match serial_commands.connect_to_port(port, 115200).await {
                Ok(_) => print_success(&format!("Connected to {}", port)),
                Err(e) => print_error(&format!("Failed to connect: {}", e)),
            }
        },
        ["serial", "connect", port, baud] => {
            let baud_rate = baud.parse::<u32>().map_err(|_| anyhow!("Invalid baud rate"))?;
            print_info(&format!("Connecting to {} at {} baud...", port, baud_rate));
            match serial_commands.connect_to_port(port, baud_rate).await {
                Ok(_) => print_success(&format!("Connected to {} at {} baud", port, baud_rate)),
                Err(e) => print_error(&format!("Failed to connect: {}", e)),
            }
        },
        ["serial", "disconnect"] => {
            print_info("Disconnecting from serial port...");
            match serial_commands.disconnect().await {
                Ok(_) => print_success("Disconnected from serial port"),
                Err(e) => print_error(&format!("Failed to disconnect: {}", e)),
            }
        },
        ["serial", "status"] => {
            if let Ok(true) = serial_commands.is_connected().await {
                if let Ok(port_name) = serial_commands.get_port_name().await {
                    print_success(&format!("Connected to {}", port_name));
                }
            } else {
                print_info("Not connected to any serial port");
            }
        },

        // SECoT commands via serial
        ["secot", "scan", "wifi"] => {
            print_info("Scanning for WiFi networks using SECoT...");
            serial_commands.scan_wifi(output_format).await?;
        },
        ["secot", "scan", "mqtt"] => {
            print_info("Scanning for MQTT brokers using SECoT...");
            serial_commands.scan_mqtt(output_format).await?;
        },
        ["secot", "attack", attack_type] => {
            print_info(&format!("Starting {} attack...", attack_type));
            serial_commands.start_attack(attack_type, None).await?;
        },
        ["secot", "attack", attack_type, duration] => {
            let dur = duration.parse::<u32>().map_err(|_| anyhow!("Invalid duration"))?;
            print_info(&format!("Starting {} attack for {} seconds...", attack_type, dur));
            serial_commands.start_attack(attack_type, Some(dur)).await?;
        },
        ["secot", "stop"] => {
            print_info("Stopping all attacks...");
            serial_commands.stop_attack(None).await?;
        },
        ["secot", "stop", attack_type] => {
            print_info(&format!("Stopping {} attack...", attack_type));
            serial_commands.stop_attack(Some(attack_type)).await?;
        },
        ["secot", "status"] => {
            print_info("Getting SECoT status...");
            serial_commands.get_status().await?;
        },
        ["secot", "set", attack, param, value] => {
            print_info(&format!("Setting {} parameter {} to {}...", attack, param, value));
            serial_commands.set_parameter(attack, param, value).await?;
        },
        ["secot", "get", attack, param] => {
            print_info(&format!("Getting {} parameter {}...", attack, param));
            serial_commands.get_parameter(attack, param).await?;
        },

        _ => {
            print_error("Unknown or malformed command");
            println!("Type 'help' for a list of available commands");
            return Err(anyhow!("Unknown or malformed command"));
        },
    }

    Ok(())
}
