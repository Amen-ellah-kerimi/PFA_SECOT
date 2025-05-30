use crate::data::models::PacketInfo;
use tokio::process::Command;
use tokio::io::{AsyncBufReadExt, BufReader};
use colored::Colorize;
use serde_json::Value;

pub async fn parse_pcap_for_mqtt_traffic(pcap_path: &str) -> Result<Vec<PacketInfo>, Box<dyn std::error::Error>> {
    println!("{}", format!("• Analyzing MQTT traffic in '{}' using tshark...", pcap_path).blue());
    let mut packets: Vec<PacketInfo> = Vec::new();

    let mut tshark_command = Command::new("tshark");
    tshark_command
        .arg("-r").arg(pcap_path)
        .arg("-Y").arg("mqtt")
        .arg("-T").arg("json");

    let mut child = tshark_command.stdout(std::process::Stdio::piped()).spawn()?;
    let stdout = child.stdout.take().expect("Failed to open stdout");
    let mut reader = BufReader::new(stdout).lines();

    while let Some(line) = reader.next_line().await? {
        if let Ok(json_val) = serde_json::from_str::<Value>(&line) {
            if let Some(layers) = json_val.get("_source").ok_or("Missing _source")?.get("layers") {
                let frame = layers.get("frame").ok_or("Missing frame")?;
                let ip_layer = layers.get("ip");
                let mqtt_layer = layers.get("mqtt");

                let timestamp = frame.get("frame.time").ok_or("Missing frame.time")?.as_str().ok_or("frame.time not a string")?.to_string();
                let source_ip = ip_layer.and_then(|ip| ip.get("ip.src")?.as_str()).unwrap_or("N/A").to_string();
                let dest_ip = ip_layer.and_then(|ip| ip.get("ip.dst")?.as_str()).unwrap_or("N/A").to_string();
                let size = frame.get("frame.len").ok_or("Missing frame.len")?.as_str().ok_or("frame.len not a string")?.parse::<u32>().unwrap_or(0);
                let protocol = "MQTT".to_string();

                let mut summary = String::new();
                if let Some(mqtt) = mqtt_layer {
                    if let Some(msg_type) = mqtt.get("mqtt.msgtype").and_then(|v| v.as_str()) {
                        summary.push_str(&format!("Type: {}", msg_type));
                    }
                    if let Some(topic) = mqtt.get("mqtt.topic").and_then(|v| v.as_str()) {
                        summary.push_str(&format!(", Topic: {}", topic));
                    }
                    if let Some(payload) = mqtt.get("mqtt.payload").and_then(|v| v.as_str()) {
                        summary.push_str(&format!(", Payload: {}", payload));
                    }
                }

                packets.push(PacketInfo {
                    timestamp,
                    source_ip,
                    dest_ip,
                    protocol,
                    size,
                    summary,
                });
            }
        }
    }

    let status = child.wait().await?;
    if !status.success() {
        eprintln!("{}", format!("  tshark exited with error: {:?}", status).red());
    }
    println!("{}", format!("• Successfully parsed {} MQTT packets.", packets.len()).green());
    Ok(packets)
}
