#include "egg_turner_manager.h"

#include "relay_controller.h"
#include "logger.h"
#include "pins.h"

/******************************************************
 * Static Member Initialization
 ******************************************************/

uint32_t EggTurnerManager::turningIntervalMs = 120000UL;  // 2 minutes (default)
uint32_t EggTurnerManager::turnDurationMs = 5000UL;       // 5 seconds (default)

bool EggTurnerManager::controlEnabled = false;
bool EggTurnerManager::turnerRunning = false;

uint32_t EggTurnerManager::lastTurnTime = 0;
uint32_t EggTurnerManager::turnStartTime = 0;
uint32_t EggTurnerManager::totalTurns = 0;

uint8_t EggTurnerManager::motorLeftPin = PIN_MOTOR_LEFT;
uint8_t EggTurnerManager::motorRightPin = PIN_MOTOR_RIGHT;

EggTurnerManager::TurnDirection EggTurnerManager::lastTurnDirection = LEFT;

/******************************************************
 * Initialize Egg Turner Manager
 ******************************************************/

void EggTurnerManager::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Egg Turner Manager Started");
    Logger::info("--------------------------------");

    // Setup motor pins
    pinMode(motorLeftPin, OUTPUT);
    pinMode(motorRightPin, OUTPUT);

    // Ensure motor is stopped
    digitalWrite(motorLeftPin, LOW);
    digitalWrite(motorRightPin, LOW);

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
    if (!controlEnabled)
    {
        return;
    }

    uint32_t now = millis();

    // Check if currently turning and need to stop
    if (turnerRunning)
    {
        uint32_t turnElapsed = now - turnStartTime;

        if (turnElapsed >= turnDurationMs)
        {
            stopTurner();
            turnerRunning = false;
        }
    }
    else
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
    if (controlEnabled)
    {
        performTurn();
        Logger::info("Manual turn initiated");
    }
}

/******************************************************
 * Enable Egg Turning
 ******************************************************/

void EggTurnerManager::enable()
{
    if (!controlEnabled)
    {
        controlEnabled = true;
        lastTurnTime = millis();
        totalTurns = 0;
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

    // Determine next turn direction (alternate left/right)
    TurnDirection nextDirection = getOppositeTurnDirection(lastTurnDirection);

    // Start motor
    setMotorDirection(nextDirection);
    turnerRunning = true;
    turnStartTime = millis();

    totalTurns++;
    lastTurnDirection = nextDirection;

    Logger::info(
        ("Turn #" + String(totalTurns) +
         " started (" + String(nextDirection == LEFT ? "LEFT" : "RIGHT") + ")").c_str());
}

/******************************************************
 * Stop Turner Motor
 ******************************************************/

void EggTurnerManager::stopTurner()
{
    digitalWrite(motorLeftPin, LOW);
    digitalWrite(motorRightPin, LOW);

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
    if (direction == LEFT)
    {
        // Turn left: motorLeftPin HIGH, motorRightPin LOW
        digitalWrite(motorLeftPin, HIGH);
        digitalWrite(motorRightPin, LOW);
    }
    else  // RIGHT
    {
        // Turn right: motorLeftPin LOW, motorRightPin HIGH
        digitalWrite(motorLeftPin, LOW);
        digitalWrite(motorRightPin, HIGH);
    }
}

/******************************************************
 * Get Opposite Turn Direction
 ******************************************************/

EggTurnerManager::TurnDirection EggTurnerManager::getOppositeTurnDirection(
    TurnDirection direction)
{
    return (direction == LEFT) ? RIGHT : LEFT;
}