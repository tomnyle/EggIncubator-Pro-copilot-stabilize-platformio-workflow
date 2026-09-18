#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/********************************************************
 * EggIncubator Pro Configuration
 ********************************************************/


// Firmware
#define FIRMWARE_NAME        "EggIncubator Pro"
#define FIRMWARE_VERSION     "0.3.0"


// Device
#define DEVICE_ID            "eggincubator01"


// WiFi
#define WIFI_SSID            "CHANGE_ME_WIFI_SSID"
#define WIFI_PASSWORD        "CHANGE_ME_WIFI_PASSWORD"


// MQTT
#define MQTT_HOST            "192.168.1.100"
#define MQTT_PORT            1883

#define MQTT_USERNAME        "CHANGE_ME_MQTT_USERNAME"
#define MQTT_PASSWORD        "CHANGE_ME_MQTT_PASSWORD"


// MQTT Topic Root
#define MQTT_ROOT_TOPIC      "eggincubator"

// Optional local override (not tracked)
#ifdef __has_include
#if __has_include("app_config.local.h")
#include "app_config.local.h"
#endif
#endif


#endif