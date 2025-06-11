package com.smartlight.app;

import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.appcompat.widget.Toolbar;
import com.google.android.material.slider.Slider;
import com.google.android.material.switchmaterial.SwitchMaterial;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttSecurityException;

public class MainActivity extends AppCompatActivity implements MqttCallback {
    private MqttClient mqttClient;
    private SwitchMaterial powerSwitch;
    private Slider brightnessSlider;
    private Slider colorSlider;
    private TextView ambientText;
    private TextView motionText;
    private TextView statusText;

    // MQTT Broker configurations
    private static final MqttConfig[] MQTT_CONFIGS = {
        // No security
        new MqttConfig("tcp://192.168.1.100:1883", "", ""),
        // Basic security
        new MqttConfig("tcp://192.168.1.100:1884", "smart_light", "1234"),
        // Secure TLS
        new MqttConfig("ssl://192.168.1.100:8883", "smart_light", "SmartLight2024!")
    };

    private static final String CLIENT_ID = "SmartLight_" + System.currentTimeMillis();
    private int currentConfigIndex = 0;

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
        initializeMqtt();

        // Setup UI controls
        setupControls();
    }

    private void initializeMqtt() {
        // Try each MQTT configuration until one succeeds
        for (int i = 0; i < MQTT_CONFIGS.length; i++) {
            currentConfigIndex = i;
            MqttConfig config = MQTT_CONFIGS[i];
            
            try {
                mqttClient = new MqttClient(this, config.broker, CLIENT_ID);
                mqttClient.setCallback(this);

                MqttConnectOptions options = new MqttConnectOptions();
                options.setCleanSession(true);
                options.setConnectionTimeout(30);
                options.setKeepAliveInterval(60);
                
                if (!config.username.isEmpty()) {
                    options.setUserName(config.username);
                    options.setPassword(config.password.toCharArray());
                }

                mqttClient.connect(options);
                updateStatus("Connected to " + config.broker, true);
                return;
            } catch (MqttSecurityException e) {
                handleMqttError("Security error: " + e.getMessage());
            } catch (MqttException e) {
                handleMqttError("Connection error: " + e.getMessage());
            }
        }
        
        // If we get here, all configurations failed
        updateStatus("Failed to connect to any MQTT broker", false);
        Toast.makeText(this, "Failed to connect to any MQTT broker", Toast.LENGTH_LONG).show();
    }

    private void handleMqttError(String error) {
        runOnUiThread(() -> {
            updateStatus(error, false);
            Toast.makeText(this, error, Toast.LENGTH_SHORT).show();
        });
    }

    private void setupControls() {
        powerSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            try {
                mqttClient.publish(MqttClient.COMMAND_TOPIC, isChecked ? "ON" : "OFF");
            } catch (MqttException e) {
                handleMqttError("Failed to send command: " + e.getMessage());
                powerSwitch.setChecked(!isChecked); // Revert the switch
            }
        });

        brightnessSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                try {
                    mqttClient.publish(MqttClient.BRIGHTNESS_TOPIC, String.valueOf((int) value));
                } catch (MqttException e) {
                    handleMqttError("Failed to update brightness: " + e.getMessage());
                }
            }
        });

        colorSlider.addOnChangeListener((slider, value, fromUser) -> {
            if (fromUser) {
                try {
                    mqttClient.publish(MqttClient.COLOR_TOPIC, String.valueOf((int) value));
                } catch (MqttException e) {
                    handleMqttError("Failed to update color: " + e.getMessage());
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
        if (item.getItemId() == R.id.action_toggle_theme) {
            toggleTheme();
            return true;
        } else if (item.getItemId() == R.id.action_reconnect) {
            reconnectMqtt();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void reconnectMqtt() {
        if (mqttClient != null) {
            try {
                mqttClient.disconnect();
            } catch (MqttException e) {
                // Ignore disconnect errors
            }
        }
        initializeMqtt();
    }

    private void toggleTheme() {
        int currentMode = AppCompatDelegate.getDefaultNightMode();
        int newMode = currentMode == AppCompatDelegate.MODE_NIGHT_YES
                ? AppCompatDelegate.MODE_NIGHT_NO
                : AppCompatDelegate.MODE_NIGHT_YES;
        AppCompatDelegate.setDefaultNightMode(newMode);
    }

    @Override
    public void connectionLost(Throwable cause) {
        handleMqttError("Connection lost: " + cause.getMessage());
        // Try to reconnect with next configuration
        reconnectMqtt();
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
                        updateStatus(payload, true);
                        break;
                }
            } catch (Exception e) {
                handleMqttError("Error processing message: " + e.getMessage());
            }
        });
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Not used in this implementation
    }

    private void updateStatus(String status, boolean isConnected) {
        statusText.setText(status);
        int colorRes = isConnected ? R.color.status_connected : R.color.status_disconnected;
        statusText.setTextColor(getColor(colorRes));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mqttClient != null) {
            try {
                mqttClient.disconnect();
            } catch (MqttException e) {
                e.printStackTrace();
                Toast.makeText(this, "Error disconnecting from MQTT broker: " + e.getMessage(), Toast.LENGTH_SHORT).show();
            }
        }
    }

    // MQTT Configuration class
    private static class MqttConfig {
        final String broker;
        final String username;
        final String password;

        MqttConfig(String broker, String username, String password) {
            this.broker = broker;
            this.username = username;
            this.password = password;
        }
    }
} 