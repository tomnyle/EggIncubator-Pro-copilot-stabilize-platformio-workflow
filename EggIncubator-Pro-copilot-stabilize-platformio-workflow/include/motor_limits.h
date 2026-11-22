#pragma once

#include <stdint.h>

enum class MotorResult : uint8_t { Idle, Running, Reached, Timeout, SwitchFault };

inline __attribute__((always_inline)) MotorResult motorLimitResult(
    bool home, bool end, bool targetHome, uint32_t ticks, uint32_t maxTicks)
{
    if (home && end)
        return MotorResult::SwitchFault;
    if (targetHome ? home : end)
        return MotorResult::Reached;
    if (ticks >= maxTicks)
        return MotorResult::Timeout;
    return MotorResult::Running;
}
