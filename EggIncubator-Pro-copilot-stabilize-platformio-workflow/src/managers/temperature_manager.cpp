#include "temperature_manager.h"

#include <math.h>
#include "sensor_manager.h"
#include "relay_controller.h"
#include "logger.h"
#include "pins.h"

/******************************************************
 * Static Member Initialization
 ******************************************************/

PIDController TemperatureManager::pidController(1.0f, 0.1f, 0.05f);

float TemperatureManager::targetTemperature = 37.5f;
float TemperatureManager::currentTemperature = 0.0f;

uint8_t TemperatureManager::heaterPin = PIN_HEATER_SSR;
uint8_t TemperatureManager::heaterPWM = 0;

bool TemperatureManager::controlEnabled = false;

float TemperatureManager::maxErrorForStable = 0.3f;
uint32_t TemperatureManager::stableCounterThreshold = 30;  // ~30 seconds at 1Hz update
uint32_t TemperatureManager::stableCounter = 0;

float TemperatureManager::heaterMinPWM = 50.0f;
float TemperatureManager::heaterMaxPWM = 255.0f;

/******************************************************
 * Initialize Temperature Manager
 ******************************************************/

void TemperatureManager::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Temperature Manager Started");
    Logger::info("--------------------------------");

    // Set default PID tunings for heating
    pidController.setTunings(2.5f, 0.15f, 0.8f);

    // Set output limits (-100 to 100 represents -full to +full)
    pidController.setOutputLimits(-100.0f, 100.0f);

    // Set integral anti-windup limits
    pidController.setIntegralLimits(-50.0f, 50.0f);

    // Set target temperature
    pidController.setSetpoint(targetTemperature);

    Logger::info("PID Tunings: Kp=2.5, Ki=0.15, Kd=0.8");
}

/******************************************************
 * Update - Run PID control loop
 ******************************************************/

void TemperatureManager::update()
{
    if (!controlEnabled)
    {
        return;
    }

    // Get current temperature from sensor
    currentTemperature = SensorManager::getAirTemperature();

    // Skip if sensor not ready or invalid reading
    if (currentTemperature <= 0 || !SensorManager::isSHT31Ready())
    {
        return;
    }

    // Calculate PID output
    float pidOutput = pidController.calculate(currentTemperature);

    // Apply heater control
    applyHeaterControl(pidOutput);

    // Track stability
    float error = pidController.getError();

    if (fabsf(error) < maxErrorForStable)
    {
        stableCounter++;
    }
    else
    {
        stableCounter = 0;
    }

    // Log debug info every 10 updates
    static uint32_t updateCount = 0;
    updateCount++;

    if (updateCount % 10 == 0)
    {
        Logger::debug(
            ("Temp: " +
             String(currentTemperature, 1) + "°C, " +
             "Target: " + String(targetTemperature, 1) + "°C, " +
             "Error: " + String(error, 2) + "°C, " +
             "PWM: " + String(heaterPWM)).c_str());
    }
}

/******************************************************
 * Set Target Temperature
 ******************************************************/

void TemperatureManager::setTargetTemperature(float target)
{
    targetTemperature = target;
    pidController.setSetpoint(target);

    stableCounter = 0;  // Reset stability counter

    Logger::info(
        ("Temperature Target Set: " + String(target, 1) + "°C").c_str());
}

/******************************************************
 * Get Current Temperature
 ******************************************************/

float TemperatureManager::getCurrentTemperature()
{
    return currentTemperature;
}

/******************************************************
 * Get Target Temperature
 ******************************************************/

float TemperatureManager::getTargetTemperature()
{
    return targetTemperature;
}

/******************************************************
 * Get Temperature Error
 ******************************************************/

float TemperatureManager::getTemperatureError()
{
    return pidController.getError();
}

/******************************************************
 * Set PID Tunings
 ******************************************************/

void TemperatureManager::setPIDTunings(float kp, float ki, float kd)
{
    pidController.setTunings(kp, ki, kd);

    Logger::info(
        ("PID Tunings Updated: Kp=" + String(kp, 2) +
         ", Ki=" + String(ki, 3) +
         ", Kd=" + String(kd, 2)).c_str());
}

/******************************************************
 * Get PID Values for Debugging
 ******************************************************/

float TemperatureManager::getPIDOutput()
{
    return pidController.getOutput();
}

float TemperatureManager::getPIDError()
{
    return pidController.getError();
}

float TemperatureManager::getPIDp()
{
    return pidController.getP();
}

float TemperatureManager::getPIDi()
{
    return pidController.getI();
}

float TemperatureManager::getPIDd()
{
    return pidController.getD();
}

/******************************************************
 * Enable Temperature Control
 ******************************************************/

void TemperatureManager::enable()
{
    if (!controlEnabled)
    {
        controlEnabled = true;
        pidController.reset();
        stableCounter = 0;

        Logger::info("Temperature Control ENABLED");
    }
}

/******************************************************
 * Disable Temperature Control
 ******************************************************/

void TemperatureManager::disable()
{
    if (controlEnabled)
    {
        controlEnabled = false;

        // API đúng của RelayController
        RelayController::heater(false);
        heaterPWM = 0;

        Logger::info("Temperature Control DISABLED");
    }
}

/******************************************************
 * Is Temperature Control Enabled
 ******************************************************/

bool TemperatureManager::isEnabled()
{
    return controlEnabled;
}

/******************************************************
 * Check if Temperature is Stable
 ******************************************************/

bool TemperatureManager::isStable()
{
    return (stableCounter >= stableCounterThreshold);
}

/******************************************************
 * Apply Heater Control
 ******************************************************/

void TemperatureManager::applyHeaterControl(float pidOutput)
{
    // Map PID output to PWM
    heaterPWM = mapPIDtoPWM(pidOutput);

    // Simple ON/OFF for relay output
    if (heaterPWM > 0)
    {
        RelayController::heater(true);
    }
    else
    {
        RelayController::heater(false);
    }
}

/******************************************************
 * Map PID output to PWM value
 * PID output range: -100 to +100
 * PWM range: 0 to 255
 * Positive output = turn on heater
 * Negative output = turn off heater
 ******************************************************/

uint8_t TemperatureManager::mapPIDtoPWM(float pidOutput)
{
    // Clamp output to valid range
    pidOutput = constrain(pidOutput, -100.0f, 100.0f);

    // Only consider positive output (heating)
    if (pidOutput <= 0)
    {
        return 0;
    }

    // Map 0-100 to heaterMinPWM-heaterMaxPWM
    float pwmValue = heaterMinPWM + (pidOutput / 100.0f) *
                     (heaterMaxPWM - heaterMinPWM);

    return (uint8_t)constrain(pwmValue, heaterMinPWM, heaterMaxPWM);
}