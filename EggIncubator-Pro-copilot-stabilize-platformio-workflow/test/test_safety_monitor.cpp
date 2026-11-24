#include <cassert>
#include <string>

#define SYSTEM_STATE_H
#define LOGGER_H
#define EGG_TURNER_MANAGER_H
#define PINS_H
#define SENSOR_MANAGER_H

bool testStorageAvailable = true;
bool testStoredAlarm = false;
static bool ready = true, home = false, end = false, moving = false;
static bool stopped = false;
static float temperature = 37.5f, humidity = 55.0f;
constexpr int PIN_LIMIT_HOME = 18, PIN_LIMIT_END = 19;
int digitalRead(int pin) { return (pin == PIN_LIMIT_HOME ? home : end) ? 0 : 1; }

struct SystemState
{
    enum AlarmType { ALARM_POWER_LOSS, ALARM_SENSOR_ERROR, ALARM_TEMP_HIGH, ALARM_MOTOR_ERROR };
    struct Alarm { bool alarmActive = false; std::string alarmMessage; };
    static Alarm alarm;
    static Alarm getAlarmStatus() { return alarm; }
    static void triggerAlarm(AlarmType, const char *message) { alarm = {true, message}; }
    static void clearAlarm() { alarm = {}; }
};
SystemState::Alarm SystemState::alarm;
struct Logger { static void error(const char*) {} static void info(const char*) {} };
struct EggTurnerManager { static bool isTurnerRunning() { return moving; } };
#include "../include/operating_mode.h"
void OperatingModeController::update() { stopped = true; }
struct SensorManager
{
    static bool isSHT31Ready() { return ready; }
    static float getAirTemperature() { return temperature; }
    static float getAirHumidity() { return humidity; }
};

#include "../src/safety_monitor.cpp"

int main()
{
    SafetyMonitor::begin();
    SafetyMonitor::update();
    assert(!SafetyMonitor::isAlarmActive());
    assert(SafetyMonitor::canEnableThermalOutput());

    temperature = 40.0f;
    assert(!SafetyMonitor::canEnableThermalOutput());
    SafetyMonitor::update();
    assert(SafetyMonitor::isAlarmActive() && stopped && testStoredAlarm);
    assert(!SafetyMonitor::clearAlarm());
    temperature = 39.9f;
    assert(!SafetyMonitor::canEnableThermalOutput());
    SafetyMonitor::update();
    assert(SafetyMonitor::isAlarmActive());
    moving = true;
    assert(!SafetyMonitor::clearAlarm());
    moving = false;
    assert(SafetyMonitor::clearAlarm() && !testStoredAlarm);
    assert(SafetyMonitor::canEnableThermalOutput());

    ready = false;
    assert(!SafetyMonitor::canEnableThermalOutput());
    SafetyMonitor::update();
    assert(SafetyMonitor::isAlarmActive() && testStoredAlarm);
    assert(!SafetyMonitor::clearAlarm());
    ready = true;
    SystemState::clearAlarm();
    SafetyMonitor::begin();
    assert(SafetyMonitor::isAlarmActive());
    home = end = true;
    assert(!SafetyMonitor::clearAlarm());
    home = end = false;
    testStorageAvailable = false;
    assert(!SafetyMonitor::clearAlarm());
    testStorageAvailable = true;
    assert(SafetyMonitor::clearAlarm());
}
