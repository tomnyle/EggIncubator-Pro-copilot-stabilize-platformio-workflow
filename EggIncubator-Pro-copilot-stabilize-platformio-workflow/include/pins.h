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
#ifndef PIN_DS18B20
// Change this to the GPIO routed to the PCB's DS18B20 DQ connector.
// The 1-Wire bus must not share the SHT31 I2C SDA pin.
#define PIN_DS18B20              16
#endif

/**********************************************************************
 * OUTPUT DEVICES
 *********************************************************************/
#define PIN_HEATER_SSR           25
#define PIN_HUMIDIFIER           26
#define PIN_FAN_CIRCULATION      27
#define PIN_FAN_VENT             14

/**********************************************************************
 * DM542 MOTOR DRIVER (Egg Turner)
 * The DM542 receives pulse, direction and enable signals.
 *********************************************************************/
#define PIN_DM542_STEP           32
#define PIN_DM542_DIR            33
#define PIN_DM542_ENABLE         23
#define DM542_STEP_INTERVAL_US   1000
#define DM542_ENABLE_ACTIVE      HIGH
#define DM542_ENABLE_INACTIVE    LOW

// Compatibility aliases for code that still refers to motor control pins.
#define PIN_BTS_RPWM             PIN_DM542_STEP
#define PIN_BTS_LPWM             PIN_DM542_DIR
#define PIN_BTS_ENABLE           PIN_DM542_ENABLE
#define PIN_MOTOR_LEFT           PIN_DM542_STEP
#define PIN_MOTOR_RIGHT          PIN_DM542_DIR
#define PIN_MOTOR_ENABLE         PIN_DM542_ENABLE

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
 * Relay outputs drive the ULN2003A inputs. HIGH enables the sink output.
 *********************************************************************/
#define RELAY_ON                 HIGH
#define RELAY_OFF                LOW

/**********************************************************************
 * OPTIONAL GENERIC STATES
 *********************************************************************/
#define DEVICE_ON                HIGH
#define DEVICE_OFF               LOW

#endif // PINS_H