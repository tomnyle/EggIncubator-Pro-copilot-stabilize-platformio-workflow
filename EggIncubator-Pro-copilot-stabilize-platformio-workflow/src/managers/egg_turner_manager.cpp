#include "egg_turner_manager.h"

#include "relay_controller.h"
#include "logger.h"
#include "pins.h"
#include "core/system_state.h"
#include "motor_limits.h"
#include "app_config.h"
#include <soc/gpio_struct.h>

namespace {
static_assert(PIN_DM542_STEP == 32 && PIN_DM542_ENABLE == 23,
              "ISR GPIO registers must match REV3 motor pins");
portMUX_TYPE motorMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *motorTimer = nullptr;
volatile MotorResult motorResult = MotorResult::Idle;
volatile uint32_t motorTicks = 0;
volatile uint8_t targetLimit = PIN_LIMIT_HOME;
volatile bool pulseHigh = false;

// Runs independently of network and sensor calls so a blocked main loop
// cannot leave the step signal running past the limit or travel timeout.
void IRAM_ATTR motorTimerInterrupt()
{
    portENTER_CRITICAL_ISR(&motorMux);
    if (motorResult == MotorResult::Running)
    {
        const uint32_t inputs = GPIO.in;
        const bool home = !(inputs & (1UL << PIN_LIMIT_HOME));
        const bool end = !(inputs & (1UL << PIN_LIMIT_END));
        ++motorTicks;
        motorResult = motorLimitResult(home, end, targetLimit == PIN_LIMIT_HOME,
                                       motorTicks,
                                       DM542_MAX_TRAVEL_MS * 2000UL / DM542_STEP_INTERVAL_US);

        if (motorResult != MotorResult::Running)
        {
            GPIO.out1_w1tc.val = 1UL << (PIN_DM542_STEP - 32);
            GPIO.out_w1tc = 1UL << PIN_DM542_ENABLE;
            pulseHigh = false;
        }
        else if (motorTicks == 1)
        {
            // Allow the direction and enable signals to settle before the first pulse.
        }
        else if (pulseHigh)
        {
            GPIO.out1_w1tc.val = 1UL << (PIN_DM542_STEP - 32);
            pulseHigh = false;
        }
        else
        {
            GPIO.out1_w1ts.val = 1UL << (PIN_DM542_STEP - 32);
            pulseHigh = true;
        }
    }
    portEXIT_CRITICAL_ISR(&motorMux);
}
}

/******************************************************
 * Static Member Initialization
 ******************************************************/

uint32_t EggTurnerManager::turningIntervalMs = 120000UL;  // 2 minutes (default)
uint32_t EggTurnerManager::turnDurationMs = 5000UL;       // 5 seconds (default)

bool EggTurnerManager::controlEnabled = false;
bool EggTurnerManager::turnerRunning = false;
bool EggTurnerManager::oneShot = false;

uint32_t EggTurnerManager::lastTurnTime = 0;
uint32_t EggTurnerManager::turnStartTime = 0;
uint32_t EggTurnerManager::totalTurns = 0;

uint8_t EggTurnerManager::stepPin = PIN_DM542_STEP;
uint8_t EggTurnerManager::directionPin = PIN_DM542_DIR;
uint8_t EggTurnerManager::enablePin = PIN_DM542_ENABLE;

EggTurnerManager::TurnDirection EggTurnerManager::lastTurnDirection = LEFT;

/******************************************************
 * Initialize Egg Turner Manager
 ******************************************************/

void EggTurnerManager::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Egg Turner Manager Started");
    Logger::info("--------------------------------");

    // Setup DM542 pulse, direction and enable pins.
    pinMode(stepPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
    pinMode(enablePin, OUTPUT);

    // Ensure motor is stopped
    digitalWrite(stepPin, LOW);
    digitalWrite(directionPin, LOW);
    digitalWrite(enablePin, DM542_ENABLE_INACTIVE);

    motorTimer = timerBegin(0, 80, true);
    if (motorTimer == nullptr)
    {
        Logger::error("DM542 timer unavailable; turner disabled");
        return;
    }
    timerAttachInterrupt(motorTimer, &motorTimerInterrupt, true);
    timerAlarmWrite(motorTimer, DM542_STEP_INTERVAL_US / 2, true);
    timerAlarmEnable(motorTimer);

    Logger::info(
        ("Turning Interval: " + String(turningIntervalMs / 60000.0f, 1) + " minutes").c_str());
    Logger::info(
        ("Turn Duration: " + String(turnDurationMs / 1000.0f, 1) + " seconds").c_str());
}

/******************************************************
 * Update - Check if turning is needed
 ******************************************************/

void EggTurnerManager::update()
{
    uint32_t now = millis();

    if (turnerRunning)
    {
        portENTER_CRITICAL(&motorMux);
        const MotorResult result = motorResult;
        portEXIT_CRITICAL(&motorMux);
        if (result != MotorResult::Running)
        {
            stopTurner();
            turnerRunning = false;
            if (result == MotorResult::Reached)
            {
                totalTurns++;
                if (oneShot)
                    controlEnabled = false;
                Logger::info("Turn completed at limit switch");
            }
            else
            {
                controlEnabled = false;
                SystemState::triggerAlarm(SystemState::ALARM_MOTOR_ERROR,
                                          result == MotorResult::Timeout ?
                                          "Turner travel timeout" : "Turner limit switch fault");
            }
        }
    }
    else if (controlEnabled)
    {
        // Check if time for next turn
        uint32_t timeSinceLastTurn = now - lastTurnTime;

        if (timeSinceLastTurn >= turningIntervalMs)
        {
            performTurn();
        }
    }
}

/******************************************************
 * Set Turning Interval (minutes)
 ******************************************************/

void EggTurnerManager::setTurningInterval(uint16_t intervalMinutes)
{
    turningIntervalMs = static_cast<uint32_t>(intervalMinutes) * 60000UL;  // Convert to milliseconds

    Logger::info(
        ("Turning Interval Set: " + String(intervalMinutes) + " minutes").c_str());
}

/******************************************************
 * Get Turning Interval (minutes)
 ******************************************************/

uint16_t EggTurnerManager::getTurningInterval()
{
    return static_cast<uint16_t>(turningIntervalMs / 60000UL);
}

/******************************************************
 * Set Turn Duration (seconds)
 ******************************************************/

void EggTurnerManager::setTurnDuration(uint16_t durationSeconds)
{
    turnDurationMs = static_cast<uint32_t>(durationSeconds) * 1000UL;  // Convert to milliseconds

    Logger::info(
        ("Turn Duration Set: " + String(durationSeconds) + " seconds").c_str());
}

/******************************************************
 * Get Turn Duration (seconds)
 ******************************************************/

uint16_t EggTurnerManager::getTurnDuration()
{
    return static_cast<uint16_t>(turnDurationMs / 1000UL);
}

/******************************************************
 * Manual Turn - Perform immediately
 ******************************************************/

void EggTurnerManager::manualTurn()
{
    if (turnerRunning)
        return;
    enable();
    if (!controlEnabled)
        return;
    oneShot = true;
    performTurn();
}

/******************************************************
 * Enable Egg Turning
 ******************************************************/

void EggTurnerManager::enable()
{
    if (!controlEnabled)
    {
        if (motorTimer == nullptr || SystemState::getAlarmStatus().alarmActive)
        {
            Logger::error("Cannot enable turner: timer unavailable or alarm active");
            return;
        }
        controlEnabled = true;
        oneShot = false;
        lastTurnTime = millis();
        turnerRunning = false;

        Logger::info("Egg Turning ENABLED");
    }
}

/******************************************************
 * Disable Egg Turning
 ******************************************************/

void EggTurnerManager::disable()
{
    if (controlEnabled)
    {
        controlEnabled = false;

        // Stop motor if running
        if (turnerRunning)
        {
            stopTurner();
            turnerRunning = false;
        }

        Logger::info("Egg Turning DISABLED");
    }
}

/******************************************************
 * Is Egg Turning Enabled
 ******************************************************/

bool EggTurnerManager::isEnabled()
{
    return controlEnabled;
}

/******************************************************
 * Check if Turner is Currently Running
 ******************************************************/

bool EggTurnerManager::isTurnerRunning()
{
    return turnerRunning;
}

/******************************************************
 * Get Last Turn Time (ms since start)
 ******************************************************/

uint32_t EggTurnerManager::getLastTurnTime()
{
    return lastTurnTime;
}

/******************************************************
 * Get Time Until Next Turn (ms)
 ******************************************************/

uint32_t EggTurnerManager::getTimeUntilNextTurn()
{
    uint32_t now = millis();
    uint32_t timeSinceLastTurn = now - lastTurnTime;

    if (timeSinceLastTurn >= turningIntervalMs)
    {
        return 0;  // Turn is due now
    }

    return turningIntervalMs - timeSinceLastTurn;
}

/******************************************************
 * Get Total Turn Count
 ******************************************************/

uint32_t EggTurnerManager::getTurnCount()
{
    return totalTurns;
}

/******************************************************
 * Perform a Single Turn
 ******************************************************/

void EggTurnerManager::performTurn()
{
    if (turnerRunning)
    {
        return;  // Already turning
    }

    const bool home = digitalRead(PIN_LIMIT_HOME) == LOW;
    const bool end = digitalRead(PIN_LIMIT_END) == LOW;
    if (home && end)
    {
        controlEnabled = false;
        SystemState::triggerAlarm(SystemState::ALARM_MOTOR_ERROR, "Both limit switches active");
        return;
    }

    // GPIO33 HIGH moves toward HOME. If between switches, home first.
    TurnDirection nextDirection = home ? LEFT : RIGHT;
    if (totalTurns > 0 && !home && !end)
        nextDirection = getOppositeTurnDirection(lastTurnDirection);

    setMotorDirection(nextDirection);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(50);
    portENTER_CRITICAL(&motorMux);
    targetLimit = nextDirection == RIGHT ? PIN_LIMIT_HOME : PIN_LIMIT_END;
    motorTicks = 0;
    pulseHigh = false;
    motorResult = MotorResult::Running;
    digitalWrite(enablePin, DM542_ENABLE_ACTIVE);
    portEXIT_CRITICAL(&motorMux);
    turnerRunning = true;
    turnStartTime = millis();

    lastTurnDirection = nextDirection;

    Logger::info(
        ("Turn #" + String(totalTurns + 1) +
         " started (" + String(nextDirection == LEFT ? "LEFT" : "RIGHT") + ")").c_str());
}

/******************************************************
 * Stop Turner Motor
 ******************************************************/

void EggTurnerManager::stopTurner()
{
    portENTER_CRITICAL(&motorMux);
    motorResult = MotorResult::Idle;
    digitalWrite(stepPin, LOW);
    digitalWrite(enablePin, DM542_ENABLE_INACTIVE);
    pulseHigh = false;
    portEXIT_CRITICAL(&motorMux);

    lastTurnTime = millis();

    Logger::debug(
        ("Turn completed. Next turn in " +
         String(turningIntervalMs / 60000.0f, 1) + " minutes").c_str());
}

/******************************************************
 * Set Motor Direction
 ******************************************************/

void EggTurnerManager::setMotorDirection(TurnDirection direction)
{
    digitalWrite(directionPin, direction == RIGHT ? HIGH : LOW);
}

/******************************************************
 * Get Opposite Turn Direction
 ******************************************************/

EggTurnerManager::TurnDirection EggTurnerManager::getOppositeTurnDirection(
    TurnDirection direction)
{
    return (direction == LEFT) ? RIGHT : LEFT;
}