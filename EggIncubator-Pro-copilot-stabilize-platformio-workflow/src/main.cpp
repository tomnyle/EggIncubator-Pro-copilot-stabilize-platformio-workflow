#include <Arduino.h>
#include <Preferences.h>
#include <esp_task_wdt.h>

#include "logger.h"
#include "sensor_manager.h"
#include "managers/temperature_manager.h"
#include "managers/humidity_manager.h"
#include "managers/egg_turner_manager.h"
#include "relay_controller.h"
#include "network_service.h"
#include "mqtt_service.h"
#include "incubation_profile.h"
#include "system_state.h"

extern NetworkService network;
extern MQTTService mqtt;

namespace
{
Preferences prefs;
uint32_t lastControlMs = 0;
uint32_t lastStatusMs = 0;
uint32_t lastPersistMs = 0;
uint32_t bootMillis = 0;
uint32_t incubationElapsedAtBoot = 0;

constexpr uint32_t CONTROL_INTERVAL_MS = 1000;
constexpr uint32_t STATUS_INTERVAL_MS = 15000;
constexpr uint32_t PERSIST_INTERVAL_MS = 30000;

void persistState()
{
    if (!prefs.begin("incubator", false))
    {
        return;
    }

    prefs.putUChar("mode", static_cast<uint8_t>(systemState.mode));
    prefs.putBool("running", systemState.incubation.incubationRunning);
    prefs.putBool("turning", systemState.incubation.turningEnabled);
    prefs.putFloat("tgt_t", systemState.incubation.targetTemperature);
    prefs.putFloat("tgt_h", systemState.incubation.targetHumidity);
    prefs.putFloat("ovrt", systemState.incubation.overTemperatureLimit);
    prefs.putULong("start", systemState.incubation.incubationStartEpoch);
    prefs.putULong("elapsed", incubationElapsedAtBoot + ((millis() - bootMillis) / 1000UL));
    prefs.putString("profile", systemState.incubation.profileCode);
    prefs.end();
}

void restoreState()
{
    if (!prefs.begin("incubator", true))
    {
        return;
    }

    systemState.mode = static_cast<SystemMode>(prefs.getUChar("mode", static_cast<uint8_t>(SystemMode::OFF)));
    systemState.incubation.incubationRunning = prefs.getBool("running", false);
    systemState.incubation.turningEnabled = prefs.getBool("turning", true);
    systemState.incubation.targetTemperature = prefs.getFloat("tgt_t", 37.5f);
    systemState.incubation.targetHumidity = prefs.getFloat("tgt_h", 60.0f);
    systemState.incubation.overTemperatureLimit = prefs.getFloat("ovrt", 38.5f);
    systemState.incubation.incubationStartEpoch = prefs.getULong("start", 0);
    incubationElapsedAtBoot = prefs.getULong("elapsed", 0);
    systemState.incubation.profileCode = prefs.getString("profile", "CHICKEN");

    prefs.end();

    IncubationProfile::setProfileByCode(systemState.incubation.profileCode, 0);
}

uint16_t getIncubationDay()
{
    if (!systemState.incubation.incubationRunning)
    {
        return 0;
    }

    const uint32_t elapsed = incubationElapsedAtBoot + ((millis() - bootMillis) / 1000UL);
    return static_cast<uint16_t>((elapsed / 86400UL) + 1);
}

void setMode(SystemMode mode)
{
    if (mode == systemState.mode)
    {
        return;
    }

    systemState.mode = mode;

    if (mode == SystemMode::OFF || mode == SystemMode::ALARM)
    {
        TemperatureManager::disable();
        HumidityManager::disable();
        EggTurnerManager::disable();
        RelayController::allOff();
    }
    else if (mode == SystemMode::AUTO)
    {
        TemperatureManager::enable();
        HumidityManager::enable();
        if (systemState.incubation.turningEnabled) EggTurnerManager::enable();
    }
}

void applyProfileControl()
{
    const auto &p = IncubationProfile::current();
    const uint16_t day = getIncubationDay();
    const bool lockdown = day >= p.lockdownDay;

    systemState.incubation.incubationDay = day;

    TemperatureManager::setTargetTemperature(systemState.incubation.targetTemperature);
    HumidityManager::setTargetHumidity(lockdown ? p.humidityLockdown : systemState.incubation.targetHumidity);

    EggTurnerManager::setTurningInterval(p.turnIntervalMin);
    EggTurnerManager::setTurnDuration(p.turnDurationSec);

    if (lockdown)
    {
        systemState.incubation.turningEnabled = false;
        EggTurnerManager::disable();
    }
    else if (systemState.mode == SystemMode::AUTO && systemState.incubation.turningEnabled)
    {
        EggTurnerManager::enable();
    }
}

void applySafetyPolicy()
{
    if (hasCriticalFault())
    {
        setMode(SystemMode::ALARM);
        RelayController::buzzer(true);
    }
    else
    {
        RelayController::buzzer(false);
        if (systemState.mode == SystemMode::ALARM)
        {
            setMode(SystemMode::OFF);
        }
    }
}

void printSystemStatus()
{
    Serial.println("===== Incubator Status =====");
    Serial.print("Mode: ");
    Serial.println(modeToString(systemState.mode));

    Serial.print("Day: ");
    Serial.print(systemState.incubation.incubationDay);
    Serial.print(" Profile: ");
    Serial.println(systemState.incubation.profileCode);

    Serial.print("Air Temp: ");
    Serial.print(systemState.sensor.airTemperature.value, 1);
    Serial.print("C (valid=");
    Serial.print(systemState.sensor.airTemperature.valid ? "Y" : "N");
    Serial.print(") Egg Temp: ");
    Serial.print(systemState.sensor.eggTemperature.value, 1);
    Serial.println("C");

    Serial.print("RH: ");
    Serial.print(systemState.sensor.airHumidity.value, 1);
    Serial.print("% Target T/H: ");
    Serial.print(systemState.incubation.targetTemperature, 1);
    Serial.print("/");
    Serial.println(systemState.incubation.targetHumidity, 1);

    Serial.print("Relay H/H/F/V: ");
    Serial.print(systemState.output.heater);
    Serial.print("/");
    Serial.print(systemState.output.humidifier);
    Serial.print("/");
    Serial.print(systemState.output.circulationFan);
    Serial.print("/");
    Serial.println(systemState.output.ventilationFan);

    Serial.print("Turner: ");
    Serial.print(EggTurnerManager::getStateName());
    Serial.print(" door=");
    Serial.println(systemState.sensor.doorOpen ? "OPEN" : "CLOSED");

    Serial.print("WiFi/MQTT: ");
    Serial.print(systemState.connection.wifi ? "UP" : "DOWN");
    Serial.print("/");
    Serial.println(systemState.connection.mqtt ? "UP" : "DOWN");

    Serial.print("Faults: ");
    Serial.println(activeFaultSummary());
    Serial.println("===========================");
}
}

void setup()
{
    Logger::begin(115200);

    bootMillis = millis();

    RelayController::begin();
    SensorManager::begin();
    TemperatureManager::begin();
    HumidityManager::begin();
    EggTurnerManager::begin();

    restoreState();

    IncubationProfile::setProfileByCode(systemState.incubation.profileCode, 0);

    network.begin();
    mqtt.begin();

    esp_task_wdt_init(15, true);
    esp_task_wdt_add(NULL);

    setMode(systemState.mode);
    applyProfileControl();
}

void loop()
{
    esp_task_wdt_reset();

    network.loop();
    mqtt.loop();

    systemState.connection.wifi = network.connected();
    systemState.connection.mqtt = mqtt.connected();
    systemState.connection.rssi = network.rssi();
    systemState.uptimeSeconds = millis() / 1000UL;

    setFault(WARN_MQTT_OFFLINE, !mqtt.connected(), "MQTT disconnected");

    const uint32_t now = millis();
    if (now - lastControlMs >= CONTROL_INTERVAL_MS)
    {
        lastControlMs = now;

        SensorManager::update();
        systemState.incubation.incubationDay = getIncubationDay();
        applyProfileControl();
        applySafetyPolicy();

        if (systemState.mode == SystemMode::AUTO && systemState.incubation.incubationRunning && !hasCriticalFault())
        {
            TemperatureManager::enable();
            HumidityManager::enable();
            TemperatureManager::update();
            HumidityManager::update();
            EggTurnerManager::update();
        }
        else
        {
            TemperatureManager::disable();
            HumidityManager::disable();
            EggTurnerManager::disable();
        }
    }

    if (now - lastStatusMs >= STATUS_INTERVAL_MS)
    {
        lastStatusMs = now;
        printSystemStatus();
    }

    if (now - lastPersistMs >= PERSIST_INTERVAL_MS)
    {
        lastPersistMs = now;
        persistState();
    }
}
