#include "mqtt_service.h"

#include <WiFi.h>
#include "app_config.h"
#include "logger.h"
#include "sensor_manager.h"
#include "network_service.h"
#include "relay_controller.h"
#include "managers/egg_turner_manager.h"
#include "incubation_profile.h"

extern MQTTService mqtt;
extern NetworkService network;

static constexpr uint32_t RECONNECT_INTERVAL = 5000;
static constexpr uint32_t PUBLISH_INTERVAL   = 10000;

MQTTService::MQTTService()
    : mqttClient(wifiClient),
      mqttConnected(false),
      discoveryPublished(false),
      lastReconnect(0),
      lastPublish(0)
{
}

void MQTTService::begin()
{
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(15);
    mqttClient.setBufferSize(1024);
    mqttClient.setCallback(callback);

    mqttConnected = false;
    discoveryPublished = false;
    lastReconnect = 0;
    lastPublish = 0;

    Serial.println("[MQTT] Service Started");
    Serial.print("[MQTT] Broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(":");
    Serial.println(MQTT_PORT);
}

bool MQTTService::connected()
{
    return mqttClient.connected();
}

void MQTTService::loop()
{
    if (!network.connected())
    {
        mqttConnected = false;
        discoveryPublished = false;
        return;
    }

    if (!mqttClient.connected())
    {
        reconnect();
        return;
    }

    mqttClient.loop();

    if (millis() - lastPublish >= PUBLISH_INTERVAL)
    {
        lastPublish = millis();
        publishDiagnostics();
        publishRelayStates();
        publishSensorStates();
    }
}

void MQTTService::reconnect()
{
    if (millis() - lastReconnect < RECONNECT_INTERVAL) return;
    lastReconnect = millis();

    Serial.println("[MQTT] reconnecting...");

    bool ok = mqttClient.connect(
        DEVICE_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        MQTT_ROOT_TOPIC "/status",
        0,
        true,
        "offline"
    );

    if (!ok)
    {
        mqttConnected = false;
        Serial.print("[MQTT] Connect Failed, state=");
        Serial.println(mqttClient.state());
        return;
    }

    mqttConnected = true;
    Serial.println("[MQTT] Connected");

    publishBirthMessage();
    subscribeTopics();
    publishDiscovery();
    publishRelayStates();
    publishDiagnostics();
    publishSensorStates();
}

void MQTTService::subscribeTopics()
{
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/heater/set");
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/humidifier/set");
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/fan/set");
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/vent/set");
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/turner/set");
    mqttClient.subscribe(MQTT_ROOT_TOPIC "/profile/set");
    Serial.println("[MQTT] Subscribe OK");
}

void MQTTService::publishBirthMessage()
{
    mqttClient.publish(MQTT_ROOT_TOPIC "/status", "online", true);
    mqttClient.publish(MQTT_ROOT_TOPIC "/ip", WiFi.localIP().toString().c_str(), true);
    mqttClient.publish(MQTT_ROOT_TOPIC "/rssi", String(WiFi.RSSI()).c_str(), true);
#ifdef FIRMWARE_VERSION
    mqttClient.publish(MQTT_ROOT_TOPIC "/firmware", FIRMWARE_VERSION, true);
#endif
}

void MQTTService::publishDiscovery()
{
    if (discoveryPublished) return;

    const String deviceJson =
        "\"device\":{\"identifiers\":[\"" DEVICE_ID "\"],"
        "\"name\":\"Egg Incubator\","
        "\"manufacturer\":\"DIY\","
        "\"model\":\"EggIncubator Pro\","
        "\"sw_version\":\"" FIRMWARE_VERSION "\"}";

    publish("homeassistant/switch/" DEVICE_ID "/heater/config",
            "{"
            "\"name\":\"Heater\","
            "\"uniq_id\":\"" DEVICE_ID "_heater\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/heater/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/heater/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/humidifier/config",
            "{"
            "\"name\":\"Humidifier\","
            "\"uniq_id\":\"" DEVICE_ID "_humidifier\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/humidifier/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/humidifier/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/fan/config",
            "{"
            "\"name\":\"Circulation Fan\","
            "\"uniq_id\":\"" DEVICE_ID "_fan\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/fan/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/fan/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/vent/config",
            "{"
            "\"name\":\"Ventilation Fan\","
            "\"uniq_id\":\"" DEVICE_ID "_vent\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/vent/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/vent/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/turner/config",
            "{"
            "\"name\":\"Egg Turner\","
            "\"uniq_id\":\"" DEVICE_ID "_turner\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/turner/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/turner/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/select/" DEVICE_ID "/profile/config",
            "{"
            "\"name\":\"Incubation Profile\","
            "\"uniq_id\":\"" DEVICE_ID "_profile\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/profile/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/profile/state\","
            "\"options\":[\"CHICKEN\",\"QUAIL\",\"DUCK\",\"GOOSE\"],"
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/sensor/" DEVICE_ID "/incubation_day/config",
            "{"
            "\"name\":\"Incubation Day\","
            "\"uniq_id\":\"" DEVICE_ID "_incubation_day\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/incubation_day\","
            "\"icon\":\"mdi:calendar-clock\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/sensor/" DEVICE_ID "/stage/config",
            "{"
            "\"name\":\"Incubation Stage\","
            "\"uniq_id\":\"" DEVICE_ID "_stage\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/stage\","
            "\"icon\":\"mdi:egg-easter\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    discoveryPublished = true;
    Serial.println("[MQTT] Discovery published");
}

bool MQTTService::publish(const char *topic, const char *payload, bool retained)
{
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topic, payload, retained);
}

bool MQTTService::publish(const String &topic, const String &payload, bool retained)
{
    return publish(topic.c_str(), payload.c_str(), retained);
}

void MQTTService::publishAvailability()
{
    publish(MQTT_ROOT_TOPIC "/status", "online", true);
}

void MQTTService::publishRelayStates()
{
    // Vì RelayController chưa có getter state trong header bạn gửi,
    // tạm publish OFF cho 4 relay này để không phụ thuộc IncubatorController.
    // Khi bạn có getter thật, mình đổi lại sau.
    publish(MQTT_ROOT_TOPIC "/heater/state", "OFF", true);
    publish(MQTT_ROOT_TOPIC "/humidifier/state", "OFF", true);
    publish(MQTT_ROOT_TOPIC "/fan/state", "OFF", true);
    publish(MQTT_ROOT_TOPIC "/vent/state", "OFF", true);

    publish(MQTT_ROOT_TOPIC "/turner/state", EggTurnerManager::isEnabled() ? "ON" : "OFF", true);
    publish(MQTT_ROOT_TOPIC "/turn_count", String(EggTurnerManager::getTurnCount()), true);

    publish(MQTT_ROOT_TOPIC "/profile/state", IncubationProfile::currentCode(), true);
    publish(MQTT_ROOT_TOPIC "/incubation_day", String(IncubationProfile::currentDay(millis())), true);
    publish(MQTT_ROOT_TOPIC "/stage", IncubationProfile::isLockdown(millis()) ? "LOCKDOWN" : "INCUBATION", true);
}

void MQTTService::publishDiagnostics()
{
    publish(MQTT_ROOT_TOPIC "/ip", WiFi.localIP().toString(), true);
    publish(MQTT_ROOT_TOPIC "/rssi", String(WiFi.RSSI()), true);
    publish(MQTT_ROOT_TOPIC "/uptime", String(millis() / 1000), true);
    publish(MQTT_ROOT_TOPIC "/free_heap", String(ESP.getFreeHeap()), true);
}

void MQTTService::publishSensorStates()
{
    if (SensorManager::isSHT31Ready())
    {
        publish(MQTT_ROOT_TOPIC "/air_temperature", String(SensorManager::getAirTemperature(), 1), true);
        publish(MQTT_ROOT_TOPIC "/air_humidity", String(SensorManager::getAirHumidity(), 1), true);
    }

    if (SensorManager::isDS18B20Ready())
    {
        publish(MQTT_ROOT_TOPIC "/egg_temperature", String(SensorManager::getEggTemperature(), 1), true);
    }
}

void MQTTService::publishAll()
{
    publishAvailability();
    publishDiagnostics();
    publishRelayStates();
    publishSensorStates();
}

void MQTTService::callback(char *topic, byte *payload, unsigned int length)
{
    String cmdTopic(topic);
    String message;
    message.reserve(length);

    for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
    message.trim();
    message.toUpperCase();

    const bool state = (message == "ON");

    Serial.print("[MQTT] RX ");
    Serial.print(cmdTopic);
    Serial.print(" = ");
    Serial.println(message);

    if (cmdTopic.endsWith("/heater/set")) RelayController::heater(state);
    else if (cmdTopic.endsWith("/humidifier/set")) RelayController::humidifier(state);
    else if (cmdTopic.endsWith("/fan/set")) RelayController::fan(state);
    else if (cmdTopic.endsWith("/vent/set")) RelayController::ventilation(state);
    else if (cmdTopic.endsWith("/turner/set"))
    {
        if (state) EggTurnerManager::enable();
        else EggTurnerManager::disable();
    }
    else if (cmdTopic.endsWith("/profile/set"))
    {
        if (IncubationProfile::setProfileByCode(message, millis()))
        {
            Serial.print("[MQTT] Profile changed to: ");
            Serial.println(IncubationProfile::currentCode());

            mqtt.publish(MQTT_ROOT_TOPIC "/profile/state", IncubationProfile::currentCode(), true);
            mqtt.publish(MQTT_ROOT_TOPIC "/incubation_day", String(IncubationProfile::currentDay(millis())), true);
            mqtt.publish(MQTT_ROOT_TOPIC "/stage", IncubationProfile::isLockdown(millis()) ? "LOCKDOWN" : "INCUBATION", true);
        }
        else
        {
            Serial.print("[MQTT] Invalid profile: ");
            Serial.println(message);
        }
    }
    else
    {
        Serial.print("[MQTT] Unknown topic: ");
        Serial.println(cmdTopic);
        return;
    }

    mqtt.publishRelayStates();
}