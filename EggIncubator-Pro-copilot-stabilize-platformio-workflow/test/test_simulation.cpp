#include <cassert>
#include <cmath>
#include "Arduino.h"
#include "logger.h"
#include "pins.h"
#include "relay_controller.h"
#include "system_state.h"
#include "managers/temperature_manager.h"
#include "managers/humidity_manager.h"

FakeSerial Serial;
uint32_t fakeMillis = 0;
int pinValues[40] = {};

void Logger::info(const char *) {}
void Logger::warning(const char *) {}
void Logger::error(const char *) {}
void Logger::debug(const char *) {}

static void assertPhysicalOutputsOff()
{
    assert(pinValues[PIN_HEATER_SSR] == RELAY_OFF);
    assert(pinValues[PIN_HUMIDIFIER] == RELAY_OFF);
    assert(pinValues[PIN_FAN_CIRCULATION] == RELAY_OFF);
    assert(pinValues[PIN_FAN_VENT] == RELAY_OFF);
    assert(!systemState.relay.heater && !systemState.relay.humidifier);
    assert(!systemState.relay.circulationFan && !systemState.relay.ventilationFan);
}

int main()
{
    RelayController::begin();
    assert(RelayController::heaterSafetyBlocked());
    RelayController::heater(true);
    assert(!RelayController::simulatedHeater());
    RelayController::updateHeaterSafety(38.5f, true);
    assert(!RelayController::heaterSafetyBlocked());
    RelayController::heater(true);
    RelayController::humidifier(true);
    RelayController::fan(true);
    RelayController::ventilation(true);
    assert(RelayController::simulatedHeater() && RelayController::simulatedHumidifier());
    assert(RelayController::simulatedFan() && RelayController::simulatedVentilation());
    assertPhysicalOutputsOff();

    RelayController::updateHeaterSafety(39.49f, true);
    assert(!RelayController::heaterSafetyBlocked());
    RelayController::updateHeaterSafety(39.5f, true);
    assert(RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    RelayController::heater(true);
    assert(!RelayController::simulatedHeater());
    RelayController::updateHeaterSafety(38.51f, true);
    assert(RelayController::heaterSafetyBlocked());
    RelayController::updateHeaterSafety(38.5f, true);
    assert(!RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    RelayController::heater(true);
    RelayController::updateHeaterSafety(37.0f, false);
    assert(RelayController::heaterSafetyBlocked());
    assert(!RelayController::simulatedHeater());
    RelayController::updateHeaterSafety(NAN, true);
    assert(RelayController::heaterSafetyBlocked());
    RelayController::updateHeaterSafety(39.0f, true);
    assert(RelayController::heaterSafetyBlocked());
    RelayController::updateHeaterSafety(37.0f, true);
    assert(!RelayController::heaterSafetyBlocked());
    RelayController::allOff();
    assert(!RelayController::simulatedHeater() && !RelayController::simulatedHumidifier());
    assert(!RelayController::simulatedFan() && !RelayController::simulatedVentilation());

    TemperatureManager::begin();
    HumidityManager::begin();
    TemperatureManager::enable();
    HumidityManager::enable();
    fakeMillis += 1000;
    TemperatureManager::update(35.0f, true);
    HumidityManager::update(40.0f, true);
    assert(RelayController::simulatedHeater());
    assert(RelayController::simulatedHumidifier());
    assertPhysicalOutputsOff();

    TemperatureManager::update(NAN, false);
    HumidityManager::update(NAN, false);
    assert(!RelayController::simulatedHeater());
    assert(!RelayController::simulatedHumidifier());
    assertPhysicalOutputsOff();

    fakeMillis += 1000;
    TemperatureManager::update(50.0f, true);
    HumidityManager::update(100.0f, true);
    assert(!RelayController::simulatedHeater());
    assert(!RelayController::simulatedHumidifier());
    TemperatureManager::disable();
    HumidityManager::disable();
    assertPhysicalOutputsOff();
}
