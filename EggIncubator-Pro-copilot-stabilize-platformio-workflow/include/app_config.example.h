#ifndef APP_CONFIG_LOCAL_H
#define APP_CONFIG_LOCAL_H

// Copy this file to include/app_config.local.h and edit values.

#undef WIFI_SSID
#undef WIFI_PASSWORD
#undef MQTT_HOST
#undef MQTT_PORT
#undef MQTT_USERNAME
#undef MQTT_PASSWORD
#undef DEVICE_ID
#undef MQTT_ROOT_TOPIC

#define DEVICE_ID            "eggincubator01"
#define WIFI_SSID            "your_wifi_ssid"
#define WIFI_PASSWORD        "your_wifi_password"
#define MQTT_HOST            "192.168.1.100"
#define MQTT_PORT            1883
#define MQTT_USERNAME        "mqtt_user"
#define MQTT_PASSWORD        "mqtt_password"
#define MQTT_ROOT_TOPIC      "eggincubator"

#endif
