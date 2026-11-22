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
#define INCUBATION_PROFILE_H

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
class MQTTService { public: void begin() {} void loop() {} };
NetworkService network;
MQTTService mqtt;
enum class IncubationProfileType { CHICKEN };
struct FakeProfile
{
    const char *name = "Chicken";
    uint16_t totalDays = 21;
    float tempSetpointC = 37.5f;
    float humidityLockdown = 68.0f;
    float humidityIncubation = 55.0f;
    uint16_t turnIntervalMin = 120;
    uint16_t turnDurationSec = 5;
};
class IncubationProfile
{
public:
    static void begin(IncubationProfileType, uint32_t) {}
    static const FakeProfile &current() { static FakeProfile p; return p; }
    static uint16_t currentDay(uint32_t) { return 1; }
    static bool isLockdown(uint32_t) { return false; }
};

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
    send("SET HEATER ON");
    send("SET TURNER ON");
    assert(RelayController::simulatedHeater() && simulatedTurner);

    send("MODE DRYRUN");
    assert(testMode == TestMode::DRY_RUN);
    assert(!RelayController::simulatedHeater() && !simulatedTurner);
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
