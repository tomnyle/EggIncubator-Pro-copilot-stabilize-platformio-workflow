#include "incubation_profile.h"

IncubationProfileType IncubationProfile::activeProfile = IncubationProfileType::CHICKEN;
uint32_t IncubationProfile::batchStartMillis = 0;

const IncubationProfileData IncubationProfile::profiles[4] = {
    // CHICKEN
    {"Chicken", 21, 18, 37.5f, 55.0f, 68.0f, 120, 5},
    // QUAIL
    {"Quail",   17, 14, 37.5f, 52.0f, 68.0f, 120, 5},
    // DUCK
    {"Duck",    28, 25, 37.4f, 58.0f, 72.0f, 120, 5},
    // GOOSE (ngan/ngỗng)
    {"Goose",   31, 28, 37.3f, 58.0f, 72.0f, 120, 5}
};

void IncubationProfile::begin(IncubationProfileType profile, uint32_t startMillis)
{
    activeProfile = profile;
    batchStartMillis = startMillis;
}

void IncubationProfile::setProfile(IncubationProfileType profile, uint32_t nowMillis)
{
    activeProfile = profile;
    batchStartMillis = nowMillis; // reset batch day count
}

const IncubationProfileData& IncubationProfile::current()
{
    return profiles[static_cast<uint8_t>(activeProfile)];
}

uint16_t IncubationProfile::currentDay(uint32_t nowMillis)
{
    uint32_t elapsedMs = nowMillis - batchStartMillis;
    uint32_t day = elapsedMs / 86400000UL; // 24h
    return static_cast<uint16_t>(day + 1);
}

bool IncubationProfile::isLockdown(uint32_t nowMillis)
{
    return currentDay(nowMillis) >= current().lockdownDay;
}

bool IncubationProfile::isHatchFinished(uint32_t nowMillis)
{
    return currentDay(nowMillis) > current().totalDays;
}

// -------- NEW --------

const char* IncubationProfile::currentCode()
{
    switch (activeProfile)
    {
        case IncubationProfileType::CHICKEN: return "CHICKEN";
        case IncubationProfileType::QUAIL:   return "QUAIL";
        case IncubationProfileType::DUCK:    return "DUCK";
        case IncubationProfileType::GOOSE:   return "GOOSE";
        default: return "CHICKEN";
    }
}

bool IncubationProfile::setProfileByCode(const String& code, uint32_t nowMillis)
{
    String c = code;
    c.trim();
    c.toUpperCase();

    if (c == "CHICKEN") { setProfile(IncubationProfileType::CHICKEN, nowMillis); return true; }
    if (c == "QUAIL")   { setProfile(IncubationProfileType::QUAIL,   nowMillis); return true; }
    if (c == "DUCK")    { setProfile(IncubationProfileType::DUCK,    nowMillis); return true; }
    if (c == "GOOSE")   { setProfile(IncubationProfileType::GOOSE,   nowMillis); return true; }

    return false;
}