#include <Arduino.h>
#include <math.h>
#include "logger.h"
#include "managers/temperature_manager.h"
#include "managers/humidity_manager.h"
#include "managers/egg_turner_manager.h"
#include "relay_controller.h"
#include "network_service.h"
#include "mqtt_service.h"
#include "incubation_profile.h"

extern NetworkService network;
extern MQTTService mqtt;

uint32_t lastUpdateTime = 0;
uint32_t updateInterval = 1000;

void printSystemStatus();
void applyProfileControl();

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("[BOOT] setup entered");

    Logger::begin(115200);
    Serial.println("[BOOT] logger started");

    Serial.println("[BOOT] network.begin()");
    network.begin();

    Serial.println("[BOOT] mqtt.begin()");
    mqtt.begin();

    RelayController::begin();
    TemperatureManager::begin();
    HumidityManager::begin();
    EggTurnerManager::begin();

    // Default profile: CHICKEN
    IncubationProfile::begin(IncubationProfileType::CHICKEN, millis());

    // Apply one-time initial profile values
    applyProfileControl();

    lastUpdateTime = millis();
    Serial.println("[BOOT] Initialization done.");
}

void loop()
{
    static uint32_t heartbeat = 0;
    if (millis() - heartbeat > 5000)
    {
        heartbeat = millis();
        Serial.println("[LOOP] network.loop() + mqtt.loop()");
    }

    network.loop();
    mqtt.loop();

    uint32_t now = millis();
    if (now - lastUpdateTime >= updateInterval)
    {
        lastUpdateTime = now;

        // Apply day/stage policy periodically
        applyProfileControl();

        TemperatureManager::update();
        HumidityManager::update();
        EggTurnerManager::update();

        static uint32_t statusCounter = 0;
        statusCounter++;
        if (statusCounter >= 10)
        {
            statusCounter = 0;
            printSystemStatus();
        }
    }

    delay(10);
}

void applyProfileControl()
{
    uint32_t now = millis();
    const auto& p = IncubationProfile::current();
    const uint16_t day = IncubationProfile::currentDay(now);
    const bool lockdown = IncubationProfile::isLockdown(now);

    const float targetTemp = p.tempSetpointC;
    const float targetHum  = lockdown ? p.humidityLockdown : p.humidityIncubation;

    // Only set when changed -> avoid INFO spam every second
    if (fabs(TemperatureManager::getTargetTemperature() - targetTemp) > 0.01f)
    {
        TemperatureManager::setTargetTemperature(targetTemp);
    }

    if (fabs(HumidityManager::getTargetHumidity() - targetHum) > 0.01f)
    {
        HumidityManager::setTargetHumidity(targetHum);
    }

    if (EggTurnerManager::getTurningInterval() != p.turnIntervalMin)
    {
        EggTurnerManager::setTurningInterval(p.turnIntervalMin);
    }

    if (EggTurnerManager::getTurnDuration() != p.turnDurationSec)
    {
        EggTurnerManager::setTurnDuration(p.turnDurationSec);
    }

    // Critical rule: in LOCKDOWN always force turner OFF
    if (lockdown && EggTurnerManager::isEnabled())
    {
        EggTurnerManager::disable();
        Serial.println("[PROFILE] Lockdown active -> Turner DISABLED");
    }

    // NOTE:
    // We intentionally DO NOT auto-enable turner in INCUBATION anymore.
    // This preserves user manual OFF command from Home Assistant.

    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 15000)
    {
        lastPrint = millis();
        Serial.print("[PROFILE] ");
        Serial.print(p.name);
        Serial.print(" day ");
        Serial.print(day);
        Serial.print("/");
        Serial.print(p.totalDays);
        Serial.print(" stage=");
        Serial.println(lockdown ? "LOCKDOWN" : "INCUBATION");
    }
}

void printSystemStatus()
{
    uint32_t now = millis();
    const auto& p = IncubationProfile::current();

    Serial.println("===== System Status =====");
    Serial.print("Profile: ");
    Serial.println(p.name);

    Serial.print("Day: ");
    Serial.print(IncubationProfile::currentDay(now));
    Serial.print("/");
    Serial.println(p.totalDays);

    Serial.print("Stage: ");
    Serial.println(IncubationProfile::isLockdown(now) ? "LOCKDOWN" : "INCUBATION");

    Serial.print("Temperature: ");
    Serial.print(TemperatureManager::getCurrentTemperature(), 1);
    Serial.print(" C (Target: ");
    Serial.print(TemperatureManager::getTargetTemperature(), 1);
    Serial.println(" C)");

    Serial.print("Humidity: ");
    Serial.print(HumidityManager::getCurrentHumidity(), 1);
    Serial.print("% (Target: ");
    Serial.print(HumidityManager::getTargetHumidity(), 1);
    Serial.println("%)");

    Serial.print("Turner enabled: ");
    Serial.println(EggTurnerManager::isEnabled() ? "YES" : "NO");

    Serial.print("Turn count: ");
    Serial.println(EggTurnerManager::getTurnCount());

    Serial.println("=========================");
}