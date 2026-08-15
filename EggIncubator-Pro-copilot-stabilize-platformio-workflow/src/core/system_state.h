#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>
#include <map>

/******************************************************
 * System State - Manages all system status and states
 * Tracks temperature, humidity, egg turner, and alarms
 ******************************************************/

class SystemState
{
public:

    /******************************************************
     * Incubation Stage Enumeration
     ******************************************************/
    enum IncubationStage
    {
        IDLE = 0,           // System not running
        SETUP = 1,          // Calibration phase
        INCUBATION = 2,     // Active incubation (0-18 days)
        LOCKDOWN = 3,       // No turning (18-21 days)
        HATCHING = 4,       // Final stage (21+ days)
        ERROR = 5           // Error state
    };

    /******************************************************
     * Alarm Type Enumeration
     ******************************************************/
    enum AlarmType
    {
        ALARM_NONE = 0,
        ALARM_TEMP_HIGH = 1,
        ALARM_TEMP_LOW = 2,
        ALARM_HUMIDITY_HIGH = 3,
        ALARM_HUMIDITY_LOW = 4,
        ALARM_SENSOR_ERROR = 5,
        ALARM_MOTOR_ERROR = 6,
        ALARM_POWER_LOSS = 7
    };

    /******************************************************
     * Temperature Status Structure
     ******************************************************/
    struct TemperatureStatus
    {
        float current = 0.0f;           // Current temperature (°C)
        float target = 37.5f;           // Target temperature (°C)
        float min = 0.0f;               // Minimum reading
        float max = 0.0f;               // Maximum reading
        bool isStable = false;          // Is temperature stable?
        bool heaterActive = false;      // Is heater on?
        uint32_t lastUpdateTime = 0;    // Last update timestamp
    };

    /******************************************************
     * Humidity Status Structure
     ******************************************************/
    struct HumidityStatus
    {
        float current = 0.0f;           // Current humidity (%)
        float target = 60.0f;           // Target humidity (%)
        float min = 0.0f;               // Minimum reading
        float max = 0.0f;               // Maximum reading
        bool isStable = false;          // Is humidity stable?
        bool humidifierActive = false;  // Is humidifier on?
        uint32_t lastUpdateTime = 0;    // Last update timestamp
    };

    /******************************************************
     * Egg Turner Status Structure
     ******************************************************/
    struct EggTurnerStatus
    {
        bool isEnabled = false;         // Is turner enabled?
        bool isRunning = false;         // Is turner currently running?
        uint16_t turnInterval = 120;    // Interval between turns (seconds)
        uint16_t turnDuration = 5;      // Duration of each turn (seconds)
        uint32_t totalTurns = 0;        // Total number of turns performed
        uint32_t lastTurnTime = 0;      // Last turn timestamp
        uint32_t timeUntilNextTurn = 0; // Time until next turn (ms)
        String lastTurnDirection = "";  // Direction of last turn
    };

    /******************************************************
     * Sensor Status Structure
     ******************************************************/
    struct SensorStatus
    {
        bool temperatureSensorOk = false;   // Temperature sensor working?
        bool humiditySensorOk = false;      // Humidity sensor working?
        bool motorControlOk = false;        // Motor control working?
        bool heaterOk = false;              // Heater working?
        bool humidifierOk = false;          // Humidifier working?
        uint32_t lastDiagnosticTime = 0;    // Last diagnostic check
    };

    /******************************************************
     * System Metrics Structure
     ******************************************************/
    struct SystemMetrics
    {
        uint32_t uptimeSeconds = 0;         // System uptime in seconds
        uint32_t incubationDayNumber = 0;   // Current day of incubation
        uint32_t totalRunTime = 0;          // Total cumulative run time
        float averageTemperature = 0.0f;    // Average temperature
        float averageHumidity = 0.0f;       // Average humidity
        uint32_t heaterOnTime = 0;          // Total heater on time (seconds)
        uint32_t humidifierOnTime = 0;      // Total humidifier on time (seconds)
    };

    /******************************************************
     * Alarm Status Structure
     ******************************************************/
    struct AlarmStatus
    {
        bool alarmActive = false;           // Is any alarm active?
        AlarmType currentAlarmType = ALARM_NONE;
        String alarmMessage = "";
        uint32_t alarmStartTime = 0;        // When alarm started
        uint32_t alarmCount = 0;            // Number of alarms triggered
        bool buzzerActive = false;          // Is buzzer sounding?
    };

    /******************************************************
     * Initialize System State
     ******************************************************/
    static void begin();

    /******************************************************
     * Temperature Status Access
     ******************************************************/
    static void setTemperatureStatus(const TemperatureStatus& status);
    static TemperatureStatus getTemperatureStatus();
    static void updateCurrentTemperature(float temp);
    static void updateTargetTemperature(float temp);
    static void setTemperatureStability(bool stable);
    static void setHeaterActive(bool active);

    /******************************************************
     * Humidity Status Access
     ******************************************************/
    static void setHumidityStatus(const HumidityStatus& status);
    static HumidityStatus getHumidityStatus();
    static void updateCurrentHumidity(float humidity);
    static void updateTargetHumidity(float humidity);
    static void setHumidityStability(bool stable);
    static void setHumidifierActive(bool active);

    /******************************************************
     * Egg Turner Status Access
     ******************************************************/
    static void setEggTurnerStatus(const EggTurnerStatus& status);
    static EggTurnerStatus getEggTurnerStatus();
    static void updateTurnerCount(uint32_t count);
    static void updateTimeUntilNextTurn(uint32_t timeMs);
    static void setTurnerEnabled(bool enabled);
    static void setTurnerRunning(bool running);

    /******************************************************
     * Sensor Status Access
     ******************************************************/
    static void setSensorStatus(const SensorStatus& status);
    static SensorStatus getSensorStatus();
    static void setSensorHealthy(bool tempOk, bool humOk, bool motorOk);

    /******************************************************
     * System Metrics Access
     ******************************************************/
    static void setSystemMetrics(const SystemMetrics& metrics);
    static SystemMetrics getSystemMetrics();
    static void updateMetrics(uint32_t uptimeSeconds, float avgTemp, float avgHumidity);
    static void updateIncubationDay(uint32_t day);

    /******************************************************
     * Alarm Status Access
     ******************************************************/
    static void setAlarmStatus(const AlarmStatus& status);
    static AlarmStatus getAlarmStatus();
    static void triggerAlarm(AlarmType type, const String& message);
    static void clearAlarm();
    static void setBuzzerActive(bool active);

    /******************************************************
     * Incubation Stage Access
     ******************************************************/
    static void setIncubationStage(IncubationStage stage);
    static IncubationStage getIncubationStage();
    static String getIncubationStageName();
    static bool isIncubating();

    /******************************************************
     * Get Stage Name String
     ******************************************************/
    static String getStageName(IncubationStage stage);

    /******************************************************
     * Get Alarm Type Name String
     ******************************************************/
    static String getAlarmTypeName(AlarmType type);

    /******************************************************
     * Print Full System Status
     ******************************************************/
    static void printFullStatus();

    /******************************************************
     * Reset System State
     ******************************************************/
    static void reset();

private:

    /******************************************************
     * Static member variables
     ******************************************************/
    static TemperatureStatus temperatureStatus;
    static HumidityStatus humidityStatus;
    static EggTurnerStatus eggTurnerStatus;
    static SensorStatus sensorStatus;
    static SystemMetrics systemMetrics;
    static AlarmStatus alarmStatus;
    static IncubationStage currentStage;
    static uint32_t stateLastUpdated;
};

#endif
