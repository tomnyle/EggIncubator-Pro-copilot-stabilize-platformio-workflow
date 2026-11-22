#include "motor_limits.h"
#include <cassert>

int main()
{
    constexpr uint32_t maxTicks = 30000;
    assert(motorLimitResult(false, false, true, 1, maxTicks) == MotorResult::Running);
    assert(motorLimitResult(false, true, true, 1, maxTicks) == MotorResult::Running);
    assert(motorLimitResult(true, false, true, 1, maxTicks) == MotorResult::Reached);
    assert(motorLimitResult(false, true, false, 1, maxTicks) == MotorResult::Reached);
    assert(motorLimitResult(true, true, true, 1, maxTicks) == MotorResult::SwitchFault);
    assert(motorLimitResult(false, false, false, maxTicks - 1, maxTicks) == MotorResult::Running);
    assert(motorLimitResult(false, false, false, maxTicks, maxTicks) == MotorResult::Timeout);
}
