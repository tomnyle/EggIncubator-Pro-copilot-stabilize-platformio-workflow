#include "gpio_manager.h"
#include "pins.h"

void GPIOManager::begin()
{
    /**********************************************************
     * OUTPUT
     **********************************************************/

    pinMode(PIN_HEATER_SSR, OUTPUT);
    pinMode(PIN_HUMIDIFIER, OUTPUT);
    pinMode(PIN_FAN_CIRCULATION, OUTPUT);
    pinMode(PIN_FAN_VENT, OUTPUT);

    // BTS7960
    // RPWM và LPWM sẽ được cấu hình trong BTS7960::begin()
    pinMode(PIN_BTS_ENABLE, OUTPUT);

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

    digitalWrite(PIN_HEATER_SSR, LOW);
    digitalWrite(PIN_HUMIDIFIER, LOW);
    digitalWrite(PIN_FAN_CIRCULATION, LOW);
    digitalWrite(PIN_FAN_VENT, LOW);

    // Tắt driver motor khi khởi động
    digitalWrite(PIN_BTS_ENABLE, LOW);

    // Tắt còi
    digitalWrite(PIN_BUZZER, LOW);
}