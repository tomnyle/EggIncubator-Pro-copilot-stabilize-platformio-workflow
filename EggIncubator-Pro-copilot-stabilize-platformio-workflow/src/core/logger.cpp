#include "logger.h"
#include <stdarg.h>

/******************************************************
 * Static Member Initialization
 ******************************************************/

Logger::LogLevel Logger::currentLogLevel = Logger::INFO;
bool Logger::consoleEnabled = true;
uint32_t Logger::startTime = 0;

/******************************************************
 * Initialize Logger
 ******************************************************/

// Overload có baudRate để khớp main.cpp
void Logger::begin(unsigned int baudRate)
{
    Serial.begin(baudRate);
    delay(100);
    begin(); // gọi hàm mặc định để tái sử dụng logic
}

void Logger::begin()
{
    startTime = millis();
    currentLogLevel = INFO;
    consoleEnabled = true;

    // Wait for Serial to be ready
    delay(100);

    Serial.println();
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║     Logger System Initialized          ║");
    Serial.println("╚════════════════════════════════════════╝");
}

/******************************************************
 * Set Log Level
 ******************************************************/

void Logger::setLogLevel(LogLevel level)
{
    currentLogLevel = level;
}

/******************************************************
 * Get Log Level
 ******************************************************/

Logger::LogLevel Logger::getLogLevel()
{
    return currentLogLevel;
}

/******************************************************
 * Log Error Message
 ******************************************************/

void Logger::error(const char* message)
{
    if (currentLogLevel >= ERROR)
    {
        logMessage(ERROR, message);
    }
}

void Logger::error(const String& message)
{
    error(message.c_str());
}

/******************************************************
 * Log Warning Message
 ******************************************************/

void Logger::warn(const char* message)
{
    if (currentLogLevel >= WARN)
    {
        logMessage(WARN, message);
    }
}

void Logger::warn(const String& message)
{
    warn(message.c_str());
}

/******************************************************
 * Backward-compatible alias (nếu code cũ gọi warning)
 ******************************************************/
void Logger::warning(const char* message)
{
    warn(message);
}

void Logger::warning(const String& message)
{
    warn(message);
}

/******************************************************
 * Log Info Message
 ******************************************************/

void Logger::info(const char* message)
{
    if (currentLogLevel >= INFO)
    {
        logMessage(INFO, message);
    }
}

void Logger::info(const String& message)
{
    info(message.c_str());
}

/******************************************************
 * Log Debug Message
 ******************************************************/

void Logger::debug(const char* message)
{
    if (currentLogLevel >= DEBUG)
    {
        logMessage(DEBUG, message);
    }
}

void Logger::debug(const String& message)
{
    debug(message.c_str());
}

/******************************************************
 * Log with Format (printf-style)
 ******************************************************/

void Logger::printf(const char* format, ...)
{
    if (currentLogLevel < INFO)
    {
        return;
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logMessage(INFO, buffer);
}

/******************************************************
 * Internal Log Message Handler
 ******************************************************/

void Logger::logMessage(LogLevel level, const char* message)
{
    if (!consoleEnabled)
    {
        return;
    }

    Serial.print("[");
    Serial.print(getTimestamp());
    Serial.print("] ");

    Serial.print("[");
    Serial.print(getLevelName(level));
    Serial.print("] ");

    Serial.println(message);
}

/******************************************************
 * Print Timestamp Only
 ******************************************************/

void Logger::printTimestamp()
{
    Serial.print(getTimestamp());
}

/******************************************************
 * Get Timestamp as String
 ******************************************************/

String Logger::getTimestamp()
{
    uint32_t uptime = getUptimeSeconds();
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;

    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02lu:%02lu:%02lu",
             (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);

    return String(timeBuffer);
}

/******************************************************
 * Get Uptime in Seconds
 ******************************************************/

uint32_t Logger::getUptimeSeconds()
{
    return (millis() - startTime) / 1000;
}

/******************************************************
 * Get Log Level Name String
 ******************************************************/

const char* Logger::getLevelName(LogLevel level)
{
    switch (level)
    {
        case ERROR: return "ERROR";
        case WARN:  return "WARN";
        case INFO:  return "INFO";
        case DEBUG: return "DEBUG";
        case OFF:
        default:    return "OFF";
    }
}

/******************************************************
 * Enable Console Output
 ******************************************************/

void Logger::enableConsole()
{
    consoleEnabled = true;
    info("Console output ENABLED");
}

/******************************************************
 * Disable Console Output
 ******************************************************/

void Logger::disableConsole()
{
    Serial.println("Console output DISABLED");
    consoleEnabled = false;
}

/******************************************************
 * Check if Console is Enabled
 ******************************************************/

bool Logger::isConsoleEnabled()
{
    return consoleEnabled;
}