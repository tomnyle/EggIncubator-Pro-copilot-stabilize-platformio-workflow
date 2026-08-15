#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include <Arduino.h>

class OutputManager
{
public:

    static void begin();

    static void update();

    static void stopAll();

    static void setHeater(bool state);
    static void setHumidifier(bool state);
    static void setFan(bool state);
    static void setVent(bool state);

    static bool heater();
    static bool humidifier();
    static bool fan();
    static bool vent();

private:

    static bool heaterState;
    static bool humidifierState;
    static bool fanState;
    static bool ventState;
};

#endif