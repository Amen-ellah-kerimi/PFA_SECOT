use std::collections::HashMap;
use crate::data::models::{WifiNetwork, WifiClient, PacketInfo, CapturedCredential};
use prettytable::{Table, Row, Cell, format};
use colored::Colorize;

pub fn clear_terminal() {
    print!("\x1B[2J\x1B[1;1H");
}

fn get_table_format() -> format::TableFormat {
    format::FormatBuilder::new()
        .column_separator('│')
        .borders('│')
        .separators(&[format::LinePosition::Top], format::LineSeparator::new('─', '┬', '┌', '┐'))
        .separators(&[format::LinePosition::Bottom], format::LineSeparator::new('─', '┴', '└', '┘'))
        .separators(&[format::LinePosition::Title], format::LineSeparator::new('─', '┼', '├', '┤'))
        .padding(1, 1)
        .build()
}

fn print_header(title: &str) {
    println!("\n{}", "╔════════════════════════════════════════════════════════════════════════════╗".cyan());
    println!("{}", format!("║ {:^70} ║", title).cyan());
    println!("{}", "╚════════════════════════════════════════════════════════════════════════════╝".cyan());
}

fn print_footer() {
    println!("{}", "════════════════════════════════════════════════════════════════════════════".cyan());
}

pub fn display_scan_results(
    networks: &HashMap<String, WifiNetwork>,
    clients: &HashMap<String, WifiClient>,
    elapsed_time: u64,
) {
    print_header(&format!("📡 Wi-Fi Scan Results (Elapsed: {}s)", elapsed_time));

    let mut ap_table = Table::new();
    ap_table.set_format(get_table_format());

    // Add a title row
    ap_table.add_row(Row::new(vec![
        Cell::new(&"Access Points".bold().cyan().to_string()).with_hspan(6)
    ]));

    ap_table.add_row(Row::new(vec![
        Cell::new(&"BSSID".bold().underline().to_string()),
        Cell::new(&"SSID".bold().underline().to_string()),
        Cell::new(&"Channel".bold().underline().to_string()),
        Cell::new(&"Power".bold().underline().to_string()),
        Cell::new(&"Encryption".bold().underline().to_string()),
        Cell::new(&"Clients".bold().underline().to_string()),
    ]));

    let mut sorted_networks: Vec<&WifiNetwork> = networks.values().collect();
    sorted_networks.sort_by(|a, b| b.power.cmp(&a.power));

    for ap in sorted_networks {
        let client_count = clients.values().filter(|c| c.bssid == ap.bssid).count();
        let power_str = format!("{} dBm", ap.power);
        let power_colored = if ap.power > -50 {
            power_str.green()
        } else if ap.power > -70 {
            power_str.yellow()
        } else {
            power_str.red()
        };
        
        ap_table.add_row(Row::new(vec![
            Cell::new(&ap.bssid.green().to_string()),
            Cell::new(&ap.ssid.yellow().to_string()),
            Cell::new(&ap.channel.to_string().blue().to_string()),
            Cell::new(&power_colored.to_string()),
            Cell::new(&ap.encryption.to_string()),
            Cell::new(&format!("{}", client_count).cyan().to_string()),
        ]));
    }

    ap_table.printstd();

    if !clients.is_empty() {
        let mut client_table = Table::new();
        client_table.set_format(get_table_format());

        // Add a title row
        client_table.add_row(Row::new(vec![
            Cell::new(&"Connected Clients".bold().cyan().to_string()).with_hspan(4)
        ]));

        client_table.add_row(Row::new(vec![
            Cell::new(&"Client MAC".bold().underline().to_string()),
            Cell::new(&"Associated AP".bold().underline().to_string()),
            Cell::new(&"Power".bold().underline().to_string()),
            Cell::new(&"Probed SSIDs".bold().underline().to_string()),
        ]));

        let mut sorted_clients: Vec<&WifiClient> = clients.values().collect();
        sorted_clients.sort_by(|a, b| b.power.cmp(&a.power));

        for client in sorted_clients {
            let power_str = format!("{} dBm", client.power);
            let power_colored = if client.power > -50 {
                power_str.green()
            } else if client.power > -70 {
                power_str.yellow()
            } else {
                power_str.red()
            };
            
            client_table.add_row(Row::new(vec![
                Cell::new(&client.mac.purple().to_string()),
                Cell::new(&client.bssid.bright_blue().to_string()),
                Cell::new(&power_colored.to_string()),
                Cell::new(&client.probe_ssid.as_deref().unwrap_or("N/A").dimmed().to_string()),
            ]));
        }

        println!("\n");
        client_table.printstd();
    }

    print_footer();
}

pub fn display_packet_info_table(packets: &[PacketInfo], title: &str) {
    print_header(&format!("📦 {}", title));
    
    let mut table = Table::new();
    table.set_format(get_table_format());

    table.add_row(Row::new(vec![
        Cell::new(&"Timestamp".bold().underline().to_string()),
        Cell::new(&"Source IP".bold().underline().to_string()),
        Cell::new(&"Dest IP".bold().underline().to_string()),
        Cell::new(&"Protocol".bold().underline().to_string()),
        Cell::new(&"Size".bold().underline().to_string()),
        Cell::new(&"Summary".bold().underline().to_string()),
    ]));

    for packet in packets {
        let protocol_colored = match packet.protocol.to_lowercase().as_str() {
            "tcp" => packet.protocol.green(),
            "udp" => packet.protocol.yellow(),
            "icmp" => packet.protocol.red(),
            _ => packet.protocol.blue()
        };

        table.add_row(Row::new(vec![
            Cell::new(&packet.timestamp.dimmed().to_string()),
            Cell::new(&packet.source_ip.to_string()),
            Cell::new(&packet.dest_ip.to_string()),
            Cell::new(&protocol_colored.to_string()),
            Cell::new(&format!("{} B", packet.size).cyan().to_string()),
            Cell::new(&packet.summary.to_string()),
        ]));
    }

    table.printstd();
    print_footer();
}

pub fn display_credentials_table(credentials: &[CapturedCredential], title: &str) {
    print_header(&format!("🔑 {}", title));
    
    let mut table = Table::new();
    table.set_format(get_table_format());

    table.add_row(Row::new(vec![
        Cell::new(&"Timestamp".bold().underline().to_string()),
        Cell::new(&"Service".bold().underline().to_string()),
        Cell::new(&"Username".bold().underline().to_string()),
        Cell::new(&"Password".bold().underline().to_string()),
        Cell::new(&"Source Attack".bold().underline().to_string()),
        Cell::new(&"Target IP".bold().underline().to_string()),
    ]));

    for cred in credentials {
        let service_colored = match cred.service.to_lowercase().as_str() {
            "mqtt" => cred.service.green(),
            "http" => cred.service.blue(),
            "ftp" => cred.service.yellow(),
            _ => cred.service.normal()
        };

        table.add_row(Row::new(vec![
            Cell::new(&cred.timestamp.dimmed().to_string()),
            Cell::new(&service_colored.to_string()),
            Cell::new(&cred.username.yellow().to_string()),
            Cell::new(&cred.password.red().to_string()),
            Cell::new(&cred.source_attack.to_string()),
            Cell::new(&cred.target_ip.as_deref().unwrap_or("N/A").to_string()),
        ]));
    }

    table.printstd();
    print_footer();
}
