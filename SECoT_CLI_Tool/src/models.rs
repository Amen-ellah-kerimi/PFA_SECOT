pub mod port {
    use serde::{Deserialize, Serialize};
    use std::net::IpAddr;
    use std::fmt;

    #[derive(Serialize, Deserialize, Debug)]
    pub struct IpAddress(pub IpAddr);

    impl fmt::Display for IpAddress {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(f, "{}", self.0)
        }
    }

    #[derive(Serialize, Deserialize, Debug)]
    pub struct PortStatus {
        pub port: u16,
        pub status: String,
        pub service: Option<String>,
    }

    impl fmt::Display for PortStatus {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(
                f,
                "Port: {}, Status: {}, Service: {}",
                self.port,
                self.status,
                self.service.as_deref().unwrap_or("unknown")
            )
        }
    }

    #[derive(Serialize, Deserialize, Debug)]
    pub struct PortScanResults {
        pub ip: IpAddress,
        pub results: Vec<PortStatus>,
    }

    impl fmt::Display for PortScanResults {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            writeln!(f, "Port Scan Results for {}:", self.ip)?;
            for port in &self.results {
                writeln!(f, "  {}", port)?;
            }
            Ok(())
        }
    }
}

pub mod network {
    use serde::{Deserialize, Serialize};
    use std::net::IpAddr;
    use std::fmt;

    #[derive(Serialize, Deserialize, Debug)]
    pub struct DeviceInfo {
        pub ip: IpAddr,
        pub hostname: String,
    }

    #[derive(Serialize, Deserialize, Debug)]
    pub struct MqttBroker {
        pub ip: IpAddr,
        pub port: u16,
        pub requires_auth: bool,
        pub supports_tls: bool,
        pub is_accessible: bool,
    }

    #[derive(Serialize, Deserialize, Debug)]
    pub struct WiFiNetwork {
        pub ssid: String,
        pub bssid: String,
        pub channel: u8,
        pub rssi: i32,
        pub encryption: String,
        pub hidden: bool,
    }

    impl fmt::Display for DeviceInfo {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(f, "IP: {}, Hostname: {}", self.ip, self.hostname)
        }
    }

    impl fmt::Display for MqttBroker {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(
                f,
                "IP: {}:{}, Auth: {}, TLS: {}, Accessible: {}",
                self.ip,
                self.port,
                self.requires_auth,
                self.supports_tls,
                self.is_accessible
            )
        }
    }

    impl fmt::Display for WiFiNetwork {
        fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            write!(
                f,
                "SSID: {}, BSSID: {}, Channel: {}, RSSI: {}, Encryption: {}, Hidden: {}",
                self.ssid,
                self.bssid,
                self.channel,
                self.rssi,
                self.encryption,
                self.hidden
            )
        }
    }
}