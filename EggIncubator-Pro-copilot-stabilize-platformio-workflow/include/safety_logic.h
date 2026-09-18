#ifndef SAFETY_LOGIC_H
#define SAFETY_LOGIC_H

#include <stdint.h>

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
