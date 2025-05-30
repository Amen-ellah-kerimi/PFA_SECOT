use tokio::fs::OpenOptions;
use tokio::io::{AsyncWriteExt, AsyncBufReadExt, BufReader};
use chrono::Local;
use serde_json;
use crate::data::models::{AttackEvent, CapturedCredential};
use colored::Colorize;
use std::path::Path;

pub async fn log_attack_event(attack_type: &str, status: &str, message: &str) {
    let timestamp = Local::now().to_rfc3339();
    let event = AttackEvent {
        timestamp,
        attack_type: attack_type.to_string(),
        status: status.to_string(),
        message: message.to_string(),
    };

    let log_entry = match serde_json::to_string(&event) {
        Ok(json) => json,
        Err(e) => {
            eprintln!("{}", format!("Error serializing log event: {}", e).red());
            return;
        }
    };

    let log_file_path = format!("logs/{}_attack_log.jsonl", Local::now().format("%Y%m%d"));

    let mut file = match OpenOptions::new()
        .create(true)
        .append(true)
        .open(&log_file_path)
        .await
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("{}", format!("Error opening log file {}: {}", log_file_path, e).red());
            return;
        }
    };

    if let Err(e) = file.write_all(format!("{}\n", log_entry).as_bytes()).await {
        eprintln!("{}", format!("Error writing to log file: {}", e).red());
    }
}

pub async fn read_credentials_from_logs(log_dir_path: &str) -> Result<Vec<CapturedCredential>, Box<dyn std::error::Error>> {
    let mut credentials = Vec::new();
    let path = Path::new(log_dir_path);

    if !path.is_dir() {
        return Err(format!("Log directory not found: {}", log_dir_path).into());
    }

    let mut entries = tokio::fs::read_dir(path).await?;
    while let Some(entry) = entries.next_entry().await? {
        let entry_path = entry.path();
        if entry_path.is_file() && entry_path.extension().map_or(false, |ext| ext == "jsonl") {
            let file = OpenOptions::new().read(true).open(&entry_path).await?;
            let mut reader = BufReader::new(file).lines();

            while let Some(line) = reader.next_line().await? {
                if let Ok(event) = serde_json::from_str::<AttackEvent>(&line) {
                    if event.status == "credential_found" && event.attack_type.contains("brute_force") {
                        if let Ok(cred_json) = serde_json::from_str::<CapturedCredential>(&event.message) {
                             credentials.push(cred_json);
                        } else if let Some(message_parts) = event.message.strip_prefix("Found: ") {
                             let parts: Vec<&str> = message_parts.split(" for MQTT broker ").collect();
                             if parts.len() == 2 {
                                 let cred_parts: Vec<&str> = parts[0].split(":").collect();
                                 if cred_parts.len() == 2 {
                                     let target_parts: Vec<&str> = parts[1].split(":").collect();
                                     if target_parts.len() == 2 {
                                         credentials.push(CapturedCredential {
                                             service: "MQTT".to_string(),
                                             username: cred_parts[0].to_string(),
                                             password: cred_parts[1].to_string(),
                                             timestamp: event.timestamp.clone(),
                                             source_attack: event.attack_type.clone(),
                                             target_ip: Some(target_parts[0].to_string()),
                                         });
                                     }
                                 }
                             }
                        }
                    }
                }
            }
        }
    }
    Ok(credentials)
}
