#include <cassert>
#include <string>

#define SYSTEM_STATE_H
#define INCUBATION_PROFILE_H
#define LOGGER_H
#define TEMPERATURE_MANAGER_H
#define HUMIDITY_MANAGER_H
#define EGG_TURNER_MANAGER_H
#define RELAY_CONTROLLER_H

using String = std::string;
unsigned long millis() { return 0; }

static bool alarmActive = false;
static bool lockedDown = false;
static bool turnerAvailable = true;
static bool heater = false, humidifier = false, fan = false, vent = false;
static bool tempControl = false, humidityControl = false, turner = false;

struct SystemState {
    struct Alarm { bool alarmActive; };
    static Alarm getAlarmStatus() { return {::alarmActive}; }
};
struct IncubationProfile {
    static bool isLockdown(unsigned long) { return lockedDown; }
};
struct Logger {
    static void info(const char*) {}
    static void error(const char*) {}
};
struct TemperatureManager {
    static void enable() { tempControl = true; }
    static void disable() { tempControl = false; heater = false; }
};
struct HumidityManager {
    static void enable() { humidityControl = true; }
    static void disable() { humidityControl = false; humidifier = false; }
};
struct EggTurnerManager {
    static void enable() { turner = turnerAvailable; }
    static void disable() { turner = false; }
    static bool isEnabled() { return turner; }
};
struct RelayController {
    static void fan(bool state) { ::fan = state; }
    static void allOff() { heater = humidifier = ::fan = vent = false; }
};

#include "../src/operating_mode.cpp"

int main()
{
    assert(OperatingModeController::current() == OperatingMode::Off);
    assert(OperatingModeController::set(OperatingMode::Manual));
    heater = humidifier = fan = vent = turner = true;
    assert(OperatingModeController::set(OperatingMode::Off));
    assert(!heater && !humidifier && !fan && !vent && !turner);

    alarmActive = true;
    assert(!OperatingModeController::set(OperatingMode::Manual));
    alarmActive = false;

#if REV3_ALLOW_THERMAL_OUTPUTS && REV3_ALLOW_TURNER
    assert(OperatingModeController::set(OperatingMode::Auto));
    assert(tempControl && humidityControl && fan && turner);
    alarmActive = true;
    OperatingModeController::update();
    assert(OperatingModeController::current() == OperatingMode::Off);
    assert(!tempControl && !humidityControl && !fan && !turner);
    alarmActive = false;
    turnerAvailable = false;
    assert(!OperatingModeController::set(OperatingMode::Auto));
    assert(OperatingModeController::current() == OperatingMode::Off);
    assert(!tempControl && !humidityControl && !fan);
    lockedDown = true;
    assert(OperatingModeController::set(OperatingMode::Auto));
    assert(tempControl && humidityControl && fan && !turner);
#else
    assert(!OperatingModeController::set(OperatingMode::Auto));
    assert(OperatingModeController::current() == OperatingMode::Off);
#endif
}
