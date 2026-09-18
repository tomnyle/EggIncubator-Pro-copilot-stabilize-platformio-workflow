#include "temperature_manager.h"

#include <math.h>
#include "sensor_manager.h"
#include "relay_controller.h"
#include "logger.h"
#include "system_state.h"

float TemperatureManager::targetTemperature = 37.5f;
float TemperatureManager::currentTemperature = 0.0f;
float TemperatureManager::hysteresis = 0.3f;
uint32_t TemperatureManager::minSwitchMs = 8000;
uint32_t TemperatureManager::lastSwitchMs = 0;
bool TemperatureManager::controlEnabled = false;
bool TemperatureManager::heaterRunning = false;
uint16_t TemperatureManager::stableCounter = 0;

void TemperatureManager::begin()
{
    targetTemperature = systemState.incubation.targetTemperature;
    controlEnabled = false;
    heaterRunning = false;
    RelayController::heater(false);
}

void TemperatureManager::update()
{
    currentTemperature = SensorManager::getAirTemperature();
    const uint32_t now = millis();

    const bool tempValid = SensorManager::isAirTemperatureValid();

    if (!controlEnabled || !tempValid)
    {
        if (heaterRunning)
        {
            heaterRunning = false;
            RelayController::heater(false);
            lastSwitchMs = now;
        }
        return;
    }

    if (currentTemperature >= systemState.incubation.overTemperatureLimit)
    {
        setFault(FAULT_OVER_TEMP, true, "Over temperature");
        heaterRunning = false;
        RelayController::heater(false);
        lastSwitchMs = now;
        return;
    }

    setFault(FAULT_OVER_TEMP, false);

    const float lowThreshold = targetTemperature - hysteresis;
    const float highThreshold = targetTemperature + hysteresis;

    if ((now - lastSwitchMs) >= minSwitchMs)
    {
        if (!heaterRunning && currentTemperature <= lowThreshold)
        {
            heaterRunning = true;
            RelayController::heater(true);
            lastSwitchMs = now;
        }
        else if (heaterRunning && currentTemperature >= highThreshold)
        {
            heaterRunning = false;
            RelayController::heater(false);
            lastSwitchMs = now;
        }
    }

    const float error = fabsf(targetTemperature - currentTemperature);
    if (error <= hysteresis)
    {
        if (stableCounter < 1000) stableCounter++;
    }
    else
    {
        stableCounter = 0;
    }
}

void TemperatureManager::setTargetTemperature(float target)
{
    targetTemperature = target;
    systemState.incubation.targetTemperature = target;
}

float TemperatureManager::getCurrentTemperature() { return currentTemperature; }
float TemperatureManager::getTargetTemperature() { return targetTemperature; }

void TemperatureManager::setHysteresis(float hysteresisC)
{
    hysteresis = (hysteresisC < 0.1f) ? 0.1f : hysteresisC;
}

void TemperatureManager::setMinSwitchMs(uint32_t minIntervalMs)
{
    minSwitchMs = (minIntervalMs < 1000) ? 1000 : minIntervalMs;
}

void TemperatureManager::enable()
{
    controlEnabled = true;
}

void TemperatureManager::disable()
{
    controlEnabled = false;
    heaterRunning = false;
    RelayController::heater(false);
}

bool TemperatureManager::isEnabled() { return controlEnabled; }
bool TemperatureManager::isStable() { return stableCounter > 15; }
