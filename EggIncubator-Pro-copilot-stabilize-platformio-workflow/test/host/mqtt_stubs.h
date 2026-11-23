#pragma once
#include "Arduino.h"

#define SENSOR_MANAGER_H
#define NETWORK_SERVICE_H
#define EGG_TURNER_MANAGER_H
#include "incubation_profile.h"

class SensorManager
{
public:
    static bool isSHT31Ready() { return true; }
    static float getAirTemperature() { return 35.0f; }
    static float getAirHumidity() { return 40.0f; }
    static bool isDS18B20Ready() { return false; }
    static float getEggTemperature() { return 0.0f; }
};
class NetworkService { public: bool connected() { return true; } };
class EggTurnerManager
{
public:
    static bool isEnabled() { return false; }
    static unsigned long getTurnCount() { return 0; }
};
