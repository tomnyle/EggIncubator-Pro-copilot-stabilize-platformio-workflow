#include <cassert>
#include <iostream>
#include <string>
#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "mqtt_service.h"
#include "mqtt_stubs.h"
#include "relay_controller.h"
#include "simulation_control.h"
#include "pins.h"

FakeSerial Serial;
FakeESP ESP;
FakeWiFi WiFi;
PubSubClient *activeClient = nullptr;
uint32_t fakeMillis = 6000;
int pinValues[40] = {};
NetworkService network;
MQTTService mqtt;
bool manualMode = true;
bool turnerState = false;

bool isManualSimulationMode() { return manualMode; }
bool simulatedTurnerState() { return turnerState; }
const char *simulationModeName() { return manualMode ? "MANUAL" : "AUTO"; }
bool setManualSimulatedOutput(SimulatedOutput output, bool state)
{
    if (!manualMode || (output == SimulatedOutput::Heater && state && RelayController::heaterSafetyBlocked()))
        return false;
    switch (output)
    {
        case SimulatedOutput::Heater: RelayController::heater(state); break;
        case SimulatedOutput::Humidifier: RelayController::humidifier(state); break;
        case SimulatedOutput::Fan: RelayController::fan(state); break;
        case SimulatedOutput::Ventilation: RelayController::ventilation(state); break;
        case SimulatedOutput::Turner: turnerState = state; break;
    }
    return true;
}

int main(int argc, char **argv)
{
    RelayController::begin();
    RelayController::updateHeaterSafety(35.0f, true);
    mqtt.begin();
    mqtt.loop();
    assert(activeClient->connected());
    assert(activeClient->subscriptions.count("eggincubator/simulation/+/set") == 1);
    const auto &published = activeClient->publications;
    assert(published.at("eggincubator/simulation/mode") == "MANUAL");
    assert(published.at("eggincubator/simulation/availability") == "online");
    assert(published.at("homeassistant/switch/eggincubator01/sim_heater/config").find("Simulated Heater") != std::string::npos);
    assert(published.at("homeassistant/switch/eggincubator01/sim_heater/config").find("availability_mode") != std::string::npos);
    for (const char *name : {"heater", "humidifier", "fan", "vent", "turner"})
    {
        const std::string topic = std::string("homeassistant/switch/eggincubator01/sim_") + name + "/config";
        const std::string &config = published.at(topic);
        assert(config.find(std::string("eggincubator/simulation/") + name + "/set") != std::string::npos);
        assert(config.find(std::string("eggincubator/simulation/") + name + "/state") != std::string::npos);
        assert(config.size() + topic.size() + 7 <= 1024);
        if (argc > 1 && std::string(argv[1]) == "--dump-discovery")
            std::cout << config << '\n';
    }
    assert(published.count("homeassistant/sensor/eggincubator01/simulation_mode/config") == 1);
    assert(published.count("homeassistant/binary_sensor/eggincubator01/simulation_heater_inhibit/config") == 1);

    activeClient->inject("eggincubator/simulation/heater/set", "ON");
    assert(RelayController::simulatedHeater());
    assert(published.at("eggincubator/simulation/heater/state") == "ON");
    assert(published.at("eggincubator/heater/state") == "OFF");
    assert(pinValues[PIN_HEATER_SSR] == RELAY_OFF);
    activeClient->inject("eggincubator/simulation/turner/set", "ON");
    assert(turnerState && published.at("eggincubator/simulation/turner/state") == "ON");
    activeClient->inject("eggincubator/simulation/humidifier/set", "ON");
    activeClient->inject("eggincubator/simulation/fan/set", "ON");
    activeClient->inject("eggincubator/simulation/vent/set", "ON");
    assert(RelayController::simulatedHumidifier());
    assert(RelayController::simulatedFan());
    assert(RelayController::simulatedVentilation());
    assert(pinValues[PIN_HUMIDIFIER] == RELAY_OFF);
    assert(pinValues[PIN_FAN_CIRCULATION] == RELAY_OFF);
    assert(pinValues[PIN_FAN_VENT] == RELAY_OFF);
    activeClient->inject("eggincubator/simulation/fan/set", "OFF");
    assert(!RelayController::simulatedFan());
    activeClient->inject("eggincubator/simulation/fan/set", "bad");
    assert(!RelayController::simulatedFan());
    activeClient->inject("eggincubator/heater/set", "ON");
    assert(pinValues[PIN_HEATER_SSR] == RELAY_OFF);

    RelayController::updateHeaterSafety(39.5f, true);
    activeClient->inject("eggincubator/simulation/heater/set", "ON");
    assert(!RelayController::simulatedHeater());
    assert(published.at("eggincubator/simulation/heater/state") == "OFF");
    assert(published.at("eggincubator/simulation/heater_inhibit") == "ON");

    manualMode = false;
    mqtt.publishSimulationStates();
    assert(published.at("eggincubator/simulation/availability") == "offline");
    activeClient->inject("eggincubator/simulation/fan/set", "ON");
    assert(!RelayController::simulatedFan());
    assert(published.at("eggincubator/simulation/fan/state") == "OFF");
}
