#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

/**********************************************************************
 * EggIncubator Pro
 * Hardware Pin Definition
 * Version : 1.0.0
 *
 * NOTE:
 * - ESP32 GPIO34-39 are input-only, avoid using them for OUTPUT.
 * - Keep boot-strapping pins safe at boot (GPIO0, 2, 12, 15).
 *********************************************************************/

/**********************************************************************
 * I2C BUS
 *********************************************************************/
#define PIN_I2C_SDA              21
#define PIN_I2C_SCL              22

/**********************************************************************
 * OUTPUT DEVICES
 *********************************************************************/
#define PIN_HEATER_SSR           25
#define PIN_HUMIDIFIER           26
#define PIN_FAN_CIRCULATION      27
#define PIN_FAN_VENT             14

/**********************************************************************
 * BTS7960 MOTOR DRIVER (Egg Turner)
 * RPWM/LPWM used as direction control in current codebase.
 *********************************************************************/
#define PIN_BTS_RPWM             32
#define PIN_BTS_LPWM             33
#define PIN_BTS_ENABLE           23

// Aliases for compatibility with EggTurnerManager
#define PIN_MOTOR_LEFT           PIN_BTS_LPWM
#define PIN_MOTOR_RIGHT          PIN_BTS_RPWM
#define PIN_MOTOR_ENABLE         PIN_BTS_ENABLE

/**********************************************************************
 * LIMIT SWITCH
 *********************************************************************/
#define PIN_LIMIT_HOME           18
#define PIN_LIMIT_END            19

/**********************************************************************
 * SAFETY / STATUS
 *********************************************************************/
#define PIN_BUZZER               4
#define PIN_DOOR_SWITCH          13
#define PIN_STATUS_LED           2

/**********************************************************************
 * LOGIC LEVEL DEFINITIONS
 *
 * Relay active level:
 * - Active HIGH module: RELAY_ON = HIGH, RELAY_OFF = LOW
 * - Active LOW module : RELAY_ON = LOW,  RELAY_OFF = HIGH
 *********************************************************************/
#define RELAY_ON                 LOW
#define RELAY_OFF                HIGH

/**********************************************************************
 * OPTIONAL GENERIC STATES
 *********************************************************************/
#define DEVICE_ON                HIGH
#define DEVICE_OFF               LOW

#endif // PINS_H