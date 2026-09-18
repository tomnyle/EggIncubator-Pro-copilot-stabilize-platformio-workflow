#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>

class TemperatureManager
{
public:
    static void begin();
    static void update();

    static void setTargetTemperature(float target);
    static float getCurrentTemperature();
    static float getTargetTemperature();

    static void setHysteresis(float hysteresisC);
    static void setMinSwitchMs(uint32_t minIntervalMs);

    static void enable();
    static void disable();
    static bool isEnabled();
    static bool isStable();

private:
    static float targetTemperature;
    static float currentTemperature;
    static float hysteresis;
    static uint32_t minSwitchMs;
    static uint32_t lastSwitchMs;
    static bool controlEnabled;
    static bool heaterRunning;
    static uint16_t stableCounter;
};

#endif
