#include "relay_controller.h"

#include <Arduino.h>

#include "pins.h"
#include "system_state.h"

void RelayController::writeRelay(int pin, bool state)
{
    digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
}

void RelayController::begin()
{
    pinMode(PIN_HEATER_SSR, OUTPUT);
    pinMode(PIN_HUMIDIFIER, OUTPUT);
    pinMode(PIN_FAN_CIRCULATION, OUTPUT);
    pinMode(PIN_FAN_VENT, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    pinMode(PIN_MOTOR_LEFT, OUTPUT);
    pinMode(PIN_MOTOR_RIGHT, OUTPUT);
    pinMode(PIN_MOTOR_ENABLE, OUTPUT);

    allOff();
    buzzer(false);
    motor(false, false, false);
}

void RelayController::heater(bool state)
{
    systemState.output.heater = state;
    writeRelay(PIN_HEATER_SSR, state);
}

void RelayController::humidifier(bool state)
{
    systemState.output.humidifier = state;
    writeRelay(PIN_HUMIDIFIER, state);
}

void RelayController::fan(bool state)
{
    systemState.output.circulationFan = state;
    writeRelay(PIN_FAN_CIRCULATION, state);
}

void RelayController::ventilation(bool state)
{
    systemState.output.ventilationFan = state;
    writeRelay(PIN_FAN_VENT, state);
}

void RelayController::buzzer(bool state)
{
    systemState.output.buzzer = state;
    digitalWrite(PIN_BUZZER, state ? BUZZER_ON : BUZZER_OFF);
}

void RelayController::motor(bool enable, bool left, bool right)
{
    if (left && right)
    {
        left = false;
        right = false;
    }

    systemState.output.motorEnable = enable;
    systemState.output.motorLeft = left;
    systemState.output.motorRight = right;

    digitalWrite(PIN_MOTOR_ENABLE, enable ? HIGH : LOW);
    digitalWrite(PIN_MOTOR_LEFT, left ? HIGH : LOW);
    digitalWrite(PIN_MOTOR_RIGHT, right ? HIGH : LOW);
}

void RelayController::toggleHeater()
{
    heater(!systemState.output.heater);
}

void RelayController::toggleHumidifier()
{
    humidifier(!systemState.output.humidifier);
}

void RelayController::toggleFan()
{
    fan(!systemState.output.circulationFan);
}

void RelayController::toggleVentilation()
{
    ventilation(!systemState.output.ventilationFan);
}

void RelayController::allOff()
{
    heater(false);
    humidifier(false);
    fan(false);
    ventilation(false);
}

bool RelayController::heaterState() { return systemState.output.heater; }
bool RelayController::humidifierState() { return systemState.output.humidifier; }
bool RelayController::fanState() { return systemState.output.circulationFan; }
bool RelayController::ventilationState() { return systemState.output.ventilationFan; }
