#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if __has_include("app_config.local.h")
#include "app_config.local.h"
#endif

// Experimental REV3 build: hazardous outputs remain disabled by default.
#ifndef REV3_ALLOW_THERMAL_OUTPUTS
#define REV3_ALLOW_THERMAL_OUTPUTS 0
#endif
#ifndef REV3_ALLOW_TURNER
#define REV3_ALLOW_TURNER 0
#endif

/********************************************************
 * EggIncubator Pro Configuration
 ********************************************************/


// Firmware
#define FIRMWARE_NAME        "EggIncubator Pro"
#define FIRMWARE_VERSION     "0.3.0-rev3-dev3"


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