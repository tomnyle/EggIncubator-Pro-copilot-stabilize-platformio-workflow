#ifndef SAFETY_LOGIC_H
#define SAFETY_LOGIC_H

#include <stdint.h>
#include <string.h>

namespace SafetyLogic
{
inline bool canSwitch(uint32_t nowMs, uint32_t lastSwitchMs, uint32_t minIntervalMs)
{
    return (nowMs - lastSwitchMs) >= minIntervalMs;
}

inline bool heaterCommand(bool currentOn, float tempC, float targetC, float hysteresisC)
{
    if (!currentOn)
    {
        return tempC <= (targetC - hysteresisC);
    }
    return tempC < (targetC + hysteresisC);
}

inline bool humidifierCommand(bool currentOn, float humidity, float target, float hysteresis)
{
    if (!currentOn)
    {
        return humidity <= (target - hysteresis);
    }
    return humidity < (target + hysteresis);
}

inline uint16_t incubationDayFromElapsed(uint32_t elapsedSeconds)
{
    return static_cast<uint16_t>((elapsedSeconds / 86400UL) + 1UL);
}

inline float applyTargetUpdate(float current, float requested, float minValue, float maxValue)
{
    if (requested < minValue || requested > maxValue)
    {
        return current;
    }
    return requested;
}

enum class RequestedMode : uint8_t
{
    INVALID = 0,
    OFF,
    MANUAL,
    AUTO,
    ALARM
};

inline RequestedMode parseMode(const char *payload)
{
    if (strcmp(payload, "OFF") == 0) return RequestedMode::OFF;
    if (strcmp(payload, "MANUAL") == 0) return RequestedMode::MANUAL;
    if (strcmp(payload, "AUTO") == 0) return RequestedMode::AUTO;
    if (strcmp(payload, "ALARM") == 0) return RequestedMode::ALARM;
    return RequestedMode::INVALID;
}

inline bool manualRelayAllowed(bool isManualMode, bool hasCriticalFault)
{
    return isManualMode && !hasCriticalFault;
}

enum class CommandRoute : uint8_t
{
    UNKNOWN = 0,
    MODE_SET,
    START_SET,
    TARGET_TEMP_SET,
    TARGET_HUM_SET,
    PROFILE_SET,
    RELAY_HEATER_SET,
    RELAY_HUMIDIFIER_SET,
    RELAY_FAN_SET,
    RELAY_VENT_SET,
    TURNER_SET,
    TURNER_COMMAND
};

inline bool textEndsWith(const char *text, const char *suffix)
{
    const size_t textLen = strlen(text);
    const size_t suffixLen = strlen(suffix);
    if (suffixLen > textLen) return false;
    return strcmp(text + textLen - suffixLen, suffix) == 0;
}

inline CommandRoute classifyCommandTopic(const char *topic)
{
    if (textEndsWith(topic, "/mode/set")) return CommandRoute::MODE_SET;
    if (textEndsWith(topic, "/start/set")) return CommandRoute::START_SET;
    if (textEndsWith(topic, "/target/temperature/set")) return CommandRoute::TARGET_TEMP_SET;
    if (textEndsWith(topic, "/target/humidity/set")) return CommandRoute::TARGET_HUM_SET;
    if (textEndsWith(topic, "/profile/set")) return CommandRoute::PROFILE_SET;
    if (textEndsWith(topic, "/relay/heater/set")) return CommandRoute::RELAY_HEATER_SET;
    if (textEndsWith(topic, "/relay/humidifier/set")) return CommandRoute::RELAY_HUMIDIFIER_SET;
    if (textEndsWith(topic, "/relay/fan/set")) return CommandRoute::RELAY_FAN_SET;
    if (textEndsWith(topic, "/relay/vent/set")) return CommandRoute::RELAY_VENT_SET;
    if (textEndsWith(topic, "/turner/set")) return CommandRoute::TURNER_SET;
    if (textEndsWith(topic, "/turner/command")) return CommandRoute::TURNER_COMMAND;
    return CommandRoute::UNKNOWN;
}

enum class TurnerState : uint8_t
{
    IDLE,
    MOVE_HOME,
    MOVE_END,
    WAIT,
    FAULT
};

struct TurnerInputs
{
    bool atHome;
    bool atEnd;
    bool doorOpen;
    bool timeout;
    bool command;
    bool nextMoveToEnd;
};

inline TurnerState stepTurnerState(TurnerState current, const TurnerInputs &in)
{
    if (in.atHome && in.atEnd) return TurnerState::FAULT;
    if (in.doorOpen) return (current == TurnerState::MOVE_HOME || current == TurnerState::MOVE_END) ? TurnerState::WAIT : current;
    if (in.timeout) return TurnerState::FAULT;

    switch (current)
    {
        case TurnerState::IDLE:
        case TurnerState::WAIT:
            return in.command ? (in.nextMoveToEnd ? TurnerState::MOVE_END : TurnerState::MOVE_HOME) : current;
        case TurnerState::MOVE_HOME:
            return in.atHome ? TurnerState::WAIT : TurnerState::MOVE_HOME;
        case TurnerState::MOVE_END:
            return in.atEnd ? TurnerState::WAIT : TurnerState::MOVE_END;
        case TurnerState::FAULT:
        default:
            return TurnerState::FAULT;
    }
}

inline uint32_t setFaultBit(uint32_t mask, uint32_t bit, bool active)
{
    return active ? (mask | bit) : (mask & ~bit);
}
}

#endif
