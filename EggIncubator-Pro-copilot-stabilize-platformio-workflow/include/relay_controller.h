#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

class RelayController
{
public:

    static void begin();

    // Output Control
    static void heater(bool state);
    static void humidifier(bool state);
    static void fan(bool state);
    static void ventilation(bool state);
    static void buzzer(bool state);
    static void motor(bool enable, bool left, bool right);

    // Output Toggle
    static void toggleHeater();
    static void toggleHumidifier();
    static void toggleFan();
    static void toggleVentilation();

    // Output OFF
    static void allOff();
    static bool heaterState();
    static bool humidifierState();
    static bool fanState();
    static bool ventilationState();

private:

    static void writeRelay(
        int pin,
        bool state);
};

#endif