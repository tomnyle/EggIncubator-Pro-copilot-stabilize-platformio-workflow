#pragma once

#include <math.h>

constexpr float MAX_AIR_TEMPERATURE_C = 40.0f;

enum class SensorFault { None, Invalid, OverTemperature };

inline SensorFault checkSensorSafety(bool ready, float temperature, float humidity)
{
    if (!ready || !isfinite(temperature) || !isfinite(humidity) ||
        temperature < -40.0f || temperature > 125.0f ||
        humidity < 0.0f || humidity > 100.0f)
        return SensorFault::Invalid;
    if (temperature >= MAX_AIR_TEMPERATURE_C)
        return SensorFault::OverTemperature;
    return SensorFault::None;
}
