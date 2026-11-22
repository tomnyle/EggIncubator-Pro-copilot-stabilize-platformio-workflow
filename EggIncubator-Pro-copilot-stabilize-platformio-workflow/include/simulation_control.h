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
bool setSimulationModeCommand(const char *mode);
bool setDryRunTemperature(float temperature);
bool setDryRunHumidity(float humidity);
bool isManualSimulationMode();
bool simulatedTurnerState();
const char *simulationModeName();
float simulatedInputTemperature();
float simulatedInputHumidity();
