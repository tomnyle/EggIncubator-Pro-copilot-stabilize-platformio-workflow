#include <Arduino.h>
#include <math.h>
#include "logger.h"
#include "gpio_manager.h"
#include "sensor_manager.h"
#include "core/system_state.h"
#include "pins.h"
#include "managers/temperature_manager.h"
#include "managers/humidity_manager.h"
#include "managers/egg_turner_manager.h"
#include "relay_controller.h"
#include "network_service.h"
#include "mqtt_service.h"
#include "incubation_profile.h"
#include "app_config.h"

#include <stdlib.h>
#include <string.h>

extern NetworkService network;
extern MQTTService mqtt;

uint32_t lastUpdateTime = 0;
uint32_t updateInterval = 1000;

enum class TestMode
{
    MANUAL,
    AUTO,
    DRY_RUN
};

TestMode testMode = TestMode::MANUAL;
float simulatedTemperature = NAN;
float simulatedHumidity = NAN;
bool simulatedTurner = false;

void printSystemStatus();
void applyProfileControl();
void processSerialCommands();
void setTestMode(TestMode mode);
void processCommand(char *command);
void refreshHeaterSafety();

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("[BOOT] setup entered");

    Logger::begin(115200);
    Serial.println("[BOOT] logger started");

    SystemState::begin();
    GPIOManager::begin();
    RelayController::begin();
    SensorManager::begin();
    TemperatureManager::begin();
    HumidityManager::begin();
    EggTurnerManager::begin();

    // Default profile: CHICKEN
    IncubationProfile::begin(IncubationProfileType::CHICKEN, millis());

    // Apply one-time initial profile values
    applyProfileControl();

    setTestMode(TestMode::MANUAL);

    Serial.println("[BOOT] network.begin()");
    network.begin();

    Serial.println("[BOOT] mqtt.begin()");
    mqtt.begin();

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
    SensorManager::update();
    refreshHeaterSafety();
    processSerialCommands();
    EggTurnerManager::update();
    digitalWrite(PIN_BUZZER, SystemState::getAlarmStatus().buzzerActive ? HIGH : LOW);

    uint32_t now = millis();
    if (now - lastUpdateTime >= updateInterval)
    {
        lastUpdateTime = now;

        // Apply day/stage policy periodically
        applyProfileControl();

        const bool useSimulatedSensors = testMode == TestMode::DRY_RUN;
        const bool sensorReady = useSimulatedSensors
            ? isfinite(simulatedTemperature) && isfinite(simulatedHumidity)
            : SensorManager::isSHT31Ready();
        TemperatureManager::update(useSimulatedSensors ? simulatedTemperature
                                                       : SensorManager::getAirTemperature(), sensorReady);
        HumidityManager::update(useSimulatedSensors ? simulatedHumidity
                                                    : SensorManager::getAirHumidity(), sensorReady);

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

void refreshHeaterSafety()
{
    const bool dryRun = testMode == TestMode::DRY_RUN;
    const float temperature = dryRun ? simulatedTemperature : SensorManager::getAirTemperature();
    const float humidity = dryRun ? simulatedHumidity : SensorManager::getAirHumidity();
    const bool ready = dryRun || SensorManager::isSHT31Ready();
    const bool valid = ready && isfinite(humidity) && humidity >= 0.0f && humidity <= 100.0f;
    RelayController::updateHeaterSafety(temperature, valid);
}

void processSerialCommands()
{
    static char command[96];
    static size_t commandLength = 0;
    static bool overflow = false;

    while (Serial.available() > 0)
    {
        const char input = static_cast<char>(Serial.read());
        if (input == '\r' || input == '\n')
        {
            if (overflow)
            {
                Serial.println("[TEST] Command too long; discarded.");
                overflow = false;
                commandLength = 0;
                continue;
            }
            if (commandLength == 0)
            {
                continue;
            }

            command[commandLength] = '\0';
            processCommand(command);
            commandLength = 0;
        }
        else if (!overflow && commandLength < sizeof(command) - 1)
        {
            command[commandLength++] = input;
        }
        else
        {
            overflow = true;
        }
    }
}

void processCommand(char *command)
{
    char *context = nullptr;
    const char *verb = strtok_r(command, " \t", &context);
    const char *first = strtok_r(nullptr, " \t", &context);
    const char *second = strtok_r(nullptr, " \t", &context);
    const char *extra = strtok_r(nullptr, " \t", &context);
    if (!verb)
        return;

    if (strcmp(verb, "MODE") == 0 && first && !second)
    {
        if (strcmp(first, "MANUAL") == 0) setTestMode(TestMode::MANUAL);
        else if (strcmp(first, "AUTO") == 0) setTestMode(TestMode::AUTO);
        else if (strcmp(first, "DRYRUN") == 0) setTestMode(TestMode::DRY_RUN);
        else Serial.println("[TEST] Invalid mode: use MANUAL, AUTO or DRYRUN.");
        return;
    }

    if (strcmp(verb, "INPUT") == 0 && first && second && !extra)
    {
        if (testMode != TestMode::DRY_RUN)
        {
            Serial.println("[TEST] INPUT is available only in DRYRUN.");
            return;
        }
        char *endTemperature = nullptr;
        char *endHumidity = nullptr;
        const float temperature = strtof(first, &endTemperature);
        const float humidity = strtof(second, &endHumidity);
        if (endTemperature == first || *endTemperature != '\0' ||
            endHumidity == second || *endHumidity != '\0' ||
            !isfinite(temperature) || temperature < -40.0f || temperature > 125.0f ||
            !isfinite(humidity) || humidity < 0.0f || humidity > 100.0f)
        {
            Serial.println("[TEST] Invalid INPUT. Use temperature -40..125 and humidity 0..100.");
            return;
        }
        simulatedTemperature = temperature;
        simulatedHumidity = humidity;
        refreshHeaterSafety();
        Serial.printf("[TEST] Simulated input: %.1f C, %.1f%%\n", temperature, humidity);
        return;
    }

    if (strcmp(verb, "SET") == 0 && first && second && !extra)
    {
        if (testMode != TestMode::MANUAL)
        {
            Serial.println("[TEST] SET is available only in MANUAL.");
            return;
        }
        if (strcmp(second, "ON") != 0 && strcmp(second, "OFF") != 0)
        {
            Serial.println("[TEST] SET requires ON or OFF.");
            return;
        }
        const bool state = strcmp(second, "ON") == 0;
        if (strcmp(first, "HEATER") == 0) RelayController::heater(state);
        else if (strcmp(first, "HUMIDIFIER") == 0) RelayController::humidifier(state);
        else if (strcmp(first, "FAN") == 0) RelayController::fan(state);
        else if (strcmp(first, "VENT") == 0) RelayController::ventilation(state);
        else if (strcmp(first, "TURNER") == 0)
        {
            simulatedTurner = state;
            Serial.printf("[SIMULATION] Turner request %s; motor held OFF\n", state ? "ON" : "OFF");
        }
        else Serial.println("[TEST] Unknown output: use HEATER, HUMIDIFIER, FAN, VENT or TURNER.");
        return;
    }
    Serial.println("[TEST] Commands: MODE MANUAL|AUTO|DRYRUN, INPUT <temp> <humidity>, SET <output> ON|OFF.");
}

void setTestMode(TestMode mode)
{
    TemperatureManager::disable();
    HumidityManager::disable();
    EggTurnerManager::disable();
    RelayController::allOff();
    simulatedTurner = false;
    simulatedTemperature = NAN;
    simulatedHumidity = NAN;

    testMode = mode;
    refreshHeaterSafety();
    if (testMode != TestMode::MANUAL)
    {
        TemperatureManager::enable();
        HumidityManager::enable();
        Serial.println(testMode == TestMode::AUTO
                           ? "[TEST] AUTO: real sensors, simulated outputs."
                           : "[TEST] DRYRUN: enter INPUT <temperature> <humidity>; simulated outputs.");
    }
    else
    {
        Serial.println("[TEST] MANUAL: use SET <output> ON|OFF; simulated outputs.");
    }
    Serial.println("[SAFETY] Relay and motor outputs locked OFF.");
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
    Serial.print("Test mode: ");
    Serial.println(testMode == TestMode::DRY_RUN ? "DRYRUN" : testMode == TestMode::AUTO ? "AUTO" : "MANUAL");
    Serial.println("Physical relay and motor outputs: LOCKED OFF");
    Serial.print("Heater safety: ");
    Serial.println(RelayController::heaterSafetyBlocked() ? "INHIBITED" : "READY");
    if (testMode == TestMode::DRY_RUN)
    {
        Serial.print("Simulated input: ");
        if (isfinite(simulatedTemperature) && isfinite(simulatedHumidity))
            Serial.printf("%.1f C, %.1f%%\n", simulatedTemperature, simulatedHumidity);
        else
            Serial.println("not set");
    }
    Serial.print("Profile: ");
    Serial.println(p.name);

    Serial.print("Day: ");
    Serial.print(IncubationProfile::currentDay(now));
    Serial.print("/");
    Serial.println(p.totalDays);

    Serial.print("Stage: ");
    Serial.println(IncubationProfile::isLockdown(now) ? "LOCKDOWN" : "INCUBATION");

    Serial.print("Temperature: ");
    if (isfinite(SensorManager::getAirTemperature()))
        Serial.print(SensorManager::getAirTemperature(), 1);
    else
        Serial.print("unavailable");
    Serial.print(" C (Target: ");
    Serial.print(TemperatureManager::getTargetTemperature(), 1);
    Serial.println(" C)");

    Serial.print("Humidity: ");
    if (isfinite(SensorManager::getAirHumidity()))
        Serial.print(SensorManager::getAirHumidity(), 1);
    else
        Serial.print("unavailable");
    Serial.print("% (Target: ");
    Serial.print(HumidityManager::getTargetHumidity(), 1);
    Serial.println("%)");

    Serial.print("Turner enabled: ");
    Serial.println(EggTurnerManager::isEnabled() ? "YES" : "NO");

    Serial.print("Turn count: ");
    Serial.println(EggTurnerManager::getTurnCount());

    Serial.printf("Simulated requests: heater=%s humidifier=%s fan=%s vent=%s turner=%s\n",
                  RelayController::simulatedHeater() ? "ON" : "OFF",
                  RelayController::simulatedHumidifier() ? "ON" : "OFF",
                  RelayController::simulatedFan() ? "ON" : "OFF",
                  RelayController::simulatedVentilation() ? "ON" : "OFF",
                  simulatedTurner ? "ON" : "OFF");
    Serial.println("=========================");
}