#include "mqtt_service.h"

#include <WiFi.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "app_config.h"
#include "logger.h"
#include "sensor_manager.h"
#include "network_service.h"
#include "relay_controller.h"
#include "managers/egg_turner_manager.h"
#include "incubation_profile.h"
#include "system_state.h"
#include "simulation_control.h"

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
    if (MQTT_HOST[0] == '\0')
    {
        Serial.println("[MQTT] Broker is not configured");
        return;
    }
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
    if (MQTT_HOST[0] == '\0' || !network.connected())
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
        if (!discoveryPublished)
            publishDiscovery();
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

    if (!subscribeTopics())
    {
        if (!mqttClient.publish(MQTT_ROOT_TOPIC "/status", "offline", true))
            Serial.println("[MQTT] Failed to publish offline status");
        mqttClient.disconnect();
        mqttConnected = false;
        return;
    }
    publishDiscovery();
    publishRelayStates();
    publishDiagnostics();
    publishSensorStates();
    publishBirthMessage();
}

bool MQTTService::subscribeTopics()
{
    bool ok = true;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/heater/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/humidifier/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/fan/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/vent/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/turner/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/profile/set") && ok;
    ok = mqttClient.subscribe(MQTT_ROOT_TOPIC "/simulation/+/set") && ok;
    if (!ok)
    {
        Serial.println("[MQTT] Subscribe failed; reconnecting");
        return false;
    }
    Serial.println("[MQTT] Subscribe OK");
    return true;
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
            "\"name\":\"Heater output (locked off)\","
            "\"uniq_id\":\"" DEVICE_ID "_heater\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/heater/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/heater/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/humidifier/config",
            "{"
            "\"name\":\"Humidifier output (locked off)\","
            "\"uniq_id\":\"" DEVICE_ID "_humidifier\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/humidifier/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/humidifier/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/fan/config",
            "{"
            "\"name\":\"Circulation fan output (locked off)\","
            "\"uniq_id\":\"" DEVICE_ID "_fan\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/fan/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/fan/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/vent/config",
            "{"
            "\"name\":\"Ventilation fan output (locked off)\","
            "\"uniq_id\":\"" DEVICE_ID "_vent\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/vent/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/vent/state\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true);

    publish("homeassistant/switch/" DEVICE_ID "/turner/config",
            "{"
            "\"name\":\"Egg turner output (locked off)\","
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

    struct SimulationSwitch
    {
        const char *id;
        const char *name;
    };
    const SimulationSwitch switches[] = {
        {"heater", "Heater"},
        {"humidifier", "Humidifier"},
        {"fan", "Circulation Fan"},
        {"vent", "Ventilation Fan"},
        {"turner", "Egg Turner"}
    };
    bool simulationDiscoveryOk = true;
    for (const auto &entry : switches)
    {
        const String topic = String("homeassistant/switch/" DEVICE_ID "/sim_") + entry.id + "/config";
        const String payload =
            String("{\"name\":\"Simulated ") + entry.name +
            "\",\"uniq_id\":\"" DEVICE_ID "_sim_" + entry.id +
            "\",\"cmd_t\":\"" MQTT_ROOT_TOPIC "/simulation/" + entry.id +
            "/set\",\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/" + entry.id +
            "/state\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"availability\":[{\"topic\":\"" MQTT_ROOT_TOPIC "/status\"},"
            "{\"topic\":\"" MQTT_ROOT_TOPIC "/simulation/availability\"}],"
            "\"availability_mode\":\"all\"," + deviceJson + "}";
        simulationDiscoveryOk = publish(topic, payload, true) && simulationDiscoveryOk;
    }
    simulationDiscoveryOk = publish("homeassistant/sensor/" DEVICE_ID "/simulation_mode/config",
            "{\"name\":\"Simulation Mode\","
            "\"uniq_id\":\"" DEVICE_ID "_simulation_mode\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/mode\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true) && simulationDiscoveryOk;
    simulationDiscoveryOk = publish("homeassistant/select/" DEVICE_ID "/simulation_mode_select/config",
            "{\"name\":\"Simulation Mode Control\","
            "\"uniq_id\":\"" DEVICE_ID "_simulation_mode_select\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/simulation/mode/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/mode\","
            "\"options\":[\"MANUAL\",\"AUTO\",\"DRYRUN\"],"
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true) && simulationDiscoveryOk;
    simulationDiscoveryOk = publish("homeassistant/number/" DEVICE_ID "/dryrun_temperature/config",
            "{\"name\":\"Dry-run Temperature Input\","
            "\"uniq_id\":\"" DEVICE_ID "_dryrun_temperature\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/simulation/input/temperature/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/input/temperature/state\","
            "\"min\":-40,\"max\":125,\"step\":0.1,\"mode\":\"box\","
            "\"unit_of_measurement\":\"°C\","
            "\"availability\":[{\"topic\":\"" MQTT_ROOT_TOPIC "/status\"},"
            "{\"topic\":\"" MQTT_ROOT_TOPIC "/simulation/dryrun/availability\"}],"
            "\"availability_mode\":\"all\"," + deviceJson + "}", true) && simulationDiscoveryOk;
    simulationDiscoveryOk = publish("homeassistant/number/" DEVICE_ID "/dryrun_humidity/config",
            "{\"name\":\"Dry-run Humidity Input\","
            "\"uniq_id\":\"" DEVICE_ID "_dryrun_humidity\","
            "\"cmd_t\":\"" MQTT_ROOT_TOPIC "/simulation/input/humidity/set\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/input/humidity/state\","
            "\"min\":0,\"max\":100,\"step\":0.1,\"mode\":\"box\","
            "\"unit_of_measurement\":\"%\","
            "\"availability\":[{\"topic\":\"" MQTT_ROOT_TOPIC "/status\"},"
            "{\"topic\":\"" MQTT_ROOT_TOPIC "/simulation/dryrun/availability\"}],"
            "\"availability_mode\":\"all\"," + deviceJson + "}", true) && simulationDiscoveryOk;
    simulationDiscoveryOk = publish("homeassistant/binary_sensor/" DEVICE_ID "/simulation_heater_inhibit/config",
            "{\"name\":\"Simulated Heater Safety Inhibit\","
            "\"uniq_id\":\"" DEVICE_ID "_simulation_heater_inhibit\","
            "\"stat_t\":\"" MQTT_ROOT_TOPIC "/simulation/heater_inhibit\","
            "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
            "\"avty_t\":\"" MQTT_ROOT_TOPIC "/status\","
            "\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\","
            + deviceJson + "}", true) && simulationDiscoveryOk;
    discoveryPublished = simulationDiscoveryOk;
    Serial.println(simulationDiscoveryOk ? "[MQTT] Discovery published"
                                         : "[MQTT] Simulation discovery failed; will retry");
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
    publish(MQTT_ROOT_TOPIC "/heater/state", systemState.relay.heater ? "ON" : "OFF", true);
    publish(MQTT_ROOT_TOPIC "/humidifier/state", systemState.relay.humidifier ? "ON" : "OFF", true);
    publish(MQTT_ROOT_TOPIC "/fan/state", systemState.relay.circulationFan ? "ON" : "OFF", true);
    publish(MQTT_ROOT_TOPIC "/vent/state", systemState.relay.ventilationFan ? "ON" : "OFF", true);

    publish(MQTT_ROOT_TOPIC "/turner/state", EggTurnerManager::isEnabled() ? "ON" : "OFF", true);
    publish(MQTT_ROOT_TOPIC "/turn_count", String(EggTurnerManager::getTurnCount()), true);

    publish(MQTT_ROOT_TOPIC "/profile/state", IncubationProfile::currentCode(), true);
    publish(MQTT_ROOT_TOPIC "/incubation_day", String(IncubationProfile::currentDay(millis())), true);
    publish(MQTT_ROOT_TOPIC "/stage", IncubationProfile::isLockdown(millis()) ? "LOCKDOWN" : "INCUBATION", true);
    publishSimulationStates();
}

void MQTTService::publishSimulationStates()
{
    if (!mqttClient.connected())
        return;
    bool ok = true;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/availability", isManualSimulationMode() ? "online" : "offline", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/dryrun/availability", strcmp(simulationModeName(), "DRYRUN") == 0 ? "online" : "offline", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/mode", simulationModeName(), true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/heater_inhibit", RelayController::heaterSafetyBlocked() ? "ON" : "OFF", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/heater/state", RelayController::simulatedHeater() ? "ON" : "OFF", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/humidifier/state", RelayController::simulatedHumidifier() ? "ON" : "OFF", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/fan/state", RelayController::simulatedFan() ? "ON" : "OFF", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/vent/state", RelayController::simulatedVentilation() ? "ON" : "OFF", true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/turner/state", simulatedTurnerState() ? "ON" : "OFF", true) && ok;
    const float temperature = simulatedInputTemperature();
    const float humidity = simulatedInputHumidity();
    ok = publish(MQTT_ROOT_TOPIC "/simulation/input/temperature/state",
                 isfinite(temperature) ? String(temperature, 1) : String(""), true) && ok;
    ok = publish(MQTT_ROOT_TOPIC "/simulation/input/humidity/state",
                 isfinite(humidity) ? String(humidity, 1) : String(""), true) && ok;
    if (!ok)
        Serial.println("[MQTT] Failed to publish simulation state; will retry periodically");
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
    if (SensorManager::isSHT31Ready() &&
        isfinite(SensorManager::getAirTemperature()) &&
        isfinite(SensorManager::getAirHumidity()))
    {
        publish(MQTT_ROOT_TOPIC "/air_temperature", String(SensorManager::getAirTemperature(), 1), true);
        publish(MQTT_ROOT_TOPIC "/air_humidity", String(SensorManager::getAirHumidity(), 1), true);
    }
    else
    {
        publish(MQTT_ROOT_TOPIC "/air_temperature", "", true);
        publish(MQTT_ROOT_TOPIC "/air_humidity", "", true);
    }

    if (SensorManager::isDS18B20Ready() &&
        isfinite(SensorManager::getEggTemperature()))
    {
        publish(MQTT_ROOT_TOPIC "/egg_temperature", String(SensorManager::getEggTemperature(), 1), true);
    }
    else
    {
        publish(MQTT_ROOT_TOPIC "/egg_temperature", "", true);
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

    Serial.print("[MQTT] RX ");
    Serial.print(cmdTopic);
    Serial.print(" = ");
    Serial.println(message);

    if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/mode/set")
    {
        if (!setSimulationModeCommand(message.c_str()))
            Serial.println("[MQTT] Simulation mode request rejected");
        mqtt.publishSimulationStates();
        return;
    }

    if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/input/temperature/set" ||
        cmdTopic == MQTT_ROOT_TOPIC "/simulation/input/humidity/set")
    {
        char *end = nullptr;
        const float value = strtof(message.c_str(), &end);
        const bool validNumber = end != message.c_str() && *end == '\0' && isfinite(value);
        const bool accepted = validNumber &&
            (cmdTopic.endsWith("/temperature/set")
                 ? setDryRunTemperature(value)
                 : setDryRunHumidity(value));
        if (!accepted)
            Serial.println("[MQTT] Dry-run input rejected; wrong mode or out of range");
        mqtt.publishSimulationStates();
        return;
    }

    const bool simulationOutputCommand = cmdTopic.startsWith(MQTT_ROOT_TOPIC "/simulation/");
    if (!simulationOutputCommand && !cmdTopic.endsWith("/profile/set") &&
        message != "ON" && message != "OFF")
    {
        Serial.println("[MQTT] Invalid ON/OFF command");
        mqtt.publishSimulationStates();
        return;
    }

    if (simulationOutputCommand)
    {
        SimulatedOutput output;
        if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/heater/set") output = SimulatedOutput::Heater;
        else if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/humidifier/set") output = SimulatedOutput::Humidifier;
        else if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/fan/set") output = SimulatedOutput::Fan;
        else if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/vent/set") output = SimulatedOutput::Ventilation;
        else if (cmdTopic == MQTT_ROOT_TOPIC "/simulation/turner/set") output = SimulatedOutput::Turner;
        else
        {
            Serial.println("[MQTT] Unknown simulated output");
            return;
        }
        if (!setManualSimulatedOutput(output, message == "ON"))
            Serial.println("[MQTT] Simulated output request rejected");
        mqtt.publishSimulationStates();
        return;
    }

    if (cmdTopic.endsWith("/heater/set") || cmdTopic.endsWith("/humidifier/set") ||
        cmdTopic.endsWith("/fan/set") || cmdTopic.endsWith("/vent/set") ||
        cmdTopic.endsWith("/turner/set"))
    {
        Serial.println("[MQTT] Physical output commands disabled in simulation build");
        mqtt.publishRelayStates();
        return;
    }

    if (cmdTopic.endsWith("/profile/set"))
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