#include "safety_limits.h"
#include <cassert>

int main()
{
    assert(checkSensorSafety(false, 37.5f, 55.0f) == SensorFault::Invalid);
    assert(checkSensorSafety(true, NAN, 55.0f) == SensorFault::Invalid);
    assert(checkSensorSafety(true, 37.5f, NAN) == SensorFault::Invalid);
    assert(checkSensorSafety(true, 37.5f, 101.0f) == SensorFault::Invalid);
    assert(checkSensorSafety(true, 39.9f, 55.0f) == SensorFault::None);
    assert(checkSensorSafety(true, 40.0f, 55.0f) == SensorFault::OverTemperature);
    assert(checkSensorSafety(true, 42.0f, 55.0f) == SensorFault::OverTemperature);
}
