#ifndef EGG_TURNER_MANAGER_H
#define EGG_TURNER_MANAGER_H

#include <Arduino.h>

/******************************************************
 * Egg Turner Manager - Controls automatic egg turning
 * for optimal embryo development
 ******************************************************/

class EggTurnerManager
{
public:

    /******************************************************
     * Initialize Egg Turner Manager
     ******************************************************/
    static void begin();

    /******************************************************
     * Update - Check if turning is needed and control motor
     ******************************************************/
    static void update();

    /******************************************************
     * Set Turning Interval (minutes between turns)
     ******************************************************/
    static void setTurningInterval(uint16_t intervalMinutes);

    /******************************************************
     * Get Turning Interval
     ******************************************************/
    static uint16_t getTurningInterval();

    /******************************************************
     * Set Turn Duration (seconds for each turn)
     ******************************************************/
    static void setTurnDuration(uint16_t durationSeconds);

    /******************************************************
     * Get Turn Duration
     ******************************************************/
    static uint16_t getTurnDuration();

    /******************************************************
     * Manual Turn - Perform a turn immediately
     ******************************************************/
    static void manualTurn();

    /******************************************************
     * Enable/Disable Egg Turning
     ******************************************************/
    static void enable();
    static void disable();
    static bool isEnabled();

    /******************************************************
     * Check if turner is currently running
     ******************************************************/
    static bool isTurnerRunning();

    /******************************************************
     * Get last turn time (ms since start)
     ******************************************************/
    static uint32_t getLastTurnTime();

    /******************************************************
     * Get time until next turn (ms)
     ******************************************************/
    static uint32_t getTimeUntilNextTurn();

    /******************************************************
     * Get total number of turns performed
     ******************************************************/
    static uint32_t getTurnCount();

private:

    // Control parameters
    static uint32_t turningIntervalMs;  // Interval between turns (in milliseconds)
    static uint32_t turnDurationMs;     // Duration of each turn (in milliseconds)

    // State tracking
    static bool controlEnabled;
    static bool turnerRunning;

    // Timing
    static uint32_t lastTurnTime;       // Timestamp of last turn (ms)
    static uint32_t turnStartTime;      // When current turn started
    static uint32_t totalTurns;         // Total number of turns performed

    // Motor control pins
    static uint8_t motorLeftPin;
    static uint8_t motorRightPin;

    // Turn direction tracking
    enum TurnDirection
    {
        LEFT = 0,
        RIGHT = 1
    };
    static TurnDirection lastTurnDirection;

    /******************************************************
     * Perform a single turn
     ******************************************************/
    static void performTurn();

    /******************************************************
     * Stop the turner motor
     ******************************************************/
    static void stopTurner();

    /******************************************************
     * Control motor direction
     ******************************************************/
    static void setMotorDirection(TurnDirection direction);

    /******************************************************
     * Get opposite turn direction
     ******************************************************/
    static TurnDirection getOppositeTurnDirection(TurnDirection direction);
};

#endif