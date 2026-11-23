#include "incubation_profile.h"
#include <cassert>
#include <cstring>

int main()
{
    constexpr uint32_t day = 86400000UL;
    IncubationProfile::begin(IncubationProfileType::CHICKEN, 0);
    assert(IncubationProfile::setProfileByCode(" chicken ", 2 * day));
    assert(IncubationProfile::currentDay(2 * day) == 3);

    for (const char* code : {"QUAIL", "DUCK", "GOOSE", "CHICKEN"})
    {
        assert(IncubationProfile::setProfileByCode(code, 2 * day));
        assert(std::strcmp(IncubationProfile::currentCode(), code) == 0);
        assert(IncubationProfile::currentDay(2 * day) == 1);
    }

    assert(!IncubationProfile::setProfileByCode("INVALID", 3 * day));
    assert(std::strcmp(IncubationProfile::currentCode(), "CHICKEN") == 0);
    assert(IncubationProfile::currentDay(3 * day) == 2);
}
