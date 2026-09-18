#include "system_state.h"

SystemState systemState;

void setFault(FaultCode code, bool active, const String &message)
{
    const uint32_t mask = static_cast<uint32_t>(code);

    if (code == WARN_MQTT_OFFLINE)
    {
        if (active) systemState.fault.warningMask |= mask;
        else systemState.fault.warningMask &= ~mask;
    }
    else
    {
        if (active) systemState.fault.activeMask |= mask;
        else systemState.fault.activeMask &= ~mask;
    }

    if (active && message.length() > 0)
    {
        systemState.fault.lastMessage = message;
    }

    systemState.fault.lastUpdateMs = millis();
}

bool hasFault(FaultCode code)
{
    return (systemState.fault.activeMask & static_cast<uint32_t>(code)) != 0;
}

bool hasCriticalFault()
{
    const uint32_t criticalMask =
        FAULT_SHT31_FAIL |
        FAULT_DS18B20_FAIL |
        FAULT_OVER_TEMP |
        FAULT_HUMIDITY_FAIL |
        FAULT_TURN_TIMEOUT |
        FAULT_LIMIT_INCONSISTENT |
        FAULT_DOOR_OPEN |
        FAULT_SYSTEM_ERROR;

    return (systemState.fault.activeMask & criticalMask) != 0;
}

const char *modeToString(SystemMode mode)
{
    switch (mode)
    {
        case SystemMode::OFF:
            return "OFF";
        case SystemMode::MANUAL:
            return "MANUAL";
        case SystemMode::AUTO:
            return "AUTO";
        case SystemMode::ALARM:
            return "ALARM";
        default:
            return "UNKNOWN";
    }
}

String activeFaultSummary()
{
    if (systemState.fault.activeMask == FAULT_NONE)
    {
        return "NONE";
    }

    String text;
    if (hasFault(FAULT_SHT31_FAIL)) text += "SHT31_FAIL,";
    if (hasFault(FAULT_DS18B20_FAIL)) text += "DS18B20_FAIL,";
    if (hasFault(FAULT_TEMP_DISAGREE)) text += "TEMP_DISAGREE,";
    if (hasFault(FAULT_OVER_TEMP)) text += "OVER_TEMP,";
    if (hasFault(FAULT_HUMIDITY_FAIL)) text += "HUM_FAIL,";
    if (hasFault(FAULT_TURN_TIMEOUT)) text += "TURN_TIMEOUT,";
    if (hasFault(FAULT_LIMIT_INCONSISTENT)) text += "LIMIT_INCONSISTENT,";
    if (hasFault(FAULT_DOOR_OPEN)) text += "DOOR_OPEN,";
    if (hasFault(FAULT_SYSTEM_ERROR)) text += "SYSTEM_ERROR,";
    if (hasFault(FAULT_WATER_LOW)) text += "WATER_LOW,";

    if (text.endsWith(","))
    {
        text.remove(text.length() - 1);
    }
    return text;
}
