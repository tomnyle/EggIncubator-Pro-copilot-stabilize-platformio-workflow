#include "mqtt_service.h"

#include <WiFi.h>
#include "app_config.h"
#include "sensor_manager.h"
#include "network_service.h"
#include "relay_controller.h"
#include "managers/egg_turner_manager.h"
#include "managers/temperature_manager.h"
#include "managers/humidity_manager.h"
#include "incubation_profile.h"
#include "safety_logic.h"
#include "system_state.h"

extern MQTTService mqtt;
extern NetworkService network;

namespace
{
constexpr uint32_t RECONNECT_INTERVAL = 5000;
constexpr uint32_t PUBLISH_INTERVAL = 10000;

String topic(const char *suffix)
{
    return String(MQTT_ROOT_TOPIC) + "/" + suffix;
}

bool parseOnOff(const String &input, bool &out)
{
    if (input == "ON")
    {
        out = true;
        return true;
    }
    if (input == "OFF")
    {
        out = false;
        return true;
    }
    return false;
}

void applyMode(SystemMode mode)
{
    systemState.mode = mode;

    if (mode == SystemMode::OFF || mode == SystemMode::ALARM)
    {
        TemperatureManager::disable();
        HumidityManager::disable();
        EggTurnerManager::stopForSafety();
        RelayController::allOff();
    }
    else if (mode == SystemMode::AUTO)
    {
        TemperatureManager::enable();
        HumidityManager::enable();
        if (systemState.incubation.turningEnabled)
        {
            EggTurnerManager::enable();
        }
    }
}
}

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
    mqttClient.setBufferSize(1536);
    mqttClient.setCallback(callback);
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
        publishFaults();
    }
}

void MQTTService::reconnect()
{
    if (millis() - lastReconnect < RECONNECT_INTERVAL) return;
    lastReconnect = millis();

    const bool ok = mqttClient.connect(
        DEVICE_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        topic("status").c_str(),
        0,
        true,
        "offline");

    if (!ok)
    {
        mqttConnected = false;
        return;
    }

    mqttConnected = true;
    publishBirthMessage();
    subscribeTopics();
    publishDiscovery();
    publishAll();
}

void MQTTService::subscribeTopics()
{
    mqttClient.subscribe(topic("mode/set").c_str());
    mqttClient.subscribe(topic("start/set").c_str());
    mqttClient.subscribe(topic("target/temperature/set").c_str());
    mqttClient.subscribe(topic("target/humidity/set").c_str());
    mqttClient.subscribe(topic("profile/set").c_str());
    mqttClient.subscribe(topic("relay/heater/set").c_str());
    mqttClient.subscribe(topic("relay/humidifier/set").c_str());
    mqttClient.subscribe(topic("relay/fan/set").c_str());
    mqttClient.subscribe(topic("relay/vent/set").c_str());
    mqttClient.subscribe(topic("turner/set").c_str());
    mqttClient.subscribe(topic("turner/command").c_str());
}

void MQTTService::publishBirthMessage()
{
    publish(topic("status"), "online", true);
    publish(topic("mode/state"), modeToString(systemState.mode), true);
}

void MQTTService::publishDiscovery()
{
    if (discoveryPublished)
    {
        return;
    }

    const String deviceJson =
        "\"device\":{\"identifiers\":[\"" DEVICE_ID "\"],"
        "\"name\":\"Egg Incubator\","
        "\"manufacturer\":\"DIY\","
        "\"model\":\"EggIncubator Pro\","
        "\"sw_version\":\"" FIRMWARE_VERSION "\"}";
    const String availabilityJson =
        "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
        "\"pl_avail\":\"online\","
        "\"pl_not_avail\":\"offline\",";

    publish("homeassistant/sensor/" DEVICE_ID "/air_temp/config",
            "{\"name\":\"Air Temperature\",\"uniq_id\":\"" DEVICE_ID "_air_temp\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/temperature/air\",\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/air_humidity/config",
            "{\"name\":\"Air Humidity\",\"uniq_id\":\"" DEVICE_ID "_air_humidity\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/humidity/air\",\"unit_of_meas\":\"%\",\"dev_cla\":\"humidity\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/egg_temp/config",
            "{\"name\":\"Egg Temperature\",\"uniq_id\":\"" DEVICE_ID "_egg_temp\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/temperature/egg\",\"unit_of_meas\":\"°C\",\"dev_cla\":\"temperature\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/incubation_day/config",
            "{\"name\":\"Incubation Day\",\"uniq_id\":\"" DEVICE_ID "_day\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/incubation/day\",\"icon\":\"mdi:calendar\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/fault/config",
            "{\"name\":\"Incubator Fault\",\"uniq_id\":\"" DEVICE_ID "_fault\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/fault/state\",\"icon\":\"mdi:alert\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/turner_state/config",
            "{\"name\":\"Turner State\",\"uniq_id\":\"" DEVICE_ID "_turner_state\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/turner/state\",\"icon\":\"mdi:rotate-right\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/sensor/" DEVICE_ID "/mode/config",
            "{\"name\":\"Mode\",\"uniq_id\":\"" DEVICE_ID "_mode\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/mode/state\",\"icon\":\"mdi:toggle-switch\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/select/" DEVICE_ID "/mode_set/config",
            "{\"name\":\"Mode Set\",\"uniq_id\":\"" DEVICE_ID "_mode_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/mode/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/mode/state\",\"options\":[\"OFF\",\"MANUAL\",\"AUTO\",\"ALARM\"]," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/start_set/config",
            "{\"name\":\"Incubation Start\",\"uniq_id\":\"" DEVICE_ID "_start_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/start/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/start/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/number/" DEVICE_ID "/target_temp_set/config",
            "{\"name\":\"Target Temperature\",\"uniq_id\":\"" DEVICE_ID "_target_temp_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/target/temperature/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/target/temperature/state\",\"min\":34,\"max\":39.5,\"step\":0.1,\"unit_of_meas\":\"°C\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/number/" DEVICE_ID "/target_humidity_set/config",
            "{\"name\":\"Target Humidity\",\"uniq_id\":\"" DEVICE_ID "_target_humidity_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/target/humidity/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/target/humidity/state\",\"min\":30,\"max\":85,\"step\":1,\"unit_of_meas\":\"%\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/turner_enable/config",
            "{\"name\":\"Turner Enable\",\"uniq_id\":\"" DEVICE_ID "_turner_enable\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/turner/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/turner/enabled\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/select/" DEVICE_ID "/profile_set/config",
            "{\"name\":\"Profile Set\",\"uniq_id\":\"" DEVICE_ID "_profile_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/profile/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/profile/state\",\"options\":[\"CHICKEN\",\"QUAIL\",\"DUCK\",\"GOOSE\"]," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/relay_heater_set/config",
            "{\"name\":\"Manual Heater\",\"uniq_id\":\"" DEVICE_ID "_relay_heater_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/relay/heater/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/relay/heater/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/relay_humidifier_set/config",
            "{\"name\":\"Manual Humidifier\",\"uniq_id\":\"" DEVICE_ID "_relay_humidifier_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/relay/humidifier/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/relay/humidifier/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/relay_fan_set/config",
            "{\"name\":\"Manual Circulation Fan\",\"uniq_id\":\"" DEVICE_ID "_relay_fan_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/relay/fan/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/relay/fan/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);
    publish("homeassistant/switch/" DEVICE_ID "/relay_vent_set/config",
            "{\"name\":\"Manual Vent Fan\",\"uniq_id\":\"" DEVICE_ID "_relay_vent_set\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/relay/vent/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/relay/vent/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"," + availabilityJson + deviceJson + "}", true);

    systemState.haDiscoveryPublished = true;
    discoveryPublished = true;
}

bool MQTTService::publish(const char *topicName, const char *payload, bool retained)
{
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topicName, payload, retained);
}

bool MQTTService::publish(const String &topicName, const String &payload, bool retained)
{
    return publish(topicName.c_str(), payload.c_str(), retained);
}

void MQTTService::publishAvailability()
{
    publish(topic("status"), "online", true);
}

void MQTTService::publishRelayStates()
{
    publish(topic("relay/heater/state"), RelayController::heaterState() ? "ON" : "OFF", true);
    publish(topic("relay/humidifier/state"), RelayController::humidifierState() ? "ON" : "OFF", true);
    publish(topic("relay/fan/state"), RelayController::fanState() ? "ON" : "OFF", true);
    publish(topic("relay/vent/state"), RelayController::ventilationState() ? "ON" : "OFF", true);

    publish(topic("turner/state"), EggTurnerManager::getStateName(), true);
    publish(topic("turner/enabled"), EggTurnerManager::isEnabled() ? "ON" : "OFF", true);
    publish(topic("mode/state"), modeToString(systemState.mode), true);
    publish(topic("start/state"), systemState.incubation.incubationRunning ? "ON" : "OFF", true);
    publish(topic("target/temperature/state"), String(systemState.incubation.targetTemperature, 2), true);
    publish(topic("target/humidity/state"), String(systemState.incubation.targetHumidity, 1), true);
    publish(topic("profile/state"), systemState.incubation.profileCode, true);
}

void MQTTService::publishDiagnostics()
{
    publish(topic("rssi"), String(network.rssi()), true);
    publish(topic("uptime"), String(systemState.uptimeSeconds), true);
    publish(topic("incubation/day"), String(systemState.incubation.incubationDay), true);
    publish(topic("status"), network.connected() ? "online" : "offline", true);
}

void MQTTService::publishSensorStates()
{
    publish(topic("temperature/air"), String(systemState.sensor.airTemperature.value, 1), true);
    publish(topic("temperature/egg"), String(systemState.sensor.eggTemperature.value, 1), true);
    publish(topic("humidity/air"), String(systemState.sensor.airHumidity.value, 1), true);
    publish(topic("door/state"), systemState.sensor.doorOpen ? "OPEN" : "CLOSED", true);
    publish(topic("water_low/state"), systemState.sensor.waterLow ? "ON" : "OFF", true);
}

void MQTTService::publishFaults()
{
    publish(topic("fault/state"), activeFaultSummary(), true);
    publish(topic("alarm/state"), hasCriticalFault() ? "ON" : "OFF", true);
}

void MQTTService::publishAll()
{
    publishAvailability();
    publishDiagnostics();
    publishRelayStates();
    publishSensorStates();
    publishFaults();
}

void MQTTService::callback(char *topicRaw, byte *payload, unsigned int length)
{
    String cmdTopic(topicRaw);
    String message;
    message.reserve(length);

    for (unsigned int i = 0; i < length; i++) message += static_cast<char>(payload[i]);
    message.trim();

    String upper = message;
    upper.toUpperCase();

    bool onOffValue = false;
    const auto route = SafetyLogic::classifyCommandTopic(cmdTopic.c_str());

    switch (route)
    {
        case SafetyLogic::CommandRoute::MODE_SET:
            switch (SafetyLogic::parseMode(upper.c_str()))
            {
                case SafetyLogic::RequestedMode::OFF: applyMode(SystemMode::OFF); break;
                case SafetyLogic::RequestedMode::MANUAL: applyMode(SystemMode::MANUAL); break;
                case SafetyLogic::RequestedMode::AUTO: applyMode(SystemMode::AUTO); break;
                case SafetyLogic::RequestedMode::ALARM: applyMode(SystemMode::ALARM); break;
                case SafetyLogic::RequestedMode::INVALID:
                default: break;
            }
            break;

        case SafetyLogic::CommandRoute::START_SET:
            if (parseOnOff(upper, onOffValue))
            {
                systemState.incubation.incubationRunning = onOffValue;
                if (!onOffValue) applyMode(SystemMode::OFF);
                else if (systemState.mode == SystemMode::OFF) applyMode(SystemMode::AUTO);
            }
            break;

        case SafetyLogic::CommandRoute::TARGET_TEMP_SET:
        {
            const float value = message.toFloat();
            const float applied = SafetyLogic::applyTargetUpdate(systemState.incubation.targetTemperature, value, 34.0f, 39.5f);
            if (applied != systemState.incubation.targetTemperature)
            {
                systemState.incubation.targetTemperature = applied;
                TemperatureManager::setTargetTemperature(applied);
            }
            break;
        }

        case SafetyLogic::CommandRoute::TARGET_HUM_SET:
        {
            const float value = message.toFloat();
            const float applied = SafetyLogic::applyTargetUpdate(systemState.incubation.targetHumidity, value, 30.0f, 85.0f);
            if (applied != systemState.incubation.targetHumidity)
            {
                systemState.incubation.targetHumidity = applied;
                HumidityManager::setTargetHumidity(applied);
            }
            break;
        }

        case SafetyLogic::CommandRoute::PROFILE_SET:
            if (IncubationProfile::setProfileByCode(upper, systemState.incubation.incubationStartEpoch))
            {
                systemState.incubation.profileCode = upper;
                const auto &profile = IncubationProfile::current();
                systemState.incubation.targetTemperature = profile.tempSetpointC;
                systemState.incubation.targetHumidity = profile.humidityIncubation;
                TemperatureManager::setTargetTemperature(profile.tempSetpointC);
                HumidityManager::setTargetHumidity(profile.humidityIncubation);
            }
            break;

        case SafetyLogic::CommandRoute::RELAY_HEATER_SET:
            if (SafetyLogic::manualRelayAllowed(systemState.mode == SystemMode::MANUAL, hasCriticalFault()) && parseOnOff(upper, onOffValue))
            {
                RelayController::heater(onOffValue);
            }
            break;

        case SafetyLogic::CommandRoute::RELAY_HUMIDIFIER_SET:
            if (SafetyLogic::manualRelayAllowed(systemState.mode == SystemMode::MANUAL, hasCriticalFault()) && parseOnOff(upper, onOffValue))
            {
                RelayController::humidifier(onOffValue);
            }
            break;

        case SafetyLogic::CommandRoute::RELAY_FAN_SET:
            if (systemState.mode == SystemMode::MANUAL && parseOnOff(upper, onOffValue))
            {
                RelayController::fan(onOffValue);
            }
            break;

        case SafetyLogic::CommandRoute::RELAY_VENT_SET:
            if (systemState.mode == SystemMode::MANUAL && parseOnOff(upper, onOffValue))
            {
                RelayController::ventilation(onOffValue);
            }
            break;

        case SafetyLogic::CommandRoute::TURNER_SET:
            if (parseOnOff(upper, onOffValue))
            {
                if (!SensorManager::isDoorOpen() && !hasFault(FAULT_TURN_TIMEOUT))
                {
                    if (onOffValue) EggTurnerManager::enable();
                    else EggTurnerManager::disable();
                }
            }
            break;

        case SafetyLogic::CommandRoute::TURNER_COMMAND:
            if (!SensorManager::isDoorOpen() && !hasCriticalFault())
            {
                if (upper == "MOVE_HOME") EggTurnerManager::requestMoveHome();
                else if (upper == "MOVE_END") EggTurnerManager::requestMoveEnd();
                else if (upper == "STEP") EggTurnerManager::manualTurn();
            }
            break;

        case SafetyLogic::CommandRoute::UNKNOWN:
        default:
            break;
    }

    mqtt.publishAll();
}
