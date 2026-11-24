#pragma once
#include <cstddef>
extern bool testStorageAvailable;
extern bool testStoredAlarm;
class Preferences
{
public:
    bool begin(const char *, bool) { return testStorageAvailable; }
    bool getBool(const char *, bool) { return testStoredAlarm; }
    size_t putBool(const char *, bool value)
    {
        if (!testStorageAvailable) return 0;
        testStoredAlarm = value;
        return 1;
    }
};
