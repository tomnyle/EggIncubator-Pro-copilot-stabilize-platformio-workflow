#pragma once
#include "Arduino.h"

#define SENSOR_MANAGER_H
#define NETWORK_SERVICE_H
#define EGG_TURNER_MANAGER_H
#define INCUBATION_PROFILE_H

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
class IncubationProfile
{
public:
    static const char *currentCode() { return "CHICKEN"; }
    static int currentDay(uint32_t) { return 1; }
    static bool isLockdown(uint32_t) { return false; }
    static bool setProfileByCode(const String &, uint32_t) { return true; }
};
