#include "relay_controller.h"

#include <Arduino.h>

#include "pins.h"
#include "system_state.h"
#include "app_config.h"
#include "operating_mode.h"
#include "safety_monitor.h"

/******************************************************
 * Private
 ******************************************************/

void RelayController::writeRelay(
    int pin,
    bool state)
{
    digitalWrite(
        pin,
        state ? RELAY_ON : RELAY_OFF);
}

/******************************************************
 * Begin
 ******************************************************/

void RelayController::begin()
{
    allOff();
}

/******************************************************
 * Heater
 ******************************************************/

void RelayController::heater(bool state)
{
    if (state && (OperatingModeController::current() == OperatingMode::Off ||
                  !SafetyMonitor::canEnableThermalOutput()))
    {
        Serial.println("[SAFETY] Heater request rejected: mode or sensor safety");
        state = false;
    }
    systemState.relay.heater = state;

    writeRelay(
        PIN_HEATER_SSR,
        state);
}

/******************************************************
 * Humidifier
 ******************************************************/

void RelayController::humidifier(bool state)
{
    if (state && (OperatingModeController::current() == OperatingMode::Off ||
                  !SafetyMonitor::canEnableThermalOutput()))
    {
        Serial.println("[SAFETY] Humidifier request rejected: mode or sensor safety");
        state = false;
    }
    systemState.relay.humidifier = state;

    writeRelay(
        PIN_HUMIDIFIER,
        state);
}

/******************************************************
 * Fan
 ******************************************************/

void RelayController::fan(bool state)
{
    systemState.relay.circulationFan = state;

    writeRelay(
        PIN_FAN_CIRCULATION,
        state);
}

/******************************************************
 * Ventilation
 ******************************************************/

void RelayController::ventilation(bool state)
{
    systemState.relay.ventilationFan = state;

    writeRelay(
        PIN_FAN_VENT,
        state);
}

/******************************************************
 * Toggle
 ******************************************************/

void RelayController::toggleHeater()
{
    heater(!systemState.relay.heater);
}

void RelayController::toggleHumidifier()
{
    humidifier(!systemState.relay.humidifier);
}

void RelayController::toggleFan()
{
    fan(!systemState.relay.circulationFan);
}

void RelayController::toggleVentilation()
{
    ventilation(!systemState.relay.ventilationFan);
}

/******************************************************
 * All OFF
 ******************************************************/

void RelayController::allOff()
{
    heater(false);
    humidifier(false);
    fan(false);
    ventilation(false);
}