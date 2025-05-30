use crate::models::network::DeviceInfo;
use crate::output::table::{create_table, FormattedTable};
use anyhow::{anyhow, Result};
use std::io::Write;
use std::net::{IpAddr, Ipv4Addr};
use std::process::Command;
use regex::Regex;

pub async fn run_network_scan(cidr: &str, output_format: &str) -> Result<()> {
    println!("Scanning network {}...", cidr);

    // Parse CIDR notation
    let parts: Vec<&str> = cidr.split('/').collect();
    if parts.len() != 2 {
        return Err(anyhow!("Invalid CIDR format. Expected format: 192.168.1.0/24"));
    }

    let ip_str = parts[0];
    let prefix_len: u8 = parts[1].parse()?;

    if prefix_len > 32 {
        return Err(anyhow!("Invalid prefix length. Must be between 0 and 32"));
    }

    // For simplicity, we'll just scan a few IPs in the subnet
    let base_ip: Ipv4Addr = ip_str.parse()?;
    let base_ip_u32 = u32::from(base_ip);
    let mask = !0u32 << (32 - prefix_len);
    let network = base_ip_u32 & mask;

    let mut devices = Vec::new();

    // Scan a limited number of IPs for demonstration
    let max_hosts = std::cmp::min(1 << (32 - prefix_len), 20); // Limit to 20 hosts max

    for i in 1..max_hosts {
        let ip_u32 = network + i;
        let ip = Ipv4Addr::from(ip_u32);

        print!(".");
        std::io::stdout().flush().unwrap();

        if ping_host(&ip.to_string()) {
            let hostname = get_hostname(&ip.to_string()).unwrap_or_else(|| "unknown".to_string());

            devices.push(DeviceInfo {
                ip: IpAddr::V4(ip),
                hostname,
            });
        }
    }

    println!("\nScan complete! Found {} devices", devices.len());

    // Format and display the results
    if output_format == "json" {
        let json = serde_json::to_string_pretty(&devices)?;
        println!("{}", json);
    } else {
        match create_table(&devices, &["ip", "hostname"]) {
            Ok(table) => {
                let formatted_table = FormattedTable::new("Discovered Devices", table);
                println!("{}", formatted_table);
            },
            Err(e) => {
                println!("Error creating table: {}", e);
                // Fallback to simple output
                println!("Discovered Devices:");
                for device in &devices {
                    println!("  IP: {}, Hostname: {}", device.ip, device.hostname);
                }
            }
        }
    }

    Ok(())
}

fn ping_host(ip: &str) -> bool {
    #[cfg(target_os = "windows")]
    let output = Command::new("ping")
        .args(&["-n", "1", "-w", "500", ip])
        .output();

    #[cfg(not(target_os = "windows"))]
    let output = Command::new("ping")
        .args(&["-c", "1", "-W", "1", ip])
        .output();

    match output {
        Ok(output) => output.status.success(),
        Err(_) => false,
    }
}

fn get_hostname(ip: &str) -> Option<String> {
    #[cfg(target_os = "windows")]
    let output = Command::new("nslookup")
        .arg(ip)
        .output();

    #[cfg(not(target_os = "windows"))]
    let output = Command::new("host")
        .arg(ip)
        .output();

    match output {
        Ok(output) => {
            let stdout = String::from_utf8_lossy(&output.stdout);

            #[cfg(target_os = "windows")]
            {
                let re = Regex::new(r"Name:\s+([^\s]+)").ok()?;
                if let Some(caps) = re.captures(&stdout) {
                    return Some(caps.get(1)?.as_str().to_string());
                }
            }

            #[cfg(not(target_os = "windows"))]
            {
                let re = Regex::new(r"domain name pointer ([^\s]+)").ok()?;
                if let Some(caps) = re.captures(&stdout) {
                    return Some(caps.get(1)?.as_str().to_string());
                }
            }

            None
        },
        Err(_) => None,
    }
}