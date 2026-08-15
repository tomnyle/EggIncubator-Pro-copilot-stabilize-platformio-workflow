#ifndef INCUBATION_PROFILE_H
#define INCUBATION_PROFILE_H

#include <Arduino.h>

enum class IncubationProfileType : uint8_t
{
    CHICKEN = 0,
    QUAIL   = 1,
    DUCK    = 2,
    GOOSE   = 3
};

struct IncubationProfileData
{
    const char* name;
    uint16_t totalDays;
    uint16_t lockdownDay;     // first day of lockdown (stop turning)
    float tempSetpointC;
    float humidityIncubation; // before lockdown
    float humidityLockdown;   // during lockdown
    uint16_t turnIntervalMin;
    uint16_t turnDurationSec;
};

class IncubationProfile
{
public:
    static void begin(IncubationProfileType profile, uint32_t startMillis);
    static void setProfile(IncubationProfileType profile, uint32_t nowMillis);

    static const IncubationProfileData& current();
    static uint16_t currentDay(uint32_t nowMillis); // day starts from 1
    static bool isLockdown(uint32_t nowMillis);
    static bool isHatchFinished(uint32_t nowMillis);

    // NEW: MQTT/HA helpers
    static const char* currentCode(); // "CHICKEN", "QUAIL", "DUCK", "GOOSE"
    static bool setProfileByCode(const String& code, uint32_t nowMillis);

private:
    static IncubationProfileType activeProfile;
    static uint32_t batchStartMillis;
    static const IncubationProfileData profiles[4];
};

#endif