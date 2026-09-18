#ifndef EGG_TURNER_MANAGER_H
#define EGG_TURNER_MANAGER_H

#include <Arduino.h>

class EggTurnerManager
{
public:
    enum class TurnerState : uint8_t
    {
        IDLE = 0,
        MOVE_TO_HOME = 1,
        MOVE_TO_END = 2,
        WAIT = 3,
        FAULT = 4
    };

    static void begin();
    static void update();

    static void setTurningInterval(uint16_t intervalMinutes);
    static uint16_t getTurningInterval();

    static void setTurnDuration(uint16_t durationSeconds);
    static uint16_t getTurnDuration();

    static void manualTurn();
    static void requestMoveHome();
    static void requestMoveEnd();

    static void enable();
    static void disable();
    static void stopForSafety();
    static bool isEnabled();

    static bool isTurnerRunning();
    static uint32_t getLastTurnTime();
    static uint32_t getTimeUntilNextTurn();
    static uint32_t getTurnCount();
    static const char* getStateName();

private:
    static uint32_t turningIntervalMs;
    static uint32_t turnDurationMs;

    static bool controlEnabled;
    static bool manualRequest;
    static TurnerState state;
    static uint32_t stateStartMs;
    static uint32_t lastTurnTime;
    static uint32_t totalTurns;
    static bool nextMoveToEnd;

    static void stopMotor();
    static void startMoveToHome();
    static void startMoveToEnd();
};

#endif
