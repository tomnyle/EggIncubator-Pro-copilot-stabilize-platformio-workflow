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
#include "simulation_control.h"

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
void publishSimulationChanges();

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

    SensorManager::update();
    refreshHeaterSafety();
    network.loop();
    mqtt.loop();
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

    publishSimulationChanges();
    delay(10);
}

bool isManualSimulationMode() { return testMode == TestMode::MANUAL; }
bool simulatedTurnerState() { return simulatedTurner; }
float simulatedInputTemperature() { return simulatedTemperature; }
float simulatedInputHumidity() { return simulatedHumidity; }

const char *simulationModeName()
{
    return testMode == TestMode::DRY_RUN ? "DRYRUN" : testMode == TestMode::AUTO ? "AUTO" : "MANUAL";
}

bool setSimulationModeCommand(const char *mode)
{
    if (strcmp(mode, "MANUAL") == 0) setTestMode(TestMode::MANUAL);
    else if (strcmp(mode, "AUTO") == 0) setTestMode(TestMode::AUTO);
    else if (strcmp(mode, "DRYRUN") == 0) setTestMode(TestMode::DRY_RUN);
    else
    {
        Serial.println("[TEST] Invalid mode: use MANUAL, AUTO or DRYRUN.");
        return false;
    }
    return true;
}

bool setDryRunTemperature(float temperature)
{
    if (testMode != TestMode::DRY_RUN || !isfinite(temperature) ||
        temperature < -40.0f || temperature > 125.0f)
        return false;
    simulatedTemperature = temperature;
    refreshHeaterSafety();
    return true;
}

bool setDryRunHumidity(float humidity)
{
    if (testMode != TestMode::DRY_RUN || !isfinite(humidity) ||
        humidity < 0.0f || humidity > 100.0f)
        return false;
    simulatedHumidity = humidity;
    refreshHeaterSafety();
    return true;
}

bool setManualSimulatedOutput(SimulatedOutput output, bool state)
{
    if (!isManualSimulationMode())
    {
        Serial.println("[TEST] Simulated output commands require MANUAL mode.");
        return false;
    }
    switch (output)
    {
        case SimulatedOutput::Heater:
            if (state && RelayController::heaterSafetyBlocked())
            {
                Serial.println("[SAFETY] Simulated heater request rejected: heater inhibited.");
                return false;
            }
            RelayController::heater(state);
            return true;
        case SimulatedOutput::Humidifier: RelayController::humidifier(state); return true;
        case SimulatedOutput::Fan: RelayController::fan(state); return true;
        case SimulatedOutput::Ventilation: RelayController::ventilation(state); return true;
        case SimulatedOutput::Turner:
            simulatedTurner = state;
            Serial.printf("[SIMULATION] Turner request %s; motor held OFF\n", state ? "ON" : "OFF");
            return true;
    }
    Serial.println("[TEST] Unknown simulated output.");
    return false;
}

void publishSimulationChanges()
{
    static bool first = true;
    static TestMode lastMode = TestMode::MANUAL;
    static bool lastHeater = false, lastHumidifier = false, lastFan = false;
    static bool lastVent = false, lastTurner = false, lastBlocked = false;
    static float lastTemperature = NAN, lastHumidity = NAN;
    const bool heater = RelayController::simulatedHeater();
    const bool humidifier = RelayController::simulatedHumidifier();
    const bool fan = RelayController::simulatedFan();
    const bool vent = RelayController::simulatedVentilation();
    const bool blocked = RelayController::heaterSafetyBlocked();
    const bool temperatureChanged =
        isfinite(lastTemperature) != isfinite(simulatedTemperature) ||
        (isfinite(lastTemperature) && lastTemperature != simulatedTemperature);
    const bool humidityChanged =
        isfinite(lastHumidity) != isfinite(simulatedHumidity) ||
        (isfinite(lastHumidity) && lastHumidity != simulatedHumidity);
    const bool inputsChanged = temperatureChanged || humidityChanged;
    if (first || lastMode != testMode || lastHeater != heater ||
        lastHumidifier != humidifier || lastFan != fan || lastVent != vent ||
        lastTurner != simulatedTurner || lastBlocked != blocked || inputsChanged)
    {
        mqtt.publishSimulationStates();
        first = false;
        lastMode = testMode;
        lastHeater = heater;
        lastHumidifier = humidifier;
        lastFan = fan;
        lastVent = vent;
        lastTurner = simulatedTurner;
        lastBlocked = blocked;
        lastTemperature = simulatedTemperature;
        lastHumidity = simulatedHumidity;
    }
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
        setSimulationModeCommand(first);
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
        if (!setDryRunTemperature(temperature) || !setDryRunHumidity(humidity))
        {
            Serial.println("[TEST] Failed to apply simulated inputs.");
            return;
        }
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
        if (strcmp(first, "HEATER") == 0) setManualSimulatedOutput(SimulatedOutput::Heater, state);
        else if (strcmp(first, "HUMIDIFIER") == 0) setManualSimulatedOutput(SimulatedOutput::Humidifier, state);
        else if (strcmp(first, "FAN") == 0) setManualSimulatedOutput(SimulatedOutput::Fan, state);
        else if (strcmp(first, "VENT") == 0) setManualSimulatedOutput(SimulatedOutput::Ventilation, state);
        else if (strcmp(first, "TURNER") == 0) setManualSimulatedOutput(SimulatedOutput::Turner, state);
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
    Serial.println(simulationModeName());
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