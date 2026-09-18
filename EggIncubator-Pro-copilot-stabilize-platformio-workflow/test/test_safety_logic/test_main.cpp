#include <cassert>
#include "safety_logic.h"

int main()
{
    assert(SafetyLogic::heaterCommand(false, 37.0f, 37.5f, 0.3f));
    assert(!SafetyLogic::heaterCommand(false, 37.4f, 37.5f, 0.3f));
    assert(!SafetyLogic::heaterCommand(true, 38.0f, 37.5f, 0.3f));

    assert(SafetyLogic::humidifierCommand(false, 50.0f, 60.0f, 4.0f));
    assert(!SafetyLogic::humidifierCommand(false, 59.0f, 60.0f, 4.0f));
    assert(!SafetyLogic::humidifierCommand(true, 65.0f, 60.0f, 4.0f));

    uint32_t mask = 0;
    mask = SafetyLogic::setFaultBit(mask, 0x01, true);
    assert(mask == 0x01);
    mask = SafetyLogic::setFaultBit(mask, 0x04, true);
    assert(mask == 0x05);
    mask = SafetyLogic::setFaultBit(mask, 0x01, false);
    assert(mask == 0x04);

    assert(SafetyLogic::incubationDayFromElapsed(0) == 1);
    assert(SafetyLogic::incubationDayFromElapsed(86400) == 2);
    assert(SafetyLogic::incubationDayFromElapsed(18UL * 86400UL) == 19);

    using SafetyLogic::TurnerInputs;
    using SafetyLogic::TurnerState;

    TurnerInputs in{false, false, false, false, true, true};
    auto state = SafetyLogic::stepTurnerState(TurnerState::IDLE, in);
    assert(state == TurnerState::MOVE_END);

    in = {false, false, true, false, false, true};
    state = SafetyLogic::stepTurnerState(TurnerState::MOVE_END, in);
    assert(state == TurnerState::WAIT);

    in = {false, false, false, true, false, true};
    state = SafetyLogic::stepTurnerState(TurnerState::MOVE_END, in);
    assert(state == TurnerState::FAULT);

    return 0;
}
