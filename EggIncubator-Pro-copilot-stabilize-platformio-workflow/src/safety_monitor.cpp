#include "safety_monitor.h"

#include <Preferences.h>
#include "core/system_state.h"
#include "logger.h"
#include "managers/egg_turner_manager.h"
#include "operating_mode.h"
#include "pins.h"
#include "safety_limits.h"
#include "sensor_manager.h"

namespace {
Preferences preferences;
bool storageReady = false;
bool alarmStored = false;

SensorFault currentSensorFault()
{
    return checkSensorSafety(SensorManager::isSHT31Ready(),
                             SensorManager::getAirTemperature(),
                             SensorManager::getAirHumidity());
}

bool switchesContradict()
{
    return digitalRead(PIN_LIMIT_HOME) == LOW && digitalRead(PIN_LIMIT_END) == LOW;
}
}

void SafetyMonitor::begin()
{
    storageReady = preferences.begin("rev3-safety", false);
    if (!storageReady)
    {
        SystemState::triggerAlarm(SystemState::ALARM_POWER_LOSS, "Alarm storage unavailable");
        return;
    }

    alarmStored = preferences.getBool("alarm", false);
    if (alarmStored)
        SystemState::triggerAlarm(SystemState::ALARM_POWER_LOSS,
                                  "Previous alarm: inspect and reset");
}

void SafetyMonitor::update()
{
    if (!SystemState::getAlarmStatus().alarmActive)
    {
        const SensorFault fault = currentSensorFault();
        if (fault == SensorFault::Invalid)
            SystemState::triggerAlarm(SystemState::ALARM_SENSOR_ERROR, "SHT31 unavailable or invalid");
        else if (fault == SensorFault::OverTemperature)
            SystemState::triggerAlarm(SystemState::ALARM_TEMP_HIGH, "Air temperature at or above 40 C");
        else if (switchesContradict())
            SystemState::triggerAlarm(SystemState::ALARM_MOTOR_ERROR, "Both motor limits active");
    }

    if (SystemState::getAlarmStatus().alarmActive)
    {
        OperatingModeController::update();
        if (storageReady && !alarmStored)
        {
            if (preferences.putBool("alarm", true) == 1)
                alarmStored = true;
            else
                Logger::error("Failed to persist alarm; do not operate device");
        }
    }
}

bool SafetyMonitor::clearAlarm()
{
    if (!storageReady || currentSensorFault() != SensorFault::None ||
        switchesContradict() || EggTurnerManager::isTurnerRunning())
    {
        Logger::error("Alarm reset refused: safety condition still present");
        return false;
    }

    if (preferences.putBool("alarm", false) != 1)
    {
        Logger::error("Alarm reset refused: unable to write storage");
        return false;
    }
    alarmStored = false;
    SystemState::clearAlarm();
    Logger::info("Alarm cleared; operating mode remains OFF");
    return true;
}

bool SafetyMonitor::isAlarmActive()
{
    return SystemState::getAlarmStatus().alarmActive;
}

String SafetyMonitor::alarmMessage()
{
    return SystemState::getAlarmStatus().alarmMessage;
}
