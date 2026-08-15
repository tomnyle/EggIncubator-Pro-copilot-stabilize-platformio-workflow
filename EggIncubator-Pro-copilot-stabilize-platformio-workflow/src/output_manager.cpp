#include "output_manager.h"
#include "relay_controller.h"

bool OutputManager::heaterState = false;
bool OutputManager::humidifierState = false;
bool OutputManager::fanState = false;
bool OutputManager::ventState = false;

void OutputManager::begin()
{
    stopAll();
}

void OutputManager::update()
{
    // Dành cho SafetyManager hoặc Auto Controller sau này
}

void OutputManager::stopAll()
{
    setHeater(false);
    setHumidifier(false);
    setFan(false);
    setVent(false);
}

void OutputManager::setHeater(bool state)
{
    heaterState = state;
    RelayController::heater(state);
}

void OutputManager::setHumidifier(bool state)
{
    humidifierState = state;
    RelayController::humidifier(state);
}

void OutputManager::setFan(bool state)
{
    fanState = state;
    RelayController::fan(state);
}

void OutputManager::setVent(bool state)
{
    ventState = state;
    RelayController::ventilation(state);
}

bool OutputManager::heater()
{
    return heaterState;
}

bool OutputManager::humidifier()
{
    return humidifierState;
}

bool OutputManager::fan()
{
    return fanState;
}

bool OutputManager::vent()
{
    return ventState;
}