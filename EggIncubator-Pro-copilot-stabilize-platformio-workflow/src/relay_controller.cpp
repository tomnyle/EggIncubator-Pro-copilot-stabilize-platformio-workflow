#include "relay_controller.h"

#include <Arduino.h>
#include <math.h>

#include "pins.h"
#include "system_state.h"

namespace
{
constexpr float heaterCutoffTemperature = 39.5f;
constexpr float heaterRecoveryTemperature = 38.5f;
bool heaterBlocked = true;
bool heaterRejectionLogged = false;
bool simulatedHeaterRequest = false;
bool simulatedHumidifierRequest = false;
bool simulatedFanRequest = false;
bool simulatedVentilationRequest = false;

void logSimulationRequest(const char *name, bool state, bool &previousState)
{
    if (state != previousState)
    {
        Serial.printf("[SIMULATION] %s request %s; hardware held OFF\n",
                      name, state ? "ON" : "OFF");
        previousState = state;
    }
}
}

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

bool RelayController::simulatedHeater() { return simulatedHeaterRequest; }
bool RelayController::simulatedHumidifier() { return simulatedHumidifierRequest; }
bool RelayController::simulatedFan() { return simulatedFanRequest; }
bool RelayController::simulatedVentilation() { return simulatedVentilationRequest; }
bool RelayController::heaterSafetyBlocked() { return heaterBlocked; }

void RelayController::updateHeaterSafety(float temperature, bool sensorValid)
{
    const bool valid = sensorValid && isfinite(temperature) &&
                       temperature >= -40.0f && temperature <= 125.0f;
    const bool wasBlocked = heaterBlocked;
    if (!valid || temperature >= heaterCutoffTemperature)
        heaterBlocked = true;
    else if (temperature <= heaterRecoveryTemperature)
        heaterBlocked = false;

    if (heaterBlocked && !wasBlocked)
    {
        Serial.println(valid ? "[SAFETY] Heater inhibited: temperature >= 39.5 C"
                             : "[SAFETY] Heater inhibited: invalid sensor");
        heater(false);
    }
    else if (!heaterBlocked && wasBlocked)
    {
        heaterRejectionLogged = false;
        Serial.println("[SAFETY] Heater inhibit cleared: valid temperature <= 38.5 C");
    }
}

/******************************************************
 * Heater
 ******************************************************/

void RelayController::heater(bool state)
{
    if (state && heaterBlocked)
    {
        if (!heaterRejectionLogged)
        {
            Serial.println("[SAFETY] Heater request rejected: overtemperature or invalid sensor");
            heaterRejectionLogged = true;
        }
        state = false;
    }
    logSimulationRequest("Heater", state, simulatedHeaterRequest);
    state = false;
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
    logSimulationRequest("Humidifier", state, simulatedHumidifierRequest);
    state = false;
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
    logSimulationRequest("Circulation fan", state, simulatedFanRequest);
    state = false;
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
    logSimulationRequest("Ventilation fan", state, simulatedVentilationRequest);
    state = false;
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