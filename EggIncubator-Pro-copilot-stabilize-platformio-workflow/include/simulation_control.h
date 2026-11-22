#pragma once

enum class SimulatedOutput
{
    Heater,
    Humidifier,
    Fan,
    Ventilation,
    Turner
};

bool setManualSimulatedOutput(SimulatedOutput output, bool state);
bool isManualSimulationMode();
bool simulatedTurnerState();
const char *simulationModeName();
