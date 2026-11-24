#include "operating_mode.h"

#include "app_config.h"
#include "core/system_state.h"
#include "incubation_profile.h"
#include "logger.h"
#include "managers/egg_turner_manager.h"
#include "managers/humidity_manager.h"
#include "managers/temperature_manager.h"
#include "relay_controller.h"

namespace {
OperatingMode mode = OperatingMode::Off;

void stopOutputs()
{
    TemperatureManager::disable();
    HumidityManager::disable();
    EggTurnerManager::disable();
    RelayController::allOff();
}
}

OperatingMode OperatingModeController::current() { return mode; }

const char *OperatingModeController::name()
{
    switch (mode)
    {
        case OperatingMode::Manual: return "MANUAL";
        case OperatingMode::Auto: return "AUTO";
        default: return "OFF";
    }
}

bool OperatingModeController::set(OperatingMode requested)
{
    if (requested != OperatingMode::Off && SystemState::getAlarmStatus().alarmActive)
    {
        Logger::error("Mode rejected: alarm active");
        return false;
    }

    if (requested == OperatingMode::Auto &&
        (!REV3_ALLOW_THERMAL_OUTPUTS || !REV3_ALLOW_TURNER))
    {
        Logger::error("AUTO rejected: REV3 test safety locks are active");
        return false;
    }

    if (mode == requested)
        return true;

    stopOutputs();
    mode = requested;
    if (mode == OperatingMode::Auto)
    {
        TemperatureManager::enable();
        HumidityManager::enable();
        RelayController::fan(true);
        if (!IncubationProfile::isLockdown(millis()))
        {
            EggTurnerManager::enable();
            if (!EggTurnerManager::isEnabled())
            {
                stopOutputs();
                mode = OperatingMode::Off;
                Logger::error("AUTO rejected: turner could not be enabled");
                return false;
            }
        }
    }
    Logger::info(("Operating mode: " + String(name())).c_str());
    return true;
}

void OperatingModeController::update()
{
    if (mode != OperatingMode::Off && SystemState::getAlarmStatus().alarmActive)
    {
        stopOutputs();
        mode = OperatingMode::Off;
        Logger::error("Alarm: switched operating mode to OFF");
    }
}
