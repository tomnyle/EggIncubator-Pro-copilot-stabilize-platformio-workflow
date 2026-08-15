#include "humidity_manager.h"

#include <math.h>
#include "sensor_manager.h"
#include "relay_controller.h"
#include "logger.h"

/******************************************************
 * Static Member Initialization
 ******************************************************/

float HumidityManager::targetHumidity = 60.0f;
float HumidityManager::currentHumidity = 0.0f;

float HumidityManager::humidityHysteresis = 5.0f;

bool HumidityManager::controlEnabled = false;
bool HumidityManager::humidifierRunning = false;

float HumidityManager::maxErrorForStable = 2.0f;
uint32_t HumidityManager::stableCounterThreshold = 60;  // ~60 seconds at 1Hz update
uint32_t HumidityManager::stableCounter = 0;

/******************************************************
 * Initialize Humidity Manager
 ******************************************************/

void HumidityManager::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Humidity Manager Started");
    Logger::info("--------------------------------");

    Logger::info(("Target Humidity: " + String(targetHumidity, 1) + "%").c_str());
    Logger::info(("Hysteresis: ±" + String(humidityHysteresis, 1) + "%").c_str());
}

/******************************************************
 * Update - Control humidity
 ******************************************************/

void HumidityManager::update()
{
    if (!controlEnabled)
    {
        return;
    }

    currentHumidity = SensorManager::getAirHumidity();

    if (currentHumidity < 0 || !SensorManager::isSHT31Ready())
    {
        return;
    }

    const float error = targetHumidity - currentHumidity;
    applyHumidifierControl(error);

    if (fabsf(error) < maxErrorForStable)
    {
        stableCounter++;
    }
    else
    {
        stableCounter = 0;
    }

    static uint32_t updateCount = 0;
    updateCount++;

    if (updateCount % 10 == 0)
    {
        Logger::debug(
            ("Humidity: " +
             String(currentHumidity, 1) + "%, " +
             "Target: " + String(targetHumidity, 1) + "%, " +
             "Error: " + String(error, 1) + "%, " +
             "Humidifier: " + String(humidifierRunning ? "ON" : "OFF")).c_str());
    }
}

/******************************************************
 * Set Target Humidity
 ******************************************************/

void HumidityManager::setTargetHumidity(float target)
{
    targetHumidity = target;
    stableCounter = 0;
    Logger::info(("Humidity Target Set: " + String(target, 1) + "%").c_str());
}

/******************************************************
 * Getters / Setters
 ******************************************************/

float HumidityManager::getCurrentHumidity() { return currentHumidity; }
float HumidityManager::getTargetHumidity() { return targetHumidity; }
float HumidityManager::getHumidityError() { return (targetHumidity - currentHumidity); }

void HumidityManager::setHysteresis(float hysteresis)
{
    humidityHysteresis = hysteresis;
    Logger::info(("Humidity Hysteresis Set: ±" + String(hysteresis, 1) + "%").c_str());
}

/******************************************************
 * Enable / Disable
 ******************************************************/

void HumidityManager::enable()
{
    if (!controlEnabled)
    {
        controlEnabled = true;
        stableCounter = 0;
        Logger::info("Humidity Control ENABLED");
    }
}

void HumidityManager::disable()
{
    if (controlEnabled)
    {
        controlEnabled = false;
        RelayController::humidifier(false);
        humidifierRunning = false;
        Logger::info("Humidity Control DISABLED");
    }
}

bool HumidityManager::isEnabled() { return controlEnabled; }
bool HumidityManager::isStable() { return (stableCounter >= stableCounterThreshold); }
bool HumidityManager::isHumidifierRunning() { return humidifierRunning; }

/******************************************************
 * Apply Humidifier Control using Hysteresis
 ******************************************************/

void HumidityManager::applyHumidifierControl(float error)
{
    const float onThreshold = humidityHysteresis;
    const float offThreshold = -humidityHysteresis;

    if (humidifierRunning)
    {
        if (error < offThreshold)
        {
            RelayController::humidifier(false);
            humidifierRunning = false;
            Logger::info("Humidifier OFF (Upper threshold reached)");
        }
    }
    else
    {
        if (error > onThreshold)
        {
            RelayController::humidifier(true);
            humidifierRunning = true;
            Logger::info("Humidifier ON (Lower threshold reached)");
        }
    }
}