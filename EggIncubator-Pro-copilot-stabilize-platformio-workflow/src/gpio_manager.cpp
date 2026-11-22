#include "gpio_manager.h"
#include "pins.h"

void GPIOManager::begin()
{
    /**********************************************************
     * OUTPUT
     **********************************************************/

    digitalWrite(PIN_HEATER_SSR, RELAY_OFF);
    digitalWrite(PIN_HUMIDIFIER, RELAY_OFF);
    digitalWrite(PIN_FAN_CIRCULATION, RELAY_OFF);
    digitalWrite(PIN_FAN_VENT, RELAY_OFF);
    pinMode(PIN_HEATER_SSR, OUTPUT);
    pinMode(PIN_HUMIDIFIER, OUTPUT);
    pinMode(PIN_FAN_CIRCULATION, OUTPUT);
    pinMode(PIN_FAN_VENT, OUTPUT);

    // DM542 STEP/DIR are configured by EggTurnerManager.
    digitalWrite(PIN_DM542_ENABLE, DM542_ENABLE_INACTIVE);
    pinMode(PIN_DM542_ENABLE, OUTPUT);

    digitalWrite(PIN_BUZZER, LOW);
    pinMode(PIN_BUZZER, OUTPUT);

    /**********************************************************
     * INPUT
     **********************************************************/

    pinMode(PIN_LIMIT_HOME, INPUT_PULLUP);
    pinMode(PIN_LIMIT_END, INPUT_PULLUP);
    pinMode(PIN_DOOR_SWITCH, INPUT_PULLUP);

    /**********************************************************
     * SAFE STATE
     **********************************************************/

    // Tắt driver motor khi khởi động
    digitalWrite(PIN_DM542_ENABLE, DM542_ENABLE_INACTIVE);

    // Tắt còi
    digitalWrite(PIN_BUZZER, LOW);
}