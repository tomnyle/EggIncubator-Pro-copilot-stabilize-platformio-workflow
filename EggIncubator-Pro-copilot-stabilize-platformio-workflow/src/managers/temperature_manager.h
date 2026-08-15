#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>
#include "../controllers/pid_controller.h"

/******************************************************
 * Temperature Manager - Controls heating/cooling
 * with PID controller for optimal egg incubation
 ******************************************************/

class TemperatureManager
{
public:

    /******************************************************
     * Initialize Temperature Manager
     ******************************************************/
    static void begin();

    /******************************************************
     * Update - Run PID calculation and control heater
     ******************************************************/
    static void update();

    /******************************************************
     * Set Target Temperature
     ******************************************************/
    static void setTargetTemperature(float target);

    /******************************************************
     * Get Current Temperature
     ******************************************************/
    static float getCurrentTemperature();

    /******************************************************
     * Get Target Temperature
     ******************************************************/
    static float getTargetTemperature();

    /******************************************************
     * Get Temperature Error
     ******************************************************/
    static float getTemperatureError();

    /******************************************************
     * PID Tuning
     ******************************************************/
    static void setPIDTunings(float kp, float ki, float kd);

    /******************************************************
     * Get PID Values for debugging
     ******************************************************/
    static float getPIDOutput();
    static float getPIDError();
    static float getPIDp();
    static float getPIDi();
    static float getPIDd();

    /******************************************************
     * Enable/Disable Temperature Control
     ******************************************************/
    static void enable();
    static void disable();
    static bool isEnabled();

    /******************************************************
     * Check if temperature is stable
     ******************************************************/
    static bool isStable();

private:

    // PID Controller instance
    static PIDController pidController;

    // Control parameters
    static float targetTemperature;
    static float currentTemperature;

    // Heater PWM parameters
    static uint8_t heaterPin;
    static uint8_t heaterPWM;

    // Control flags
    static bool controlEnabled;

    // Stability tracking
    static float maxErrorForStable;
    static uint32_t stableCounterThreshold;
    static uint32_t stableCounter;

    // PID output mapping
    static float heaterMinPWM;     // 0-255, typically 50 for safety
    static float heaterMaxPWM;     // 0-255, typically 255

    /******************************************************
     * Apply heater PWM based on PID output
     ******************************************************/
    static void applyHeaterControl(float pidOutput);

    /******************************************************
     * Map PID output (-100 to 100) to PWM (0-255)
     ******************************************************/
    static uint8_t mapPIDtoPWM(float pidOutput);
};

#endif
