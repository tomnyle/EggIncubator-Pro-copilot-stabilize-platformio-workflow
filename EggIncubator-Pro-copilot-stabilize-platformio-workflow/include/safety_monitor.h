#pragma once

#include <Arduino.h>

class SafetyMonitor
{
public:
    static void begin();
    static void update();
    static bool clearAlarm();
    static bool isAlarmActive();
    static String alarmMessage();
};
