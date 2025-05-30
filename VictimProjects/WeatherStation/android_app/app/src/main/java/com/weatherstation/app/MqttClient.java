package com.weatherstation.app;

import android.content.Context;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MqttClient {
    private static final String BROKER = "tcp://localhost:1883";
    private static final String CLIENT_ID = "android_weatherstation_" + System.currentTimeMillis();
    private static final int QOS = 1;

    // MQTT Topics
    public static final String TEMPERATURE_TOPIC = "home/weatherstation/temperature";
    public static final String HUMIDITY_TOPIC = "home/weatherstation/humidity";
    public static final String STATUS_TOPIC = "home/weatherstation/status";
    public static final String DATA_TOPIC = "home/weatherstation/data";

    private org.eclipse.paho.client.mqttv3.MqttClient mqttClient;
    private MqttCallback callback;
    private String username;
    private String password;

    public MqttClient(Context context, String username, String password) {
        this.username = username;
        this.password = password;
    }

    public void connect() throws MqttException {
        mqttClient = new org.eclipse.paho.client.mqttv3.MqttClient(BROKER, CLIENT_ID, new MemoryPersistence());
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true);
        options.setConnectionTimeout(30);
        options.setKeepAliveInterval(60);
        options.setAutomaticReconnect(true);

        if (username != null && !username.isEmpty() && password != null && !password.isEmpty()) {
            options.setUserName(username);
            options.setPassword(password.toCharArray());
        }

        if (callback != null) {
            mqttClient.setCallback(callback);
        }

        mqttClient.connect(options);
        subscribeToTopics();
    }

    private void subscribeToTopics() throws MqttException {
        mqttClient.subscribe(TEMPERATURE_TOPIC, QOS);
        mqttClient.subscribe(HUMIDITY_TOPIC, QOS);
        mqttClient.subscribe(STATUS_TOPIC, QOS);
        mqttClient.subscribe(DATA_TOPIC, QOS);
    }

    public void disconnect() throws MqttException {
        if (mqttClient != null && mqttClient.isConnected()) {
            mqttClient.disconnect();
        }
    }

    public void publish(String topic, String message) throws MqttException {
        if (mqttClient != null && mqttClient.isConnected()) {
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            mqttMessage.setQos(QOS);
            mqttClient.publish(topic, mqttMessage);
        }
    }

    public void setCallback(MqttCallback callback) {
        this.callback = callback;
        if (mqttClient != null) {
            mqttClient.setCallback(callback);
        }
    }

    public boolean isConnected() {
        return mqttClient != null && mqttClient.isConnected();
    }
} 