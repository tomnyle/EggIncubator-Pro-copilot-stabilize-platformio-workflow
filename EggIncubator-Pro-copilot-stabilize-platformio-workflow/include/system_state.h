#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

enum class SystemMode : uint8_t
{
    OFF = 0,
    MANUAL = 1,
    AUTO = 2,
    ALARM = 3
};

enum FaultCode : uint32_t
{
    FAULT_NONE = 0,
    FAULT_SHT31_FAIL = 1u << 0,
    FAULT_DS18B20_FAIL = 1u << 1,
    FAULT_TEMP_DISAGREE = 1u << 2,
    FAULT_OVER_TEMP = 1u << 3,
    FAULT_HUMIDITY_FAIL = 1u << 4,
    FAULT_TURN_TIMEOUT = 1u << 5,
    FAULT_LIMIT_INCONSISTENT = 1u << 6,
    FAULT_DOOR_OPEN = 1u << 7,
    FAULT_SYSTEM_ERROR = 1u << 8,
    FAULT_WATER_LOW = 1u << 9,
    WARN_MQTT_OFFLINE = 1u << 16
};

struct SensorReading
{
    float value = 0.0f;
    bool valid = false;
    bool stale = true;
    uint32_t lastUpdateMs = 0;
};

struct SensorState
{
    SensorReading airTemperature;
    SensorReading airHumidity;
    SensorReading eggTemperature;
    bool doorOpen = false;
    bool waterLow = false;
    bool tempDisagreement = false;
};

struct OutputState
{
    bool heater = false;
    bool humidifier = false;
    bool circulationFan = false;
    bool ventilationFan = false;
    bool motorLeft = false;
    bool motorRight = false;
    bool motorEnable = false;
    bool buzzer = false;
};

struct IncubationState
{
    bool incubationRunning = false;
    bool turningEnabled = true;
    float targetTemperature = 37.5f;
    float targetHumidity = 60.0f;
    float overTemperatureLimit = 38.5f;
    float maxTempDisagreementC = 1.0f;
    uint32_t incubationStartEpoch = 0;
    uint16_t incubationDay = 0;
    String profileCode = "CHICKEN";
};

struct FaultState
{
    uint32_t activeMask = FAULT_NONE;
    uint32_t warningMask = FAULT_NONE;
    String lastMessage;
    uint32_t lastUpdateMs = 0;
};

struct ConnectionState
{
    bool wifi = false;
    bool mqtt = false;
    int32_t rssi = 0;
};

struct SystemState
{
    SystemMode mode = SystemMode::OFF;
    SensorState sensor;
    OutputState output;
    IncubationState incubation;
    FaultState fault;
    ConnectionState connection;
    uint32_t uptimeSeconds = 0;
    bool haDiscoveryPublished = false;
};

extern SystemState systemState;

void setFault(FaultCode code, bool active, const String &message = "");
bool hasFault(FaultCode code);
bool hasCriticalFault();
const char *modeToString(SystemMode mode);
String activeFaultSummary();

#endif
