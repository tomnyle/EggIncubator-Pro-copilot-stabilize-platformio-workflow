#include "logger.h"

#include <stdarg.h>

Logger::LogLevel Logger::currentLogLevel = Logger::INFO;
bool Logger::consoleEnabled = true;
uint32_t Logger::startTime = 0;

void Logger::begin(unsigned int baudRate)
{
    Serial.begin(baudRate);
    delay(100);
    begin();
}

void Logger::begin()
{
    startTime = millis();
    currentLogLevel = INFO;
    consoleEnabled = true;
}

void Logger::setLogLevel(LogLevel level)
{
    currentLogLevel = level;
}

Logger::LogLevel Logger::getLogLevel()
{
    return currentLogLevel;
}

void Logger::error(const char* message)
{
    if (currentLogLevel >= ERROR) logMessage(ERROR, message);
}

void Logger::error(const String& message)
{
    error(message.c_str());
}

void Logger::warn(const char* message)
{
    if (currentLogLevel >= WARN) logMessage(WARN, message);
}

void Logger::warn(const String& message)
{
    warn(message.c_str());
}

void Logger::warning(const char* message)
{
    warn(message);
}

void Logger::warning(const String& message)
{
    warn(message.c_str());
}

void Logger::info(const char* message)
{
    if (currentLogLevel >= INFO) logMessage(INFO, message);
}

void Logger::info(const String& message)
{
    info(message.c_str());
}

void Logger::debug(const char* message)
{
    if (currentLogLevel >= DEBUG) logMessage(DEBUG, message);
}

void Logger::debug(const String& message)
{
    debug(message.c_str());
}

void Logger::printf(const char* format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    info(buffer);
}

void Logger::printTimestamp()
{
    Serial.print(getTimestamp());
}

String Logger::getTimestamp()
{
    const uint32_t uptime = getUptimeSeconds();
    const uint32_t hours = uptime / 3600;
    const uint32_t minutes = (uptime % 3600) / 60;
    const uint32_t seconds = uptime % 60;

    char timeBuffer[16];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02lu:%02lu:%02lu",
             static_cast<unsigned long>(hours),
             static_cast<unsigned long>(minutes),
             static_cast<unsigned long>(seconds));
    return String(timeBuffer);
}

uint32_t Logger::getUptimeSeconds()
{
    return (millis() - startTime) / 1000;
}

void Logger::enableConsole()
{
    consoleEnabled = true;
}

void Logger::disableConsole()
{
    consoleEnabled = false;
}

bool Logger::isConsoleEnabled()
{
    return consoleEnabled;
}

void Logger::logMessage(LogLevel level, const char* message)
{
    if (!consoleEnabled)
    {
        return;
    }

    Serial.print("[");
    Serial.print(getTimestamp());
    Serial.print("] [");
    Serial.print(getLevelName(level));
    Serial.print("] ");
    Serial.println(message);
}

const char* Logger::getLevelName(LogLevel level)
{
    switch (level)
    {
        case ERROR:
            return "ERROR";
        case WARN:
            return "WARN";
        case INFO:
            return "INFO";
        case DEBUG:
            return "DEBUG";
        case OFF:
        default:
            return "OFF";
    }
}
