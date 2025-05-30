use tokio::process::Command;
use colored::Colorize;
use tokio::time::{sleep, Duration};
use crate::data::logger;

pub async fn run_general_mitm(
    interface: &str,
    duration: u64,
) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Starting general MITM attack using Ettercap on '{}'...", interface).blue());
    println!("{}", "  This will launch Ettercap's graphical interface.".dimmed());
    println!("{}", "  You need to manually select 'Sniff -> Unified sniffing', choose the interface, then 'Hosts -> Scan for hosts', and finally 'Mitm -> Arp poisoning' with 'Sniff remote connections' checked.".dimmed());

    let mut ettercap_command = Command::new("sudo");
    ettercap_command.arg("ettercap").arg("-G");

    println!("{}", format!("  Executing: sudo ettercap -G").dimmed());
    let mut child = ettercap_command.spawn()?;

    println!("{}", "\nGeneral MITM initiated. Please configure Ettercap. (Press Ctrl+C to stop Ettercap)".yellow().bold());
    logger::log_attack_event("mitm_general", "active", &format!("General MITM initiated on {}.", interface)).await;

    if duration > 0 {
        sleep(Duration::from_secs(duration)).await;
        println!("{}", "\n• MITM duration reached. Stopping Ettercap...".blue());
        let _ = child.kill().await;
    } else {
        println!("{}", "\nMITM will run indefinitely until stopped manually (Ctrl+C on Ettercap window).".dimmed());
        let _ = child.wait().await;
    }

    println!("{}", "• General MITM attack stopped.".green());
    logger::log_attack_event("mitm_general", "stopped", &format!("General MITM on {} stopped.", interface)).await;

    Ok(())
}
