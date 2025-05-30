use tokio::io::{AsyncBufReadExt, BufReader};
use tokio::process::Command;
use colored::Colorize;
use std::collections::{HashMap};
use std::time::{Instant, Duration};
use tokio::time::sleep;
use chrono::Local;

use crate::data::models::{WifiNetwork, WifiClient};
use crate::ui::formatter;
use crate::parsers::airodump_parser;
use crate::data::logger;

pub async fn run_scan(interface: &str, duration_secs: u64) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Ensuring '{}' is in monitor mode...", interface).blue());
    let airmon_output = Command::new("sudo")
        .arg("airmon-ng")
        .arg("start")
        .arg(interface)
        .output()
        .await?;

    if !airmon_output.status.success() {
        eprintln!("{}", format!("  Warning: Failed to set '{}' to monitor mode. Airodump-ng might not work correctly.", interface).yellow());
        eprintln!("  Stderr: {}", String::from_utf8_lossy(&airmon_output.stderr).dimmed());
    } else {
        println!("{}", format!("  '{}' should now be in monitor mode.", interface).green());
    }

    println!("{}", format!("• Starting airodump-ng on '{}' for {} seconds...", interface, duration_secs).blue());
    println!("{}", "  Press Ctrl+C to stop scanning early.".dimmed());

    let mut networks: HashMap<String, WifiNetwork> = HashMap::new();
    let mut clients: HashMap<String, WifiClient> = HashMap::new();

    let temp_pcap_prefix = format!("/tmp/airodump_scan_{}", Local::now().format("%Y%m%d_%H%M%S"));
    let command_args = vec![
        "--output-format", "kismet,csv",
        "-w", &temp_pcap_prefix,
        "--update", "1",
        interface,
    ];

    let mut child = Command::new("sudo")
        .arg("airodump-ng")
        .args(&command_args)
        .stdout(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        .spawn()?;

    let stdout = child.stdout.take().expect("Failed to open stdout");
    let stderr = child.stderr.take().expect("Failed to open stderr");

    let mut reader_stdout = BufReader::new(stdout).lines();
    let mut reader_stderr = BufReader::new(stderr).lines();

    let start_time = Instant::now();

    tokio::spawn(async move {
        while let Some(line) = reader_stderr.next_line().await.unwrap() {
            eprintln!("{}", format!("[Airodump-ng Error]: {}", line).red().dimmed());
            logger::log_attack_event("wifi_scan", "error", &format!("Airodump-ng stderr: {}", line)).await;
        }
    });

    while let Some(line) = tokio::select! {
        line = reader_stdout.next_line() => line?,
        _ = sleep(Duration::from_secs(duration_secs)) => {
            println!("{}", "\n• Scan duration reached. Stopping airodump-ng...".blue());
            None
        }
    } {
        if line.starts_with("BSSID") && line.contains("CH") {
            continue;
        }

        if let Some(ap_data) = airodump_parser::parse_airodump_ap_line(&line) {
            networks.insert(ap_data.bssid.clone(), ap_data);
        } else if let Some(client_data) = airodump_parser::parse_airodump_client_line(&line) {
            clients.insert(client_data.mac.clone(), client_data.clone());
            if let Some(ap_net) = networks.get_mut(&client_data.bssid) {
                if !ap_net.clients.iter().any(|c| c.mac == client_data.mac) {
                    ap_net.clients.push(client_data);
                }
            }
        }

        if start_time.elapsed().as_millis() % 2000 < 500 {
            formatter::clear_terminal();
            formatter::display_scan_results(&networks, &clients, start_time.elapsed().as_secs());
        }
    }

    let _ = child.kill().await;
    let status = child.wait().await?;

    println!("{}", format!("\n• Airodump-ng finished with status: {}", status).blue());
    println!("{}", format!("• Captured data saved to files starting with: {}", temp_pcap_prefix.bold()));

    formatter::clear_terminal();
    formatter::display_scan_results(&networks, &clients, start_time.elapsed().as_secs());

    println!("{}", "\n--- Scan Summary ---".cyan().bold());
    println!("Total Unique APs Found: {}", networks.len().to_string().green());
    println!("Total Unique Clients Found: {}", clients.len().to_string().green());
    println!("Captured Data Location: {}", temp_pcap_prefix.bold().yellow());
    println!("{}", "--------------------".cyan().bold());

    logger::log_attack_event("wifi_scan", "completed", &format!("Found {} APs and {} clients. Data in {}", networks.len(), clients.len(), temp_pcap_prefix)).await;

    Ok(())
}
