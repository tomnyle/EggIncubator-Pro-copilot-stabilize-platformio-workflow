#include "discovery_service.h"

#include <ArduinoJson.h>

#include "app_config.h"
#include "mqtt_service.h"
#include "logger.h"

extern MQTTService mqtt;

/*****************************************************
 * Device Information
 *****************************************************/

static void addDevice(JsonDocument &doc)
{
    JsonObject dev = doc["dev"].to<JsonObject>();

    JsonArray ids = dev["ids"].to<JsonArray>();
    ids.add(DEVICE_ID);

    dev["name"] = "EggIncubator Pro";
    dev["manufacturer"] = "Le Danh";
    dev["model"] = "ESP32 Egg Incubator";
    dev["sw_version"] = "1.0.0";
}

/*****************************************************
 * Availability
 *****************************************************/

static void addAvailability(JsonDocument &doc)
{
    doc["availability_topic"] = MQTT_ROOT_TOPIC "/status";
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";
}

/*****************************************************
 * Publish Switch
 *****************************************************/

static void publishSwitch(
    const char *objectId,
    const char *name,
    const char *commandTopic,
    const char *stateTopic,
    const char *icon)
{
    JsonDocument doc;

    doc["name"] = name;

    doc["object_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["unique_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["command_topic"] = commandTopic;
    doc["state_topic"] = stateTopic;

    doc["payload_on"] = "ON";
    doc["payload_off"] = "OFF";

    doc["icon"] = icon;

    addAvailability(doc);
    addDevice(doc);

    char payload[1024];

    serializeJson(doc, payload);

    String topic =
        "homeassistant/switch/" +
        String(DEVICE_ID) +
        "/" +
        objectId +
        "/config";

    mqtt.publish(topic.c_str(), payload, true);

    Logger::info(("Discovery -> " + topic).c_str());
}

/*****************************************************
 * Publish Temperature Sensor
 *****************************************************/

static void publishTemperatureSensor(
    const char *objectId,
    const char *name,
    const char *stateTopic,
    const char *icon,
    const char *unit = "°C")
{
    JsonDocument doc;

    doc["name"] = name;

    doc["object_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["unique_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["state_topic"] = stateTopic;

    doc["unit_of_measurement"] = unit;

    doc["device_class"] = "temperature";

    doc["icon"] = icon;

    doc["value_template"] = "{{ value }}";

    addAvailability(doc);
    addDevice(doc);

    char payload[1024];

    serializeJson(doc, payload);

    String topic =
        "homeassistant/sensor/" +
        String(DEVICE_ID) +
        "/" +
        objectId +
        "/config";

    mqtt.publish(topic.c_str(), payload, true);

    Logger::info(("Discovery -> " + topic).c_str());
}

/*****************************************************
 * Publish Humidity Sensor
 *****************************************************/

static void publishHumiditySensor(
    const char *objectId,
    const char *name,
    const char *stateTopic,
    const char *icon = "mdi:water-percent")
{
    JsonDocument doc;

    doc["name"] = name;

    doc["object_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["unique_id"] =
        String(DEVICE_ID) + "_" + objectId;

    doc["state_topic"] = stateTopic;

    doc["unit_of_measurement"] = "%";

    doc["device_class"] = "humidity";

    doc["icon"] = icon;

    doc["value_template"] = "{{ value }}";

    addAvailability(doc);
    addDevice(doc);

    char payload[1024];

    serializeJson(doc, payload);

    String topic =
        "homeassistant/sensor/" +
        String(DEVICE_ID) +
        "/" +
        objectId +
        "/config";

    mqtt.publish(topic.c_str(), payload, true);

    Logger::info(("Discovery -> " + topic).c_str());
}

/*****************************************************
 * Publish Binary Sensor
 *****************************************************/

static void publishStatusSensor()
{
    JsonDocument doc;

    doc["name"] = "ESP32 Status";

    doc["object_id"] =
        String(DEVICE_ID) + "_status";

    doc["unique_id"] =
        String(DEVICE_ID) + "_status";

    doc["state_topic"] =
        String(MQTT_ROOT_TOPIC) + "/status";

    doc["icon"] = "mdi:lan-connect";

    addAvailability(doc);
    addDevice(doc);

    char payload[768];

    serializeJson(doc, payload);

    String topic =
        "homeassistant/binary_sensor/" +
        String(DEVICE_ID) +
        "/status/config";

    mqtt.publish(topic.c_str(), payload, true);

    Logger::info(("Discovery -> " + topic).c_str());
}

/*****************************************************
 * Publish Firmware Sensor
 *****************************************************/

static void publishFirmwareSensor()
{
    JsonDocument doc;

    doc["name"] = "Firmware";

    doc["object_id"] =
        String(DEVICE_ID) + "_firmware";

    doc["unique_id"] =
        String(DEVICE_ID) + "_firmware";

    doc["state_topic"] =
        String(MQTT_ROOT_TOPIC) + "/firmware";

    doc["entity_category"] = "diagnostic";

    doc["icon"] = "mdi:chip";

    addAvailability(doc);
    addDevice(doc);

    char payload[768];

    serializeJson(doc, payload);

    String topic =
        "homeassistant/sensor/" +
        String(DEVICE_ID) +
        "/firmware/config";

    mqtt.publish(topic.c_str(), payload, true);

    Logger::info(("Discovery -> " + topic).c_str());
}

/*****************************************************
 * Begin - Publish all entities
 *****************************************************/

void DiscoveryService::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Publishing MQTT Discovery");
    Logger::info("--------------------------------");

    /******* Device Status *******/
    publishStatusSensor();
    publishFirmwareSensor();

    /******* Relay Switches *******/
    publishSwitch(
        "heater",
        "Heater",
        MQTT_ROOT_TOPIC "/heater/set",
        MQTT_ROOT_TOPIC "/heater/state",
        "mdi:radiator");

    publishSwitch(
        "humidifier",
        "Humidifier",
        MQTT_ROOT_TOPIC "/humidifier/set",
        MQTT_ROOT_TOPIC "/humidifier/state",
        "mdi:air-humidifier");

    publishSwitch(
        "fan",
        "Circulation Fan",
        MQTT_ROOT_TOPIC "/fan/set",
        MQTT_ROOT_TOPIC "/fan/state",
        "mdi:fan");

    publishSwitch(
        "ventilation",
        "Ventilation Fan",
        MQTT_ROOT_TOPIC "/vent/set",
        MQTT_ROOT_TOPIC "/vent/state",
        "mdi:fan-chevron-up");

    /******* Sensor Entities *******/
    
    // Air Temperature (SHT31)
    publishTemperatureSensor(
        "air_temperature",
        "Air Temperature",
        MQTT_ROOT_TOPIC "/air_temperature",
        "mdi:thermometer");

    // Air Humidity (SHT31)
    publishHumiditySensor(
        "air_humidity",
        "Air Humidity",
        MQTT_ROOT_TOPIC "/air_humidity");

    // Egg Temperature (DS18B20)
    publishTemperatureSensor(
        "egg_temperature",
        "Egg Temperature",
        MQTT_ROOT_TOPIC "/egg_temperature",
        "mdi:egg");

    Logger::info("--------------------------------");
    Logger::info("Discovery Publish Completed");
    Logger::info("--------------------------------");
}
