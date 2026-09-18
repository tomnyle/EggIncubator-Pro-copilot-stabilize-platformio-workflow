#include "egg_turner_manager.h"

#include "relay_controller.h"
#include "sensor_manager.h"
#include "pins.h"
#include "system_state.h"
#include "incubation_profile.h"

uint32_t EggTurnerManager::turningIntervalMs = 120000UL;
uint32_t EggTurnerManager::turnDurationMs = 5000UL;

bool EggTurnerManager::controlEnabled = false;
bool EggTurnerManager::manualRequest = false;
EggTurnerManager::TurnerState EggTurnerManager::state = EggTurnerManager::TurnerState::IDLE;
uint32_t EggTurnerManager::stateStartMs = 0;
uint32_t EggTurnerManager::lastTurnTime = 0;
uint32_t EggTurnerManager::totalTurns = 0;
bool EggTurnerManager::nextMoveToEnd = true;

namespace
{
constexpr uint32_t MAX_MOVE_TIMEOUT_MS = 12000;
}

void EggTurnerManager::begin()
{
    pinMode(PIN_LIMIT_HOME, INPUT_PULLUP);
    pinMode(PIN_LIMIT_END, INPUT_PULLUP);

    stopMotor();
    state = TurnerState::IDLE;
    stateStartMs = millis();
}

void EggTurnerManager::update()
{
    const uint32_t now = millis();

    const bool atHome = (digitalRead(PIN_LIMIT_HOME) == INPUT_ACTIVE);
    const bool atEnd = (digitalRead(PIN_LIMIT_END) == INPUT_ACTIVE);

    if (atHome && atEnd)
    {
        setFault(FAULT_LIMIT_INCONSISTENT, true, "Both limit switches active");
        state = TurnerState::FAULT;
        stopMotor();
    }
    else
    {
        setFault(FAULT_LIMIT_INCONSISTENT, false);
    }

    if (SensorManager::isDoorOpen())
    {
        stopMotor();
        setFault(FAULT_DOOR_OPEN, true, "Door opened while turner active");
        if (state == TurnerState::MOVE_TO_HOME || state == TurnerState::MOVE_TO_END)
        {
            state = TurnerState::WAIT;
        }
        return;
    }

    if (!controlEnabled || hasFault(FAULT_TURN_TIMEOUT) || hasFault(FAULT_LIMIT_INCONSISTENT))
    {
        stopMotor();
        if (state != TurnerState::FAULT)
        {
            state = TurnerState::IDLE;
        }
        return;
    }

    if (IncubationProfile::isLockdown(IncubationProfile::getStartEpoch() + (millis() / 1000UL)))
    {
        stopMotor();
        state = TurnerState::IDLE;
        return;
    }

    switch (state)
    {
        case TurnerState::IDLE:
        case TurnerState::WAIT:
        {
            const bool due = (now - lastTurnTime) >= turningIntervalMs;
            if (manualRequest || due)
            {
                manualRequest = false;
                if (nextMoveToEnd) startMoveToEnd();
                else startMoveToHome();
            }
            break;
        }

        case TurnerState::MOVE_TO_HOME:
            if (atHome)
            {
                stopMotor();
                state = TurnerState::WAIT;
                lastTurnTime = now;
                totalTurns++;
                nextMoveToEnd = true;
                setFault(FAULT_TURN_TIMEOUT, false);
            }
            else if ((now - stateStartMs) > MAX_MOVE_TIMEOUT_MS)
            {
                stopMotor();
                state = TurnerState::FAULT;
                setFault(FAULT_TURN_TIMEOUT, true, "Turner timeout to HOME");
            }
            break;

        case TurnerState::MOVE_TO_END:
            if (atEnd)
            {
                stopMotor();
                state = TurnerState::WAIT;
                lastTurnTime = now;
                totalTurns++;
                nextMoveToEnd = false;
                setFault(FAULT_TURN_TIMEOUT, false);
            }
            else if ((now - stateStartMs) > MAX_MOVE_TIMEOUT_MS)
            {
                stopMotor();
                state = TurnerState::FAULT;
                setFault(FAULT_TURN_TIMEOUT, true, "Turner timeout to END");
            }
            break;

        case TurnerState::FAULT:
        default:
            stopMotor();
            break;
    }

    systemState.output.motorEnable = (state == TurnerState::MOVE_TO_HOME || state == TurnerState::MOVE_TO_END);
}

void EggTurnerManager::setTurningInterval(uint16_t intervalMinutes)
{
    turningIntervalMs = static_cast<uint32_t>(intervalMinutes) * 60000UL;
}

uint16_t EggTurnerManager::getTurningInterval()
{
    return static_cast<uint16_t>(turningIntervalMs / 60000UL);
}

void EggTurnerManager::setTurnDuration(uint16_t durationSeconds)
{
    turnDurationMs = static_cast<uint32_t>(durationSeconds) * 1000UL;
}

uint16_t EggTurnerManager::getTurnDuration()
{
    return static_cast<uint16_t>(turnDurationMs / 1000UL);
}

void EggTurnerManager::manualTurn()
{
    manualRequest = true;
}

void EggTurnerManager::requestMoveHome()
{
    manualRequest = false;
    startMoveToHome();
}

void EggTurnerManager::requestMoveEnd()
{
    manualRequest = false;
    startMoveToEnd();
}

void EggTurnerManager::enable()
{
    controlEnabled = true;
    systemState.incubation.turningEnabled = true;
}

void EggTurnerManager::disable()
{
    controlEnabled = false;
    systemState.incubation.turningEnabled = false;
    stopMotor();
    state = TurnerState::IDLE;
}

bool EggTurnerManager::isEnabled()
{
    return controlEnabled;
}

bool EggTurnerManager::isTurnerRunning()
{
    return state == TurnerState::MOVE_TO_HOME || state == TurnerState::MOVE_TO_END;
}

uint32_t EggTurnerManager::getLastTurnTime() { return lastTurnTime; }

uint32_t EggTurnerManager::getTimeUntilNextTurn()
{
    const uint32_t now = millis();
    if (now - lastTurnTime >= turningIntervalMs) return 0;
    return turningIntervalMs - (now - lastTurnTime);
}

uint32_t EggTurnerManager::getTurnCount() { return totalTurns; }

const char* EggTurnerManager::getStateName()
{
    switch (state)
    {
        case TurnerState::IDLE: return "IDLE";
        case TurnerState::MOVE_TO_HOME: return "MOVE_TO_HOME";
        case TurnerState::MOVE_TO_END: return "MOVE_TO_END";
        case TurnerState::WAIT: return "WAIT";
        case TurnerState::FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

void EggTurnerManager::stopMotor()
{
    RelayController::motor(false, false, false);
}

void EggTurnerManager::startMoveToHome()
{
    if (SensorManager::isDoorOpen())
    {
        return;
    }

    state = TurnerState::MOVE_TO_HOME;
    stateStartMs = millis();
    RelayController::motor(true, true, false);
}

void EggTurnerManager::startMoveToEnd()
{
    if (SensorManager::isDoorOpen())
    {
        return;
    }

    state = TurnerState::MOVE_TO_END;
    stateStartMs = millis();
    RelayController::motor(true, false, true);
}
