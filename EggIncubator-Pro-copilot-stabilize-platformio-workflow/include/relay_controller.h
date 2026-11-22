#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

class RelayController
{
public:

    static void begin();
    static bool simulatedHeater();
    static bool simulatedHumidifier();
    static bool simulatedFan();
    static bool simulatedVentilation();

    // Output Control
    static void heater(bool state);
    static void humidifier(bool state);
    static void fan(bool state);
    static void ventilation(bool state);

    // Output Toggle
    static void toggleHeater();
    static void toggleHumidifier();
    static void toggleFan();
    static void toggleVentilation();

    // Output OFF
    static void allOff();

private:

    static void writeRelay(
        int pin,
        bool state);
};

#endif