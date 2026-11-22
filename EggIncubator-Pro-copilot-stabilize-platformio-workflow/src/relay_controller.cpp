#include "relay_controller.h"

#include <Arduino.h>

#include "pins.h"
#include "system_state.h"
#include "app_config.h"

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
    if (!REV3_ALLOW_THERMAL_OUTPUTS && state)
    {
        Serial.println("[SAFETY] Heater disabled in REV3 test build");
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
    if (!REV3_ALLOW_THERMAL_OUTPUTS && state)
    {
        Serial.println("[SAFETY] Humidifier disabled in REV3 test build");
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