/*
 * Logger.h
 * 
 * Logging utility for SECoT
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "../config/Config.h"

// Log entry structure
struct LogEntry {
  unsigned long timestamp;
  uint8_t level;
  String module;
  String message;
};

class Logger {
  public:
    Logger();
    
    // Initialize the logger
    void begin();
    
    // Log a message with specified level and module
    void log(uint8_t level, const String& module, const String& message);
    
    // Get the most recent log entries
    LogEntry* getRecentLogs(int count, int& actualCount);
    
    // Clear all logs
    void clearLogs();
    
  private:
    LogEntry logBuffer[MAX_LOG_ENTRIES];
    int logIndex;
    int logCount;
    
    // Convert log level to string
    String levelToString(uint8_t level);
};

#endif // LOGGER_H
