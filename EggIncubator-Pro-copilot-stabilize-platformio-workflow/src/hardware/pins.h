#ifndef PINS_H
#define PINS_H

/******************************************************
 * ESP32 Pin Definitions for EggIncubator-Pro
 ******************************************************/

// ========== SENSOR PINS ==========

// I2C Pins for SHT31 (Temperature & Humidity Sensor)
#define PIN_I2C_SDA             21      // GPIO21
#define PIN_I2C_SCL             22      // GPIO22
#define SHT31_I2C_ADDRESS       0x44    // Default I2C address

// ========== HEATING ELEMENT CONTROL ==========

// Heater SSR (Solid State Relay) - PWM capable
#define PIN_HEATER_SSR          15      // GPIO15 - PWM for heater

// ========== HUMIDIFIER CONTROL ==========

// Humidifier Relay - Digital output
#define PIN_HUMIDIFIER_RELAY    4       // GPIO4

// ========== EGG TURNER MOTOR CONTROL ==========

// Motor Direction Control (via L298N or similar H-Bridge)
#define PIN_MOTOR_LEFT          13      // GPIO13 - Motor Left direction
#define PIN_MOTOR_RIGHT         12      // GPIO12 - Motor Right direction

// Motor PWM Speed Control (optional, for variable speed)
#define PIN_MOTOR_PWM           14      // GPIO14 - PWM speed control

// ========== ADDITIONAL RELAYS ==========

// Fan Relay (for ventilation)
#define PIN_FAN_RELAY           2       // GPIO2

// Alarm/Buzzer (optional)
#define PIN_ALARM               32      // GPIO32

// Status LED
#define PIN_STATUS_LED          25      // GPIO25

// ========== BUTTON INPUTS ==========

// Manual Mode Button
#define PIN_BUTTON_MANUAL       35      // GPIO35 - Input only
#define PIN_BUTTON_POWER        34      // GPIO34 - Input only

// ========== UART COMMUNICATION ==========

// Serial 0 (USB) - Used for debugging
#define UART0_RX                3       // RX0
#define UART0_TX                1       // TX0

// UART2 - For additional serial communication (optional)
#define PIN_UART2_RX            16      // GPIO16
#define PIN_UART2_TX            17      // GPIO17

// ========== SPI PINS (IF NEEDED) ==========

#define PIN_SPI_MOSI            23      // GPIO23
#define PIN_SPI_MISO            19      // GPIO19
#define PIN_SPI_CLK             18      // GPIO18
#define PIN_SPI_CS              5       // GPIO5

// ========== PWM CONFIGURATIONS ==========

// PWM Frequency settings
#define PWM_FREQUENCY_HEATER    1000    // 1 kHz for heater
#define PWM_FREQUENCY_MOTOR     2000    // 2 kHz for motor
#define PWM_RESOLUTION          8       // 8-bit resolution (0-255)

// PWM Channels (ESP32 has 16 channels)
#define PWM_CHANNEL_HEATER      0
#define PWM_CHANNEL_MOTOR       1

// ========== ADC PINS (IF NEEDED) ==========

#define PIN_ADC_POWER_SENSE     36      // GPIO36 (VP) - Power supply voltage
#define PIN_ADC_CURRENT_SENSE   39      // GPIO39 (VN) - Current monitoring

// ========== SYSTEM PARAMETERS ==========

// WiFi Configuration (optional)
#define ENABLE_WIFI             1
#define WIFI_SSID              "YourSSID"
#define WIFI_PASSWORD          "YourPassword"

// MQTT Configuration (optional)
#define ENABLE_MQTT             0
#define MQTT_BROKER            "192.168.1.100"
#define MQTT_PORT              1883

// Logging Level
#define LOG_LEVEL              3    // 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG

#endif
