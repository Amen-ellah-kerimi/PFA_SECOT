use crate::data::models::CapturedCredential;
use chrono::Local;

pub fn parse_hydra_mqtt_output(line: &str, broker: &str, port: u16) -> Option<CapturedCredential> {
    if line.contains("host:") && line.contains("login:") && line.contains("password:") {
        let parts: Vec<&str> = line.split_whitespace().collect();
        if let (Some(host_idx), Some(login_idx), Some(password_idx)) = (
            parts.iter().position(|&p| p == "host:"),
            parts.iter().position(|&p| p == "login:"),
            parts.iter().position(|&p| p == "password:"),
        ) {
            let host = parts.get(host_idx + 1)?.trim_end_matches(':');
            let login = parts.get(login_idx + 1)?;
            let password = parts.get(password_idx + 1)?;

            if host == broker && parts.get(host_idx + 2).unwrap_or(&"").parse::<u16>().ok() == Some(port) {
                return Some(CapturedCredential {
                    service: "MQTT".to_string(),
                    username: login.to_string(),
                    password: password.to_string(),
                    timestamp: Local::now().to_rfc3339(),
                    source_attack: "MQTT_BruteForce".to_string(),
                    target_ip: Some(broker.to_string()),
                });
            }
        }
    }
    None
}
