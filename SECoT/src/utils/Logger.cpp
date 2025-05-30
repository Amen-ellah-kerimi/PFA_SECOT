/*
 * Logger.cpp
 * 
 * Implementation of the Logger class
 */

#include "Logger.h"

Logger::Logger() {
  logIndex = 0;
  logCount = 0;
}

void Logger::begin() {
  // Nothing to initialize
}

void Logger::log(uint8_t level, const String& module, const String& message) {
  // Only log if level is at or above the current log level
  if (level >= CURRENT_LOG_LEVEL) {
    // Create log entry
    LogEntry entry;
    entry.timestamp = millis();
    entry.level = level;
    entry.module = module;
    entry.message = message;
    
    // Store in circular buffer
    logBuffer[logIndex] = entry;
    
    // Update index and count
    logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
    if (logCount < MAX_LOG_ENTRIES) {
      logCount++;
    }
    
    // Print to serial
    Serial.print("[");
    Serial.print(entry.timestamp);
    Serial.print("] [");
    Serial.print(levelToString(level));
    Serial.print("] [");
    Serial.print(module);
    Serial.print("] ");
    Serial.println(message);
  }
}

LogEntry* Logger::getRecentLogs(int count, int& actualCount) {
  // Limit count to available logs
  if (count > logCount) {
    count = logCount;
  }
  
  // Allocate memory for logs (caller must free this)
  LogEntry* logs = new LogEntry[count];
  
  // Copy the most recent logs
  for (int i = 0; i < count; i++) {
    int index = (logIndex - 1 - i + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
    logs[i] = logBuffer[index];
  }
  
  // Return actual count
  actualCount = count;
  
  return logs;
}

void Logger::clearLogs() {
  logIndex = 0;
  logCount = 0;
}

String Logger::levelToString(uint8_t level) {
  switch (level) {
    case LOG_DEBUG:
      return "DEBUG";
    case LOG_INFO:
      return "INFO";
    case LOG_WARNING:
      return "WARNING";
    case LOG_ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
