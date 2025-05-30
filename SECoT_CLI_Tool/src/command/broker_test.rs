use crate::models::network::MqttBroker;
use crate::mqtt::broker_utils::test_mqtt_broker;
use crate::output::formatter::{format_output, print_success, print_error};
use anyhow::Result;
use std::net::IpAddr;

pub async fn run_broker_test(ip_str: &str, output_format: &str) -> Result<()> {
    let ip: IpAddr = ip_str.parse()?;
    let port = 1883; // Default MQTT port

    println!("Testing MQTT broker at {}:{}...", ip, port);

    // Test direct connection to broker
    let is_accessible = test_mqtt_broker(&ip.to_string(), port).await?;

    let broker = MqttBroker {
        ip,
        port,
        requires_auth: false, // We don't know this yet
        supports_tls: false,  // We don't know this yet
        is_accessible,
    };

    if is_accessible {
        print_success(&format!("Successfully connected to MQTT broker at {}:{}", ip, port));
    } else {
        print_error(&format!("Failed to connect to MQTT broker at {}:{}", ip, port));
    }

    // Format and display the result
    let output = format_output(&broker, output_format)?;
    println!("{}", output);

    Ok(())
}