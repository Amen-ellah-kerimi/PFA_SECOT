use std::process::{Child, Command};
use std::thread::sleep;
use std::time::Duration;

pub fn start_broker() -> Child {
    let child = Command::new("mosquitto")
        .arg("-v")
        .spawn()
        .expect("Failed to start MQTT broker");
    println!("MQTT Broker Started (PID: {})", child.id());
    sleep(Duration::from_secs(1));
    child
}

pub fn stop_broker(mut child: Child){
    if let Err(e) = child.kill(){
        eprintln!("Err Stopping MQTT Broker: {}", e);
    }
    else{
        println!("Disconnected from MQTT Broker");
    }
}