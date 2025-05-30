use crate::models::port::{IpAddress, PortScanResults, PortStatus};
use crate::output::formatter::format_output;
use anyhow::Result;
use std::io::Write;
use std::net::IpAddr;
use tokio::net::TcpStream;
use tokio::time::timeout;
use std::time::Duration;

pub async fn run_port_scan(ip_str: &str, output_format: &str) -> Result<()> {
    let ip: IpAddr = ip_str.parse()?;
    let ip_address = IpAddress(ip);

    println!("Scanning ports on {}...", ip);

    // Common ports to scan
    let common_ports = vec![
        21, 22, 23, 25, 53, 80, 110, 143, 443, 465, 587, 993, 995, 1883, 3306, 5432, 8080, 8883
    ];

    let mut results = Vec::new();

    for &port in &common_ports {
        let status = scan_port(ip, port).await;
        let service = get_service_name(port);

        results.push(PortStatus {
            port,
            status: status.to_string(),
            service: Some(service.to_string()),
        });

        print!(".");
        std::io::stdout().flush().unwrap();
    }

    println!("\nScan complete!");

    let result = PortScanResults {
        ip: ip_address,
        results,
    };

    // Format and display the result
    let output = format_output(&result, output_format)?;
    println!("{}", output);

    Ok(())
}

async fn scan_port(ip: IpAddr, port: u16) -> &'static str {
    let addr = format!("{}:{}", ip, port);
    match timeout(Duration::from_secs(2), TcpStream::connect(&addr)).await {
        Ok(Ok(_)) => "open",
        Ok(Err(_)) => "closed",
        Err(_) => "timeout",
    }
}

fn get_service_name(port: u16) -> &'static str {
    match port {
        21 => "FTP",
        22 => "SSH",
        23 => "Telnet",
        25 => "SMTP",
        53 => "DNS",
        80 => "HTTP",
        110 => "POP3",
        143 => "IMAP",
        443 => "HTTPS",
        465 => "SMTPS",
        587 => "SMTP",
        993 => "IMAPS",
        995 => "POP3S",
        1883 => "MQTT",
        3306 => "MySQL",
        5432 => "PostgreSQL",
        8080 => "HTTP-Alt",
        8883 => "MQTTS",
        _ => "unknown",
    }
}
