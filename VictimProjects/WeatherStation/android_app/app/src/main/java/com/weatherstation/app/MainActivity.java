package com.weatherstation.app;

import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.appcompat.widget.Toolbar;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallbackExtended;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private MqttClient mqttClient;
    private TextView temperatureText;
    private TextView humidityText;
    private TextView statusText;
    private LineChart temperatureChart;
    private LineChart humidityChart;
    private List<Entry> temperatureEntries;
    private List<Entry> humidityEntries;
    private int dataPointCount = 0;

    private static final String BROKER = "tcp://192.168.1.100:1883"; // Replace with your MQTT broker address
    private static final String CLIENT_ID = "WeatherStation_" + System.currentTimeMillis();
    private static final String USERNAME = ""; // Add if needed
    private static final String PASSWORD = ""; // Add if needed

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Set up toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // Initialize views
        temperatureText = findViewById(R.id.temperature_text);
        humidityText = findViewById(R.id.humidity_text);
        statusText = findViewById(R.id.status_text);
        temperatureChart = findViewById(R.id.temperature_chart);
        humidityChart = findViewById(R.id.humidity_chart);

        // Initialize data lists
        temperatureEntries = new ArrayList<>();
        humidityEntries = new ArrayList<>();

        // Set up charts
        setupCharts();

        // Initialize MQTT client
        mqttClient = new MqttClient(this, BROKER, CLIENT_ID, USERNAME, PASSWORD);
        mqttClient.setCallback(new MqttCallbackExtended() {
            @Override
            public void connectComplete(boolean reconnect, String serverURI) {
                runOnUiThread(() -> {
                    statusText.setText("Connected");
                    statusText.setTextColor(getColor(R.color.status_connected));
                });
            }

            @Override
            public void connectionLost(Throwable cause) {
                runOnUiThread(() -> {
                    statusText.setText("Disconnected");
                    statusText.setTextColor(getColor(R.color.status_disconnected));
                });
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) {
                String payload = new String(message.getPayload());
                runOnUiThread(() -> updateUI(topic, payload));
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                // Not used
            }
        });

        // Connect to MQTT broker
        try {
            mqttClient.connect();
        } catch (MqttException e) {
            e.printStackTrace();
            Toast.makeText(this, "Failed to connect to MQTT broker: " + e.getMessage(), Toast.LENGTH_LONG).show();
            statusText.setText("Connection Failed");
            statusText.setTextColor(getColor(R.color.status_disconnected));
        }
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
        }
        return super.onOptionsItemSelected(item);
    }

    private void toggleTheme() {
        int currentNightMode = AppCompatDelegate.getDefaultNightMode();
        int newNightMode = (currentNightMode == AppCompatDelegate.MODE_NIGHT_YES) ?
                AppCompatDelegate.MODE_NIGHT_NO : AppCompatDelegate.MODE_NIGHT_YES;
        AppCompatDelegate.setDefaultNightMode(newNightMode);
    }

    private void setupCharts() {
        // Temperature chart setup
        temperatureChart.getDescription().setEnabled(false);
        temperatureChart.setTouchEnabled(true);
        temperatureChart.setDragEnabled(true);
        temperatureChart.setScaleEnabled(true);
        temperatureChart.setPinchZoom(true);
        temperatureChart.setDrawGridBackground(false);
        temperatureChart.getAxisRight().setEnabled(false);
        temperatureChart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        temperatureChart.getXAxis().setDrawGridLines(false);
        temperatureChart.getAxisLeft().setDrawGridLines(true);
        temperatureChart.getAxisLeft().setGridColor(Color.LTGRAY);
        temperatureChart.getAxisLeft().setTextColor(getColor(R.color.temperature_chart));
        temperatureChart.getXAxis().setTextColor(getColor(R.color.temperature_chart));
        temperatureChart.getLegend().setEnabled(false);

        // Humidity chart setup
        humidityChart.getDescription().setEnabled(false);
        humidityChart.setTouchEnabled(true);
        humidityChart.setDragEnabled(true);
        humidityChart.setScaleEnabled(true);
        humidityChart.setPinchZoom(true);
        humidityChart.setDrawGridBackground(false);
        humidityChart.getAxisRight().setEnabled(false);
        humidityChart.getXAxis().setPosition(XAxis.XAxisPosition.BOTTOM);
        humidityChart.getXAxis().setDrawGridLines(false);
        humidityChart.getAxisLeft().setDrawGridLines(true);
        humidityChart.getAxisLeft().setGridColor(Color.LTGRAY);
        humidityChart.getAxisLeft().setTextColor(getColor(R.color.humidity_chart));
        humidityChart.getXAxis().setTextColor(getColor(R.color.humidity_chart));
        humidityChart.getLegend().setEnabled(false);
    }

    private void updateUI(String topic, String payload) {
        try {
            if (topic.equals("weather/temperature")) {
                float temperature = Float.parseFloat(payload);
                temperatureText.setText(String.format("%.1f°C", temperature));
                updateTemperatureChart(temperature);
            } else if (topic.equals("weather/humidity")) {
                float humidity = Float.parseFloat(payload);
                humidityText.setText(String.format("%.1f%%", humidity));
                updateHumidityChart(humidity);
            }
        } catch (NumberFormatException e) {
            e.printStackTrace();
        }
    }

    private void updateTemperatureChart(float temperature) {
        temperatureEntries.add(new Entry(dataPointCount, temperature));
        if (temperatureEntries.size() > 20) {
            temperatureEntries.remove(0);
        }

        LineDataSet dataSet = new LineDataSet(temperatureEntries, "Temperature");
        dataSet.setColor(getColor(R.color.temperature_chart));
        dataSet.setCircleColor(getColor(R.color.temperature_chart));
        dataSet.setDrawValues(false);
        dataSet.setMode(LineDataSet.Mode.CUBIC_BEZIER);

        LineData lineData = new LineData(dataSet);
        temperatureChart.setData(lineData);
        temperatureChart.invalidate();
    }

    private void updateHumidityChart(float humidity) {
        humidityEntries.add(new Entry(dataPointCount, humidity));
        if (humidityEntries.size() > 20) {
            humidityEntries.remove(0);
        }

        LineDataSet dataSet = new LineDataSet(humidityEntries, "Humidity");
        dataSet.setColor(getColor(R.color.humidity_chart));
        dataSet.setCircleColor(getColor(R.color.humidity_chart));
        dataSet.setDrawValues(false);
        dataSet.setMode(LineDataSet.Mode.CUBIC_BEZIER);

        LineData lineData = new LineData(dataSet);
        humidityChart.setData(lineData);
        humidityChart.invalidate();

        dataPointCount++;
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
} 