#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

/******************************************************
 * Connection Status
 ******************************************************/
struct ConnectionState
{
    bool wifi = false;
    bool mqtt = false;
    bool discovery = false;
};

/******************************************************
 * Relay State
 ******************************************************/
struct RelayState
{
    bool heater = false;
    bool humidifier = false;
    bool circulationFan = false;
    bool ventilationFan = false;
};

/******************************************************
 * Motor State
 ******************************************************/
struct MotorState
{
    bool enabled = false;
    bool moving = false;
    bool homed = false;

    int position = 0;
};

/******************************************************
 * Sensor State
 ******************************************************/
struct SensorState
{
    float airTemperature = 0.0f;
    float airHumidity = 0.0f;

    float eggTemperature = 0.0f;

    float light = 0.0f;

    bool doorOpen = false;

    bool waterLow = false;
};

/******************************************************
 * Environment Target
 ******************************************************/
struct TargetState
{
    float temperature = 37.5f;
    float humidity = 60.0f;
};

/******************************************************
 * Alarm
 ******************************************************/
struct AlarmState
{
    bool active = false;

    String message = "";
};

/******************************************************
 * Device Information
 ******************************************************/
struct DeviceState
{
    String ip;

    String mac;

    String firmware;

    int rssi = 0;

    uint32_t uptime = 0;
};

/******************************************************
 * System State
 ******************************************************/
struct SystemState
{
    ConnectionState connection;

    RelayState relay;

    MotorState motor;

    SensorState sensor;

    TargetState target;

    AlarmState alarm;

    DeviceState device;
};

extern SystemState systemState;

#endif