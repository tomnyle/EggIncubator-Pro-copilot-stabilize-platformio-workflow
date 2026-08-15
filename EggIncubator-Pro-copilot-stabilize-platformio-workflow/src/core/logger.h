#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/******************************************************
 * Logger - System for logging messages to Serial
 * Supports different log levels for debugging
 ******************************************************/

class Logger
{
public:
    /******************************************************
     * Log Levels
     ******************************************************/
    enum LogLevel
    {
        OFF = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4
    };

    /******************************************************
     * Initialize Logger
     ******************************************************/
    static void begin();
    static void begin(unsigned int baudRate);  // thêm để khớp main.cpp

    /******************************************************
     * Set Log Level (0-4)
     ******************************************************/
    static void setLogLevel(LogLevel level);

    /******************************************************
     * Get Current Log Level
     ******************************************************/
    static LogLevel getLogLevel();

    /******************************************************
     * Log Error Message
     ******************************************************/
    static void error(const char* message);
    static void error(const String& message);

    /******************************************************
     * Log Warning Message
     ******************************************************/
    static void warn(const char* message);
    static void warn(const String& message);

    // Alias tương thích code cũ đang gọi warning(...)
    static void warning(const char* message);
    static void warning(const String& message);

    /******************************************************
     * Log Info Message
     ******************************************************/
    static void info(const char* message);
    static void info(const String& message);

    /******************************************************
     * Log Debug Message
     ******************************************************/
    static void debug(const char* message);
    static void debug(const String& message);

    /******************************************************
     * Log with Format (printf-style)
     ******************************************************/
    static void printf(const char* format, ...);

    /******************************************************
     * Print timestamp only
     ******************************************************/
    static void printTimestamp();

    /******************************************************
     * Get timestamp as string
     ******************************************************/
    static String getTimestamp();

    /******************************************************
     * Get uptime in seconds
     ******************************************************/
    static uint32_t getUptimeSeconds();

    /******************************************************
     * Enable/Disable console output
     ******************************************************/
    static void enableConsole();
    static void disableConsole();
    static bool isConsoleEnabled();

private:
    /******************************************************
     * Internal log message handler
     ******************************************************/
    static void logMessage(LogLevel level, const char* message);

    /******************************************************
     * Get log level name string
     ******************************************************/
    static const char* getLevelName(LogLevel level);

    /******************************************************
     * Static member variables
     ******************************************************/
    static LogLevel currentLogLevel;
    static bool consoleEnabled;
    static uint32_t startTime;
};

#endif