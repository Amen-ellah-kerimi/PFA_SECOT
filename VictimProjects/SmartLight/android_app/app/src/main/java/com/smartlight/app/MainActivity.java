package com.smartlight.app;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Switch;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import com.google.android.material.slider.Slider;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.JSONObject;

public class MainActivity extends AppCompatActivity implements MqttCallback {
    private MqttClient mqttClient;
    private Switch powerSwitch;
    private Slider brightnessSlider;
    private Slider colorSlider;
    private TextView ambientText;
    private TextView motionText;
    private TextView statusText;

    private static final String BROKER = "tcp://your-mqtt-broker:1883";
    private static final String CLIENT_ID = "smartlight-android";
    private static final String USERNAME = "your-username";
    private static final String PASSWORD = "your-password";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Initialize views
        powerSwitch = findViewById(R.id.power_switch);
        brightnessSlider = findViewById(R.id.brightness_slider);
        colorSlider = findViewById(R.id.color_slider);
        ambientText = findViewById(R.id.ambient_text);
        motionText = findViewById(R.id.motion_text);
        statusText = findViewById(R.id.status_text);

        // Setup MQTT client
        mqttClient = new MqttClient(this, BROKER, CLIENT_ID, USERNAME, PASSWORD);
        mqttClient.setCallback(this);

        // Setup UI controls
        setupControls();

        // Connect to MQTT broker
        try {
            mqttClient.connect();
            updateStatus("Connected");
        } catch (Exception e) {
            updateStatus("Connection failed: " + e.getMessage());
        }
    }

    private void setupControls() {
        powerSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            try {
                mqttClient.publish(MqttClient.COMMAND_TOPIC, isChecked ? "ON" : "OFF");
            } catch (Exception e) {
                Toast.makeText(this, "Failed to send command", Toast.LENGTH_SHORT).show();
            }
        });

        brightnessSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                try {
                    mqttClient.publish(MqttClient.BRIGHTNESS_TOPIC, String.valueOf((int) value));
                } catch (Exception e) {
                    Toast.makeText(this, "Failed to update brightness", Toast.LENGTH_SHORT).show();
                }
            }
        });

        colorSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                try {
                    mqttClient.publish(MqttClient.COLOR_TOPIC, String.valueOf((int) value));
                } catch (Exception e) {
                    Toast.makeText(this, "Failed to update color", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.action_refresh) {
            try {
                mqttClient.publish(MqttClient.COMMAND_TOPIC, "STATUS");
                return true;
            } catch (Exception e) {
                Toast.makeText(this, "Failed to refresh status", Toast.LENGTH_SHORT).show();
            }
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void connectionLost(Throwable cause) {
        runOnUiThread(() -> updateStatus("Connection lost: " + cause.getMessage()));
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) {
        String payload = new String(message.getPayload());
        runOnUiThread(() -> {
            try {
                switch (topic) {
                    case MqttClient.STATE_TOPIC:
                        powerSwitch.setChecked(payload.equals("ON"));
                        break;
                    case MqttClient.BRIGHTNESS_TOPIC:
                        brightnessSlider.setValue(Float.parseFloat(payload));
                        break;
                    case MqttClient.COLOR_TOPIC:
                        colorSlider.setValue(Float.parseFloat(payload));
                        break;
                    case MqttClient.AMBIENT_TOPIC:
                        ambientText.setText("Ambient Light: " + payload + " lux");
                        break;
                    case MqttClient.MOTION_TOPIC:
                        motionText.setText("Motion: " + payload);
                        break;
                    case MqttClient.STATUS_TOPIC:
                        updateStatus(payload);
                        break;
                }
            } catch (Exception e) {
                Toast.makeText(this, "Error processing message: " + e.getMessage(), Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Not used in this implementation
    }

    private void updateStatus(String status) {
        statusText.setText(status);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            mqttClient.disconnect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
} 