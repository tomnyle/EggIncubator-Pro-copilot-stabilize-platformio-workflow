#ifndef HUMIDITY_MANAGER_H
#define HUMIDITY_MANAGER_H

#include <Arduino.h>

class HumidityManager
{
public:
    static void begin();
    static void update();

    static void setTargetHumidity(float target);
    static float getCurrentHumidity();
    static float getTargetHumidity();
    static float getHumidityError();

    static void setHysteresis(float hysteresis);

    static void enable();
    static void disable();
    static bool isEnabled();
    static bool isStable();
    static bool isHumidifierRunning();

private:
    static float targetHumidity;
    static float currentHumidity;
    static float humidityHysteresis;

    static bool controlEnabled;
    static bool humidifierRunning;
    static uint32_t runStartMs;
    static uint32_t cooldownUntilMs;
    static uint16_t stableCounter;

    static const uint32_t MAX_RUNTIME_MS;
    static const uint32_t REST_INTERVAL_MS;

    static void applyHumidifierControl(float error);
};

#endif
