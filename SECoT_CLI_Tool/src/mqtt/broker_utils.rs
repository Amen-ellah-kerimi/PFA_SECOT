use anyhow::{anyhow, Result};
use rumqttc::{AsyncClient, Event, MqttOptions, Packet, QoS};
use serde_json::Value;
use std::time::Duration;
use tokio::time::timeout;
use uuid::Uuid;

pub async fn send_mqtt_request(topic: &str, payload: &Value) -> Result<Value> {
    let mut mqtt_options = MqttOptions::new("secot_cli_request", "localhost", 1883);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqtt_options, 10);
    let response_topic = format!("secot/response/{}", Uuid::new_v4());

    let mut payload = payload.clone();
    payload["response_topic"] = Value::String(response_topic.clone());

    client.subscribe(&response_topic, QoS::AtLeastOnce).await?;
    client.publish(topic, QoS::AtLeastOnce, false, serde_json::to_vec(&payload)?).await?;

    loop {
        match timeout(Duration::from_secs(5), eventloop.poll()).await {
            Ok(Ok(event)) => {
                if let Event::Incoming(Packet::Publish(p)) = event {
                    if p.topic == response_topic {
                        return Ok(serde_json::from_slice(&p.payload)?);
                    }
                }
            },
            Ok(Err(e)) => return Err(anyhow!("MQTT connection error: {}", e)),
            Err(_) => return Err(anyhow!("Timeout waiting for MQTT response")),
        }
    }
}

pub async fn scan_mqtt_brokers(_network: &str) -> Result<Vec<(String, u16)>> {
    // This is a simplified implementation
    // In a real implementation, you would scan the network for MQTT brokers

    // For now, just return a hardcoded list of common MQTT broker ports for localhost
    let brokers = vec![
        ("localhost".to_string(), 1883),
        ("localhost".to_string(), 8883),
    ];

    Ok(brokers)
}

pub async fn test_mqtt_broker(host: &str, port: u16) -> Result<bool> {
    let client_id = format!("secot_cli_test_{}", Uuid::new_v4());
    let mut mqtt_options = MqttOptions::new(client_id, host, port);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    // Create a new client
    let (_client, mut eventloop) = AsyncClient::new(mqtt_options, 10);

    // Try to receive the first event, which should be a connection event
    match timeout(Duration::from_secs(5), eventloop.poll()).await {
        Ok(Ok(_)) => {
            // Successfully connected
            Ok(true)
        },
        _ => Ok(false),
    }
}
