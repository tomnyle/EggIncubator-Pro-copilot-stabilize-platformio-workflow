#ifndef INCUBATOR_CONTROLLER_H
#define INCUBATOR_CONTROLLER_H

class IncubatorController
{
public:

    void begin();

    void loop();

    void requestHeater(bool state);

    void requestHumidifier(bool state);

    void requestCirculationFan(bool state);

    void requestVentilationFan(bool state);

    bool heaterState() const;

    bool humidifierState() const;

    bool circulationFanState() const;

    bool ventilationFanState() const;

private:

    void applyOutputs();
};

extern IncubatorController controller;

#endif