/*
 * Config.h
 *
 * Configuration settings for the SECoT toolkit
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== HARDWARE CONFIGURATION =====

// Uncomment to enable TFT display support
#define ENABLE_TFT

// Uncomment to enable web interface
#define ENABLE_WEB_INTERFACE

// TFT display pins (if enabled)
#ifdef ENABLE_TFT
  #define TFT_CS    5
  #define TFT_DC    4
  #define TFT_MOSI  23
  #define TFT_CLK   18
  #define TFT_RST   22
  #define TFT_MISO  19
  #define TFT_LED   15  // Optional backlight control
#endif

// ===== ATTACK CONFIGURATION =====

// Maximum number of targets to track
#define MAX_TARGETS 20

// Default attack intervals (ms)
#define DEFAULT_DEAUTH_INTERVAL 1000
#define DEFAULT_ARP_SPOOF_INTERVAL 10000
#define DEFAULT_MQTT_SPOOF_INTERVAL 5000

// Default attack durations (ms, 0 = run until stopped)
#define DEFAULT_ATTACK_DURATION 0

// ===== NETWORK CONFIGURATION =====

// Default scan settings
#define DEFAULT_SCAN_TIME 10000  // 10 seconds

// Network scan types
#define NETWORK_TYPE_WIFI 0
#define NETWORK_TYPE_MQTT 1
#define NETWORK_TYPE_BLE 2

// WiFi settings for access point mode
#define AP_SSID "SECoT_AP"
#define AP_PASSWORD "securitytool"
#define AP_CHANNEL 1

// ===== LOGGING CONFIGURATION =====

// Log levels
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_ERROR 3

// Current log level (only logs >= this level will be shown)
#define CURRENT_LOG_LEVEL LOG_INFO

// Maximum log buffer size
#define MAX_LOG_ENTRIES 100

// ===== MEMORY CONFIGURATION =====

// Memory limits to prevent crashes
#define MAX_PACKET_BUFFER 1024
#define MAX_SSID_LENGTH 32
#define MAX_BSSID_LENGTH 6

// ===== FEATURE FLAGS =====

// Uncomment to enable experimental features
// #define ENABLE_EXPERIMENTAL

// Uncomment to enable Bluetooth features
// #define ENABLE_BLUETOOTH

// Uncomment to enable 2.4GHz jamming (use with caution!)
// #define ENABLE_JAMMING

// ===== INTERFACE CONFIGURATION =====

// Serial interface settings
#define SERIAL_BAUD_RATE 115200
#define SERIAL_COMMAND_BUFFER 256
#define SERIAL_MAX_ARGS 10

// Web interface settings
#ifdef ENABLE_WEB_INTERFACE
  #define WEB_SERVER_PORT 80
  #define WEB_MAX_CONNECTIONS 5
#endif

#endif // CONFIG_H
