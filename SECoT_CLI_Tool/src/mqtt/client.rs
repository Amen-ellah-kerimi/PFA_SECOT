use crate::models::network::MqttBroker;
use anyhow::{anyhow, Result};
use rumqttc::{AsyncClient, Event, MqttOptions, QoS};
use std::time::Duration;
use tokio::{sync::mpsc, task};
use uuid::Uuid;

pub struct MqttClient {
    client: AsyncClient,
    tx: mpsc::Sender<(String, String)>,
    client_id: String,
}

impl MqttClient {
    pub async fn connect(broker: &MqttBroker, username: Option<&str>, password: Option<&str>) -> Result<Self> {
        let client_id = format!("secot_cli_{}", Uuid::new_v4());
        
        let mut mqtt_options = MqttOptions::new(&client_id, broker.ip.to_string(), broker.port);
        mqtt_options.set_keep_alive(Duration::from_secs(5));
        
        // Set credentials if provided
        if let (Some(user), Some(pass)) = (username, password) {
            mqtt_options.set_credentials(user, pass);
        }
        
        let (client, mut eventloop) = AsyncClient::new(mqtt_options, 10);
        let (tx, mut rx) = mpsc::channel::<(String, String)>(100);
        
        // Spawn a task to handle incoming messages
        task::spawn(async move {
            while let Ok(notification) = eventloop.poll().await {
                if let Event::Incoming(incoming) = notification {
                    println!("[MQTT IN] {:?}", incoming);
                }
            }
        });
        
        // Spawn a task to handle outgoing messages
        let client_clone = client.clone();
        task::spawn(async move {
            while let Some((topic, message)) = rx.recv().await {
                if let Err(e) = client_clone
                    .publish(&topic, QoS::AtLeastOnce, false, message)
                    .await
                {
                    eprintln!("MQTT publish error: {:?}", e);
                }
            }
        });
        
        Ok(Self {
            client,
            tx,
            client_id,
        })
    }
    
    pub async fn subscribe(&self, topic: &str) -> Result<()> {
        self.client.subscribe(topic, QoS::AtLeastOnce).await?;
        Ok(())
    }
    
    pub async fn publish(&self, topic: &str, message: &str) -> Result<()> {
        self.tx.send((topic.to_string(), message.to_string())).await
            .map_err(|e| anyhow!("Failed to send message to MQTT channel: {}", e))?;
        Ok(())
    }
    
    pub async fn disconnect(&self) -> Result<()> {
        self.client.disconnect().await?;
        Ok(())
    }
    
    pub fn get_client_id(&self) -> &str {
        &self.client_id
    }
}
