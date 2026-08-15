#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum LogLevel
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG
};


class Logger
{
public:

    static void begin(uint32_t baud);

    static void info(const char *message);

    static void warning(const char *message);

    static void error(const char *message);

    static void debug(const char *message);


private:

    static void print(LogLevel level, const char *message);

};


#endif