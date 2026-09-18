#include "incubation_profile.h"

IncubationProfileType IncubationProfile::activeProfile = IncubationProfileType::CHICKEN;
uint32_t IncubationProfile::batchStartEpoch = 0;

const IncubationProfileData IncubationProfile::profiles[4] = {
    {"Chicken", 21, 18, 37.5f, 55.0f, 68.0f, 120, 5},
    {"Quail", 17, 14, 37.5f, 52.0f, 68.0f, 120, 5},
    {"Duck", 28, 25, 37.4f, 58.0f, 72.0f, 120, 5},
    {"Goose", 31, 28, 37.3f, 58.0f, 72.0f, 120, 5}
};

void IncubationProfile::begin(IncubationProfileType profile, uint32_t startEpoch)
{
    activeProfile = profile;
    batchStartEpoch = startEpoch;
}

void IncubationProfile::setProfile(IncubationProfileType profile, uint32_t nowEpoch)
{
    activeProfile = profile;
    batchStartEpoch = nowEpoch;
}

void IncubationProfile::setStartEpoch(uint32_t startEpoch)
{
    batchStartEpoch = startEpoch;
}

uint32_t IncubationProfile::getStartEpoch()
{
    return batchStartEpoch;
}

const IncubationProfileData& IncubationProfile::current()
{
    return profiles[static_cast<uint8_t>(activeProfile)];
}

uint16_t IncubationProfile::currentDay(uint32_t nowEpoch)
{
    if (batchStartEpoch == 0 || nowEpoch < batchStartEpoch)
    {
        return 1;
    }

    const uint32_t elapsedSec = nowEpoch - batchStartEpoch;
    const uint32_t day = elapsedSec / 86400UL;
    return static_cast<uint16_t>(day + 1);
}

bool IncubationProfile::isLockdown(uint32_t nowEpoch)
{
    return currentDay(nowEpoch) >= current().lockdownDay;
}

bool IncubationProfile::isHatchFinished(uint32_t nowEpoch)
{
    return currentDay(nowEpoch) > current().totalDays;
}

const char* IncubationProfile::currentCode()
{
    switch (activeProfile)
    {
        case IncubationProfileType::CHICKEN: return "CHICKEN";
        case IncubationProfileType::QUAIL: return "QUAIL";
        case IncubationProfileType::DUCK: return "DUCK";
        case IncubationProfileType::GOOSE: return "GOOSE";
        default: return "CHICKEN";
    }
}

bool IncubationProfile::setProfileByCode(const String& code, uint32_t nowEpoch)
{
    const IncubationProfileType profile = profileFromCode(code);
    const bool valid = (code.equalsIgnoreCase("CHICKEN") ||
                        code.equalsIgnoreCase("QUAIL") ||
                        code.equalsIgnoreCase("DUCK") ||
                        code.equalsIgnoreCase("GOOSE"));
    if (!valid) return false;

    setProfile(profile, nowEpoch);
    return true;
}

IncubationProfileType IncubationProfile::profileFromCode(const String& code)
{
    String c = code;
    c.trim();
    c.toUpperCase();

    if (c == "QUAIL") return IncubationProfileType::QUAIL;
    if (c == "DUCK") return IncubationProfileType::DUCK;
    if (c == "GOOSE") return IncubationProfileType::GOOSE;
    return IncubationProfileType::CHICKEN;
}
