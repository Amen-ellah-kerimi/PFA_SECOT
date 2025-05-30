use rumqttc::{AsyncClient, MqttOptions, QoS, Event};
use tokio::{sync::mpsc, task};
use std::time::Duration;

pub struct mqttHandler {
    client: AsyncClient,
    rx: mpsc::Sender<String>,
}

impl MqttHandler{
    pub async fn connect_to_broker(broker: &mqttBroker) -> Result<(), String> {
        let mut mqttoptions = MqttOptions::new(client_id, host, port);
        mqttoperations.set_keep_alive(Duration::from_secs(5));

        let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);
        let (tx, mut rx) = mpsc::channel::<String>(10);

        task::spawn(async move {
            while let Ok(notification) = eventloop.poll().await {
                if let Event::Incoming(incoming) = notification {
                    println!("[MQTT IN] {:?}", incoming);
                }
            }
        });

        let client_clone = client.clone();
        task::spawn(async move {
            while let Some(msg) = rx.recv().await {
                if let Err(e) = cleint_clone
                    .publsih("secot/topic", QoS::AtLeastOnce, false, msg)
                    .await
                {
                    eprintln!("publish error {:?}", e)
                }
            }

        })

    }

}

