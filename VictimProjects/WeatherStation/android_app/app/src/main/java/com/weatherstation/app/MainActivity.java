package com.weatherstation.app;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity implements MqttCallback {
    private MqttClient mqttClient;
    private TextView temperatureText;
    private TextView humidityText;
    private TextView statusText;
    private LineChart temperatureChart;
    private LineChart humidityChart;
    private List<Entry> temperatureEntries;
    private List<Entry> humidityEntries;
    private int dataPointCount = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Setup toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Initialize views
        temperatureText = findViewById(R.id.temperature_text);
        humidityText = findViewById(R.id.humidity_text);
        statusText = findViewById(R.id.status_text);
        temperatureChart = findViewById(R.id.temperature_chart);
        humidityChart = findViewById(R.id.humidity_chart);

        // Initialize charts
        setupCharts();

        // Initialize MQTT client
        String broker = "tcp://192.168.1.100:1883"; // Replace with your MQTT broker address
        String clientId = "WeatherStation_" + System.currentTimeMillis();
        mqttClient = new MqttClient(this, broker, clientId, "", "");
        mqttClient.setCallback(this);
        mqttClient.connect();
    }

    private void setupCharts() {
        temperatureEntries = new ArrayList<>();
        humidityEntries = new ArrayList<>();

        // Setup temperature chart
        LineDataSet temperatureDataSet = new LineDataSet(temperatureEntries, "Temperature (°C)");
        temperatureDataSet.setColor(getResources().getColor(android.R.color.holo_red_dark));
        temperatureDataSet.setCircleColor(getResources().getColor(android.R.color.holo_red_dark));
        temperatureDataSet.setDrawValues(false);
        temperatureChart.getDescription().setEnabled(false);
        temperatureChart.setData(new LineData(temperatureDataSet));
        temperatureChart.invalidate();

        // Setup humidity chart
        LineDataSet humidityDataSet = new LineDataSet(humidityEntries, "Humidity (%)");
        humidityDataSet.setColor(getResources().getColor(android.R.color.holo_blue_dark));
        humidityDataSet.setCircleColor(getResources().getColor(android.R.color.holo_blue_dark));
        humidityDataSet.setDrawValues(false);
        humidityChart.getDescription().setEnabled(false);
        humidityChart.setData(new LineData(humidityDataSet));
        humidityChart.invalidate();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.action_refresh) {
            if (mqttClient.isConnected()) {
                mqttClient.publish(MqttClient.STATUS_TOPIC, "refresh");
                return true;
            } else {
                Toast.makeText(this, "Not connected to MQTT broker", Toast.LENGTH_SHORT).show();
            }
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void connectionLost(Throwable cause) {
        runOnUiThread(() -> {
            statusText.setText("Disconnected");
            Toast.makeText(this, "Connection lost", Toast.LENGTH_SHORT).show();
        });
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) {
        String payload = message.toString();
        runOnUiThread(() -> {
            switch (topic) {
                case MqttClient.TEMPERATURE_TOPIC:
                    updateTemperature(Float.parseFloat(payload));
                    break;
                case MqttClient.HUMIDITY_TOPIC:
                    updateHumidity(Float.parseFloat(payload));
                    break;
                case MqttClient.STATUS_TOPIC:
                    statusText.setText("Connected");
                    break;
                case MqttClient.DATA_TOPIC:
                    // Handle combined data if needed
                    break;
            }
        });
    }

    private void updateTemperature(float temperature) {
        temperatureText.setText(String.format("%.1f°C", temperature));
        temperatureEntries.add(new Entry(dataPointCount, temperature));
        if (temperatureEntries.size() > 50) {
            temperatureEntries.remove(0);
        }
        temperatureChart.getData().notifyDataChanged();
        temperatureChart.notifyDataSetChanged();
        temperatureChart.invalidate();
    }

    private void updateHumidity(float humidity) {
        humidityText.setText(String.format("%.1f%%", humidity));
        humidityEntries.add(new Entry(dataPointCount, humidity));
        if (humidityEntries.size() > 50) {
            humidityEntries.remove(0);
        }
        humidityChart.getData().notifyDataChanged();
        humidityChart.notifyDataSetChanged();
        humidityChart.invalidate();
        dataPointCount++;
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // Not needed for this implementation
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mqttClient != null) {
            mqttClient.disconnect();
        }
    }
} 