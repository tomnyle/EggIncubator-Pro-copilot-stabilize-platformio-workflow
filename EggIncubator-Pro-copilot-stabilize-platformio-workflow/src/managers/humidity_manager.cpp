#include "humidity_manager.h"

#include <math.h>
#include "sensor_manager.h"
#include "relay_controller.h"
#include "logger.h"
#include "system_state.h"

float HumidityManager::targetHumidity = 60.0f;
float HumidityManager::currentHumidity = 0.0f;
float HumidityManager::humidityHysteresis = 4.0f;

bool HumidityManager::controlEnabled = false;
bool HumidityManager::humidifierRunning = false;
uint32_t HumidityManager::runStartMs = 0;
uint32_t HumidityManager::cooldownUntilMs = 0;
uint16_t HumidityManager::stableCounter = 0;

const uint32_t HumidityManager::MAX_RUNTIME_MS = 120000;
const uint32_t HumidityManager::REST_INTERVAL_MS = 45000;

void HumidityManager::begin()
{
    targetHumidity = systemState.incubation.targetHumidity;
    controlEnabled = false;
    humidifierRunning = false;
    RelayController::humidifier(false);
}

void HumidityManager::update()
{
    currentHumidity = SensorManager::getAirHumidity();
    const uint32_t now = millis();

    if (!controlEnabled || !SensorManager::isHumidityValid())
    {
        setFault(FAULT_HUMIDITY_FAIL, true, "Humidity sensor invalid or stale");
        RelayController::humidifier(false);
        humidifierRunning = false;
        return;
    }

    setFault(FAULT_HUMIDITY_FAIL, false);

    if (humidifierRunning && (now - runStartMs) >= MAX_RUNTIME_MS)
    {
        RelayController::humidifier(false);
        humidifierRunning = false;
        cooldownUntilMs = now + REST_INTERVAL_MS;
    }

    const float error = targetHumidity - currentHumidity;
    applyHumidifierControl(error);

    if (fabsf(error) <= humidityHysteresis)
    {
        if (stableCounter < 1000) stableCounter++;
    }
    else
    {
        stableCounter = 0;
    }
}

void HumidityManager::setTargetHumidity(float target)
{
    targetHumidity = target;
    systemState.incubation.targetHumidity = target;
}

float HumidityManager::getCurrentHumidity() { return currentHumidity; }
float HumidityManager::getTargetHumidity() { return targetHumidity; }
float HumidityManager::getHumidityError() { return targetHumidity - currentHumidity; }

void HumidityManager::setHysteresis(float hysteresis)
{
    humidityHysteresis = (hysteresis < 1.0f) ? 1.0f : hysteresis;
}

void HumidityManager::enable()
{
    controlEnabled = true;
    runStartMs = 0;
    cooldownUntilMs = 0;
}

void HumidityManager::disable()
{
    controlEnabled = false;
    RelayController::humidifier(false);
    humidifierRunning = false;
    runStartMs = 0;
    cooldownUntilMs = 0;
}

bool HumidityManager::isEnabled() { return controlEnabled; }
bool HumidityManager::isStable() { return stableCounter > 15; }
bool HumidityManager::isHumidifierRunning() { return humidifierRunning; }

void HumidityManager::applyHumidifierControl(float error)
{
    const uint32_t now = millis();
    const float onThreshold = humidityHysteresis;
    const float offThreshold = -humidityHysteresis;

    if (humidifierRunning)
    {
        if (error < offThreshold)
        {
            RelayController::humidifier(false);
            humidifierRunning = false;
            cooldownUntilMs = now + REST_INTERVAL_MS;
        }
    }
    else
    {
        if (now < cooldownUntilMs)
        {
            return;
        }

        if (error > onThreshold)
        {
            RelayController::humidifier(true);
            humidifierRunning = true;
            runStartMs = now;
        }
    }
}
