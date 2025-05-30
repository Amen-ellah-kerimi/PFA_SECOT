use clap::{Parser, Subcommand};
use colored::Colorize;

mod attack_modules;
mod data;
mod parsers;
mod ui;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
#[command(propagate_version = true)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    Scan {
        #[arg(short, long)]
        interface: String,
        #[arg(short, long, default_value_t = 30)]
        duration: u64,
    },
    Deauth {
        #[arg(short, long)]
        interface: String,
        #[arg(short, long)]
        ap_bssid: String,
        #[arg(short, long)]
        client_mac: Option<String>,
        #[arg(short, long, default_value_t = 100)]
        packets: u32,
    },
    #[command(name = "evil-twin")]
    EvilTwin {
        #[arg(short, long)]
        interface: String,
        #[arg(short, long)]
        ssid: String,
        #[arg(short, long)]
        channel: u8,
        #[arg(short, long)]
        target_ip_range: String,
        #[arg(short, long, default_value = "")]
        phishing_template: String,
    },
    #[command(name = "arp-spoof")]
    ArpSpoof {
        #[arg(short, long)]
        interface: String,
        #[arg(short, long)]
        victim_ip: String,
        #[arg(short, long)]
        gateway_ip: String,
        #[arg(short, long, default_value_t = 0)]
        duration: u64,
    },
    #[command(name = "mqtt-attack")]
    MqttAttack {
        #[arg(short, long)]
        broker: String,
        #[arg(short, long, default_value_t = 1883)]
        port: u16,
        #[command(subcommand)]
        scenario: MqttAttackScenario,
    },
    Analyze {
        #[arg(short, long)]
        path: String,
        #[arg(short, long)]
        analysis_type: String,
    },
}

#[derive(Subcommand, Debug)]
enum MqttAttackScenario {
    Flood {
        #[arg(short, long, default_value = "test/flood")]
        topic: String,
        #[arg(short, long, default_value_t = 0)]
        count: u32,
    },
    #[command(name = "fake-publish")]
    FakePublish {
        #[arg(short, long)]
        topic: String,
        #[arg(short, long)]
        payload: String,
        #[arg(short, long, default_value_t = 1)]
        count: u32,
    },
    #[command(name = "brute-force")]
    BruteForce {
        #[arg(short, long)]
        users: String,
        #[arg(short, long)]
        passwords: String,
    },
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Scan { interface, duration } => {
            println!("{}", format!("\n📡 Starting Wi-Fi scan on '{}' for {} seconds...\n", interface, duration).cyan().bold());
            attack_modules::wifi_scan::run_scan(interface, *duration).await?;
        },
        Commands::Deauth { interface, ap_bssid, client_mac, packets } => {
            println!("{}", format!("\n💀 Initiating deauthentication attack on AP '{}'...\n", ap_bssid).red().bold());
            attack_modules::wifi_deauth::run_deauth_attack(interface, ap_bssid, client_mac.as_deref(), *packets).await?;
        },
        Commands::EvilTwin { interface, ssid, channel, target_ip_range, phishing_template } => {
            println!("{}", format!("\n😈 Deploying Evil Twin AP '{}' on channel {}...\n", ssid, channel).purple().bold());
            attack_modules::wifi_evil_twin::run_evil_twin(interface, ssid, *channel, target_ip_range, phishing_template).await?;
        },
        Commands::ArpSpoof { interface, victim_ip, gateway_ip, duration } => {
            println!("{}", format!("\n🕸️ Starting ARP Spoofing: Victim '{}' Gateway '{}' on '{}'...\n", victim_ip, gateway_ip, interface).magenta().bold());
            attack_modules::wifi_arp_spoof::run_arp_spoof(interface, victim_ip, gateway_ip, *duration).await?;
        },
        Commands::MqttAttack { broker, port, scenario } => {
            println!("{}", format!("\n💡 Targeting MQTT broker at {}:{}...\n", broker, port).yellow().bold());
            match scenario {
                MqttAttackScenario::Flood { topic, count } => {
                    attack_modules::mqtt_attacks::flood_broker(broker, *port, topic, *count).await?;
                },
                MqttAttackScenario::FakePublish { topic, payload, count } => {
                    attack_modules::mqtt_attacks::fake_publish(broker, *port, topic, payload, *count).await?;
                },
                MqttAttackScenario::BruteForce { users, passwords } => {
                    attack_modules::mqtt_attacks::brute_force_mqtt(broker, *port, users, passwords).await?;
                },
            }
        },
        Commands::Analyze { path, analysis_type } => {
            println!("{}", format!("\n🔍 Analyzing '{}' for '{}'...\n", path, analysis_type).blue().bold());
            ui::analysis::perform_analysis(path, analysis_type).await?;
        },
    }

    println!("{}", "\n✅ Operation completed.\n".green().bold());
    Ok(())
}
