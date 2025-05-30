#[derive(Debug, Clone)]
pub struct WifiNetwork {
    pub bssid: String,
    pub ssid: String,
    pub channel: u8,
    pub encryption: String,
    pub power: i32,
    pub clients: Vec<WifiClient>,
}

#[derive(Debug, Clone)]
pub struct WifiClient {
    pub mac: String,
    pub bssid: String,
    pub power: i32,
    pub probe_ssid: Option<String>,
}

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct CapturedCredential {
    pub service: String,
    pub username: String,
    pub password: String,
    pub timestamp: String,
    pub source_attack: String,
    pub target_ip: Option<String>,
}

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct AttackEvent {
    pub timestamp: String,
    pub attack_type: String,
    pub status: String,
    pub message: String,
}
#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct PacketInfo{
    pub timestamp: String,
    pub source_ip: String,
    pub dest_ip: String,
    pub protocol: String,
    pub size: u32,
    pub summary: String,
}
