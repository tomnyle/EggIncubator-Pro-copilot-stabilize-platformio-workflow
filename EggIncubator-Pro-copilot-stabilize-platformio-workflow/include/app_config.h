#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/********************************************************
 * EggIncubator Pro Configuration
 ********************************************************/


// Firmware
#define FIRMWARE_NAME        "EggIncubator Pro"
#define FIRMWARE_VERSION     "0.3.0"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// Device
#define DEVICE_ID            "eggincubator01"


// WiFi
#ifndef WIFI_SSID
#define WIFI_SSID            ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD        ""
#endif


// MQTT
#ifndef MQTT_HOST
#define MQTT_HOST            ""
#endif
#ifndef MQTT_PORT
#define MQTT_PORT            1883
#endif
#ifndef MQTT_USERNAME
#define MQTT_USERNAME        ""
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD        ""
#endif


// MQTT Topic Root

#define MQTT_ROOT_TOPIC      "eggincubator"



#endif