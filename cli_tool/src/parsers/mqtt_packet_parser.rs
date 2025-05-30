use crate::data::models::PacketInfo;
use colored::Colorize;

pub fn parse_mqtt_packet_summary(raw_data: &[u8]) -> Option<PacketInfo> {
    if raw_data.len() < 1 {
        return None;
    }

    let first_byte = raw_data[0];
    let msg_type = (first_byte >> 4) & 0x0F;

    let msg_type_str = match msg_type {
        1 => "CONNECT",
        2 => "CONNACK",
        3 => "PUBLISH",
        4 => "PUBACK",
        5 => "PUBREC",
        6 => "PUBREL",
        7 => "PUBCOMP",
        8 => "SUBSCRIBE",
        9 => "SUBACK",
        10 => "UNSUBSCRIBE",
        11 => "UNSUBACK",
        12 => "PINGREQ",
        13 => "PINGRESP",
        14 => "DISCONNECT",
        _ => "UNKNOWN",
    };

    let mut summary = format!("MQTT {}", msg_type_str);

    if msg_type == 3 && raw_data.len() > 3 {
        let remaining_length = decode_remaining_length(&raw_data[1..])?;
        let mut offset = 1 + get_remaining_length_bytes(&raw_data[1..])?;

        if offset + 2 > raw_data.len() {
            return None;
        }
        let topic_length = ((raw_data[offset] as u16) << 8) | (raw_data[offset + 1] as u16);
        offset += 2;

        if offset + topic_length as usize > raw_data.len() {
            return None;
        }
        let topic = String::from_utf8_lossy(&raw_data[offset..offset + topic_length as usize]);
        summary.push_str(&format!(", Topic: {}", topic));
        offset += topic_length as usize;

        let qos = (first_byte >> 1) & 0x03;
        if qos > 0 && offset + 2 <= raw_data.len() {
            offset += 2;
        }

        if offset < raw_data.len() {
            let payload = String::from_utf8_lossy(&raw_data[offset..]);
            summary.push_str(&format!(", Payload: {}", payload));
        }
    }

    Some(PacketInfo {
        timestamp: chrono::Local::now().to_rfc3339(),
        source_ip: "N/A".to_string(),
        dest_ip: "N/A".to_string(),
        protocol: "MQTT".to_string(),
        size: raw_data.len() as u32,
        summary,
    })
}

fn decode_remaining_length(bytes: &[u8]) -> Option<u32> {
    let mut multiplier = 1;
    let mut value = 0;
    let mut i = 0;
    loop {
        if i >= bytes.len() { return None; }
        let encoded_byte = bytes[i];
        value += (encoded_byte & 127) as u32 * multiplier;
        multiplier *= 128;
        if multiplier > 128 * 128 * 128 { return None; } // Max 4 bytes for length
        if (encoded_byte & 128) == 0 {
            break;
        }
        i += 1;
    }
    Some(value)
}

fn get_remaining_length_bytes(bytes: &[u8]) -> Option<usize> {
    let mut i = 0;
    loop {
        if i >= bytes.len() { return None; }
        let encoded_byte = bytes[i];
        i += 1;
        if (encoded_byte & 128) == 0 {
            break;
        }
    }
    Some(i)
}
