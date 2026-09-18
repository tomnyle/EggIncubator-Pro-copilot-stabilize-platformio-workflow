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
    assert(SafetyLogic::applyTargetUpdate(37.5f, 38.0f, 34.0f, 39.5f) == 38.0f);
    assert(SafetyLogic::applyTargetUpdate(37.5f, 45.0f, 34.0f, 39.5f) == 37.5f);
    assert(SafetyLogic::applyTargetUpdate(60.0f, 65.0f, 30.0f, 85.0f) == 65.0f);
    assert(SafetyLogic::applyTargetUpdate(60.0f, 10.0f, 30.0f, 85.0f) == 60.0f);
    assert(SafetyLogic::parseMode("AUTO") == SafetyLogic::RequestedMode::AUTO);
    assert(SafetyLogic::parseMode("INVALID") == SafetyLogic::RequestedMode::INVALID);
    assert(SafetyLogic::manualRelayAllowed(true, false));
    assert(!SafetyLogic::manualRelayAllowed(true, true));
    assert(SafetyLogic::classifyCommandTopic("eggincubator/mode/set") == SafetyLogic::CommandRoute::MODE_SET);
    assert(SafetyLogic::classifyCommandTopic("eggincubator/target/temperature/set") == SafetyLogic::CommandRoute::TARGET_TEMP_SET);
    assert(SafetyLogic::classifyCommandTopic("eggincubator/relay/heater/set") == SafetyLogic::CommandRoute::RELAY_HEATER_SET);
    assert(SafetyLogic::classifyCommandTopic("eggincubator/turner/command") == SafetyLogic::CommandRoute::TURNER_COMMAND);
    assert(SafetyLogic::classifyCommandTopic("eggincubator/unknown") == SafetyLogic::CommandRoute::UNKNOWN);

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
