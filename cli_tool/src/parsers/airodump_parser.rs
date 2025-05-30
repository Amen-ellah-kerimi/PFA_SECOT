use crate::data::models::{WifiNetwork, WifiClient};

pub fn parse_airodump_ap_line(line: &str) -> Option<WifiNetwork> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    if parts.len() < 10 {
        return None;
    }

    let bssid = parts.get(0)?.to_string();
    if !bssid.contains(':') || bssid.len() != 17 {
        return None;
    }

    let power = parts.get(1)?.parse::<i32>().ok()?;
    let channel = parts.get(5)?.parse::<u8>().ok()?;
    let encryption = parts.get(7)?.to_string();

    let ssid_parts = &parts[9..];
    let ssid = ssid_parts.join(" ").trim().to_string();
    if ssid.is_empty() || ssid == "<lengthy" || ssid == "<hidden>" {
        return None;
    }

    Some(WifiNetwork {
        bssid,
        ssid,
        channel,
        encryption,
        power,
        clients: Vec::new(),
    })
}

pub fn parse_airodump_client_line(line: &str) -> Option<WifiClient> {
    let parts: Vec<&str> = line.split_whitespace().collect();
    if parts.len() < 2 {
        return None;
    }

    let mac = parts.get(0)?.to_string();
    if !mac.contains(':') || mac.len() != 17 {
        return None;
    }

    let power = parts.get(1)?.parse::<i32>().ok()?;

    let mut bssid = "N/A".to_string();
    let mut probe_ssid: Option<String> = None;

    if let Some(ap_index) = parts.iter().position(|&p| p == "(AP)") {
        if let Some(ap_mac) = parts.get(ap_index + 1) {
            bssid = ap_mac.to_string();
        }
    }

    if let Some(probe_index) = parts.iter().position(|&p| p == "Probe") {
        let ssid_parts_start_index = probe_index + 1;
        if ssid_parts_start_index < parts.len() {
            let probed_ssids = parts[ssid_parts_start_index..].join(" ").trim().to_string();
            if !probed_ssids.is_empty() {
                probe_ssid = Some(probed_ssids);
            }
        }
    }

    Some(WifiClient {
        mac,
        bssid,
        power,
        probe_ssid,
    })
}
