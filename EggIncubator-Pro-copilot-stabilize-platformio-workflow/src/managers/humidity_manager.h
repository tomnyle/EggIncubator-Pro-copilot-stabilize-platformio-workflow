#ifndef HUMIDITY_MANAGER_H
#define HUMIDITY_MANAGER_H

#include <Arduino.h>

/******************************************************
 * Humidity Manager - Controls humidifier for optimal
 * egg incubation humidity levels
 ******************************************************/

class HumidityManager
{
public:

    /******************************************************
     * Initialize Humidity Manager
     ******************************************************/
    static void begin();

    /******************************************************
     * Update - Check humidity and control humidifier
     ******************************************************/
    static void update();

    /******************************************************
     * Set Target Humidity
     ******************************************************/
    static void setTargetHumidity(float target);

    /******************************************************
     * Get Current Humidity
     ******************************************************/
    static float getCurrentHumidity();

    /******************************************************
     * Get Target Humidity
     ******************************************************/
    static float getTargetHumidity();

    /******************************************************
     * Get Humidity Error/Difference
     ******************************************************/
    static float getHumidityError();

    /******************************************************
     * Set Hysteresis for on/off control
     * (Prevents relay chattering)
     ******************************************************/
    static void setHysteresis(float hysteresis);

    /******************************************************
     * Enable/Disable Humidity Control
     ******************************************************/
    static void enable();
    static void disable();
    static bool isEnabled();

    /******************************************************
     * Check if humidity is stable
     ******************************************************/
    static bool isStable();

    /******************************************************
     * Get humidifier state
     ******************************************************/
    static bool isHumidifierRunning();

private:

    // Control parameters
    static float targetHumidity;
    static float currentHumidity;

    // Hysteresis control (prevents relay chattering)
    static float humidityHysteresis;

    // Control flags
    static bool controlEnabled;
    static bool humidifierRunning;

    // Stability tracking
    static float maxErrorForStable;
    static uint32_t stableCounterThreshold;
    static uint32_t stableCounter;

    /******************************************************
     * Apply humidifier control based on humidity error
     ******************************************************/
    static void applyHumidifierControl(float error);
};

#endif
