#pragma once

enum class OperatingMode { Off, Manual, Auto };

class OperatingModeController
{
public:
    static OperatingMode current();
    static const char *name();
    static bool set(OperatingMode mode);
    static void update();
};
