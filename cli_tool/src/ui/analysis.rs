use crate::parsers::tshark_parser;
use crate::data::logger;
use crate::ui::formatter;
use colored::Colorize;

pub async fn perform_analysis(path: &str, analysis_type: &str) -> Result<(), Box<dyn std::error::Error>> {
    println!("{}", format!("• Starting analysis type '{}' on path '{}'...", analysis_type, path).blue());

    match analysis_type {
        "mqtt-traffic" => {
            let packets = tshark_parser::parse_pcap_for_mqtt_traffic(path).await?;
            if packets.is_empty() {
                println!("{}", "  No MQTT traffic found in the provided pcap file.".yellow());
                logger::log_attack_event("analysis", "no_data", &format!("No MQTT traffic found in {}", path)).await;
            } else {
                formatter::display_packet_info_table(&packets, &format!("MQTT Traffic from {}", path));
                logger::log_attack_event("analysis", "success", &format!("Analyzed {} MQTT packets in {}", packets.len(), path)).await;
            }
        },
        "credentials-log" => {
            let credentials = logger::read_credentials_from_logs(path).await?;
            if credentials.is_empty() {
                println!("{}", "  No captured credentials found in the log files.".yellow());
                logger::log_attack_event("analysis", "no_data", &format!("No credentials found in logs at {}", path)).await;
            } else {
                formatter::display_credentials_table(&credentials, "Captured Credentials");
                logger::log_attack_event("analysis", "success", &format!("Analyzed {} credentials from logs at {}", credentials.len(), path)).await;
            }
        }
        _ => {
            eprintln!("{}", format!("Unknown analysis type: '{}'. Supported types: mqtt-traffic, credentials-log.", analysis_type).red());
            logger::log_attack_event("analysis", "error", &format!("Unknown analysis type: {}", analysis_type)).await;
        }
    }

    println!("{}", "• Analysis completed.".green());
    Ok(())
}
