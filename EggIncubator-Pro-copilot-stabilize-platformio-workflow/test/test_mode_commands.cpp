#include <cassert>
#include <cmath>
#include <string>
#include "Arduino.h"
#include "logger.h"
#include "relay_controller.h"

#define GPIO_MANAGER_H
#define SENSOR_MANAGER_H
#define SYSTEM_STATE_H
#define EGG_TURNER_MANAGER_H
#define NETWORK_SERVICE_H
#define MQTT_SERVICE_H

class GPIOManager { public: static void begin() {} };
class SensorManager
{
public:
    static float temperature;
    static float humidity;
    static bool ready;
    static void begin() {}
    static void update() {}
    static bool isSHT31Ready() { return ready; }
    static float getAirTemperature() { return temperature; }
    static float getAirHumidity() { return humidity; }
};
float SensorManager::temperature = 35.0f;
float SensorManager::humidity = 40.0f;
bool SensorManager::ready = true;
class SystemState
{
public:
    struct Alarm { bool buzzerActive = false; };
    static void begin() {}
    static Alarm getAlarmStatus() { return Alarm(); }
};
class EggTurnerManager
{
public:
    static void begin() {}
    static void update() {}
    static void disable() {}
    static bool isEnabled() { return false; }
    static uint16_t getTurningInterval() { return 120; }
    static void setTurningInterval(uint16_t) {}
    static uint16_t getTurnDuration() { return 5; }
    static void setTurnDuration(uint16_t) {}
    static uint32_t getTurnCount() { return 0; }
};
class NetworkService { public: void begin() {} void loop() {} };
class MQTTService
{
public:
    static int simulationPublishCount;
    void begin() {}
    void loop() {}
    void publishSimulationStates() { ++simulationPublishCount; }
    void publishRelayStates() {}
};
int MQTTService::simulationPublishCount = 0;
NetworkService network;
MQTTService mqtt;
#include "../src/main.cpp"

FakeSerial Serial;
uint32_t fakeMillis = 0;
int pinValues[40] = {};
void Logger::begin(uint32_t) {}
void Logger::info(const char *) {}
void Logger::warning(const char *) {}
void Logger::error(const char *) {}
void Logger::debug(const char *) {}

static void send(const std::string &command)
{
    Serial.input += command + "\n";
    processSerialCommands();
}

int main()
{
    setup();
    assert(testMode == TestMode::MANUAL);
    for (const char *name : {"QUAIL", "DUCK", "GOOSE", "CHICKEN"})
    {
        send(std::string("PROFILE ") + name);
        assert(std::string(IncubationProfile::currentCode()) == name);
    }
    send("PROFILE duck");
    assert(std::string(IncubationProfile::currentCode()) == "DUCK");
    send("PROFILE QUAIL EXTRA");
    send("PROFILE UNKNOWN");
    assert(std::string(IncubationProfile::currentCode()) == "DUCK");
    assert(testMode == TestMode::MANUAL);
    send("PROFILE CHICKEN");
    publishSimulationChanges();
    const int initialPublishCount = MQTTService::simulationPublishCount;
    publishSimulationChanges();
    assert(MQTTService::simulationPublishCount == initialPublishCount);
    send("SET HEATER ON");
    send("SET TURNER ON");
    assert(RelayController::simulatedHeater() && simulatedTurner);
    assert(isManualSimulationMode());
    assert(setManualSimulatedOutput(SimulatedOutput::Fan, true));
    assert(RelayController::simulatedFan());

    send("MODE DRYRUN");
    assert(testMode == TestMode::DRY_RUN);
    assert(!RelayController::simulatedHeater() && !simulatedTurner);
    assert(!RelayController::simulatedFan());
    assert(!setManualSimulatedOutput(SimulatedOutput::Fan, true));
    assert(std::isnan(simulatedTemperature) && std::isnan(simulatedHumidity));
    send("INPUT nan 50");
    send("INPUT 35 101");
    assert(std::isnan(simulatedTemperature) && std::isnan(simulatedHumidity));
    send("INPUT 35 40");
    assert(simulatedTemperature == 35 && simulatedHumidity == 40);
    loop();
    fakeMillis += 1000;
    loop();
    assert(RelayController::simulatedHeater() && RelayController::simulatedHumidifier());
    send("INPUT 39.5 40");
    assert(RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    send("INPUT 39.0 40");
    assert(RelayController::heaterSafetyBlocked());
    send("INPUT 38.5 40");
    assert(!RelayController::heaterSafetyBlocked());
    send("SET HEATER OFF");
    assert(!RelayController::simulatedHeater());

    send("MODE AUTO");
    assert(testMode == TestMode::AUTO);
    assert(!RelayController::simulatedHeater() && !RelayController::simulatedHumidifier());
    fakeMillis += 1000;
    loop();
    assert(RelayController::simulatedHeater() && RelayController::simulatedHumidifier());
    send("INPUT 60 80");
    assert(std::isnan(simulatedTemperature));

    send("MODE MANUAL");
    assert(!RelayController::simulatedHeater() && !RelayController::simulatedHumidifier());
    send("SET HEATER ON");
    assert(RelayController::simulatedHeater());
    SensorManager::temperature = 39.5f;
    loop();
    assert(RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    send("SET HEATER ON");
    assert(!RelayController::simulatedHeater());
    assert(!setManualSimulatedOutput(SimulatedOutput::Heater, true));
    SensorManager::temperature = 38.5f;
    loop();
    assert(!RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    send("SET HEATER ON");
    assert(RelayController::simulatedHeater());
    SensorManager::humidity = NAN;
    loop();
    assert(RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    send(std::string(100, 'x') + "MODE AUTO");
    assert(testMode == TestMode::MANUAL);
    assert(pinValues[PIN_HEATER_SSR] == RELAY_OFF);
    assert(pinValues[PIN_HUMIDIFIER] == RELAY_OFF);
    assert(pinValues[PIN_FAN_CIRCULATION] == RELAY_OFF);
    assert(pinValues[PIN_FAN_VENT] == RELAY_OFF);
}
