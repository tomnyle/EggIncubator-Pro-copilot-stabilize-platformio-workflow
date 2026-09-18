#include "sensor_manager.h"

#include <math.h>
#include <Wire.h>
#include "pins.h"
#include "logger.h"
#include "system_state.h"

Adafruit_SHT31 SensorManager::sht31;
OneWire SensorManager::oneWire(PIN_DS18B20);
DallasTemperature SensorManager::ds18b20(&SensorManager::oneWire);

float SensorManager::airTemp = 0.0f;
float SensorManager::airHumidity = 0.0f;
float SensorManager::eggTemp = 0.0f;

bool SensorManager::sht31Ready = false;
bool SensorManager::ds18b20Ready = false;

unsigned long SensorManager::lastSHT31Update = 0;
unsigned long SensorManager::lastDS18B20Update = 0;

const uint32_t SensorManager::SHT31_UPDATE_INTERVAL = 2000;
const uint32_t SensorManager::DS18B20_UPDATE_INTERVAL = 3000;
const uint32_t SensorManager::SENSOR_STALE_MS = 15000;

void SensorManager::begin()
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    pinMode(PIN_DOOR_SWITCH, INPUT_PULLUP);
    pinMode(PIN_WATER_LOW_SWITCH, INPUT_PULLUP);

    sht31Ready = sht31.begin(0x44);
    setFault(FAULT_SHT31_FAIL, !sht31Ready, "SHT31 init failed");

    ds18b20.begin();
    ds18b20Ready = ds18b20.getDS18Count() > 0;
    if (ds18b20Ready)
    {
        ds18b20.setResolution(12);
    }
    setFault(FAULT_DS18B20_FAIL, !ds18b20Ready, "DS18B20 init failed");

    Logger::info(sht31Ready ? "SHT31 ready" : "SHT31 not ready");
    Logger::info(ds18b20Ready ? "DS18B20 ready" : "DS18B20 not ready");
}

void SensorManager::update()
{
    const uint32_t now = millis();

    if (sht31Ready && (now - lastSHT31Update >= SHT31_UPDATE_INTERVAL))
    {
        lastSHT31Update = now;
        updateSHT31();
    }

    if (ds18b20Ready && (now - lastDS18B20Update >= DS18B20_UPDATE_INTERVAL))
    {
        lastDS18B20Update = now;
        updateDS18B20();
    }

    systemState.sensor.airTemperature.stale = (now - systemState.sensor.airTemperature.lastUpdateMs) > SENSOR_STALE_MS;
    systemState.sensor.airHumidity.stale = (now - systemState.sensor.airHumidity.lastUpdateMs) > SENSOR_STALE_MS;
    systemState.sensor.eggTemperature.stale = (now - systemState.sensor.eggTemperature.lastUpdateMs) > SENSOR_STALE_MS;

    systemState.sensor.doorOpen = (digitalRead(PIN_DOOR_SWITCH) == INPUT_ACTIVE);
    systemState.sensor.waterLow = (digitalRead(PIN_WATER_LOW_SWITCH) == INPUT_ACTIVE);

    setFault(FAULT_DOOR_OPEN, systemState.sensor.doorOpen, "Door open");
    setFault(FAULT_WATER_LOW, systemState.sensor.waterLow, "Water low");

    const bool tempCompareValid = isAirTemperatureValid() && isEggTemperatureValid();
    const float delta = tempCompareValid ? fabsf(systemState.sensor.airTemperature.value - systemState.sensor.eggTemperature.value) : 0.0f;

    systemState.sensor.tempDisagreement = tempCompareValid && (delta > systemState.incubation.maxTempDisagreementC);
    setFault(FAULT_TEMP_DISAGREE, systemState.sensor.tempDisagreement, "SHT31/DS18B20 disagreement");

    setFault(FAULT_SHT31_FAIL, !(isAirTemperatureValid() && isHumidityValid()), "SHT31 invalid or stale");
    setFault(FAULT_DS18B20_FAIL, !isEggTemperatureValid(), "DS18B20 invalid or stale");
    setFault(FAULT_HUMIDITY_FAIL, !isHumidityValid(), "Humidity invalid or stale");
}

void SensorManager::updateSHT31()
{
    const float temp = sht31.readTemperature();
    const float humidity = sht31.readHumidity();

    if (!isnan(temp) && !isnan(humidity) && temp > -10.0f && temp < 70.0f && humidity >= 0.0f && humidity <= 100.0f)
    {
        airTemp = temp;
        airHumidity = humidity;

        systemState.sensor.airTemperature.value = temp;
        systemState.sensor.airTemperature.valid = true;
        systemState.sensor.airTemperature.lastUpdateMs = millis();

        systemState.sensor.airHumidity.value = humidity;
        systemState.sensor.airHumidity.valid = true;
        systemState.sensor.airHumidity.lastUpdateMs = millis();
    }
    else
    {
        systemState.sensor.airTemperature.valid = false;
        systemState.sensor.airHumidity.valid = false;
    }
}

void SensorManager::updateDS18B20()
{
    ds18b20.requestTemperatures();
    const float temp = ds18b20.getTempCByIndex(0);

    if (temp > -40.0f && temp < 90.0f && temp != DEVICE_DISCONNECTED_C)
    {
        eggTemp = temp;
        systemState.sensor.eggTemperature.value = temp;
        systemState.sensor.eggTemperature.valid = true;
        systemState.sensor.eggTemperature.lastUpdateMs = millis();
    }
    else
    {
        systemState.sensor.eggTemperature.valid = false;
    }
}

float SensorManager::getAirTemperature() { return airTemp; }
float SensorManager::getAirHumidity() { return airHumidity; }
float SensorManager::getEggTemperature() { return eggTemp; }

bool SensorManager::isSHT31Ready() { return sht31Ready; }
bool SensorManager::isDS18B20Ready() { return ds18b20Ready; }

bool SensorManager::isAirTemperatureValid()
{
    return systemState.sensor.airTemperature.valid && !systemState.sensor.airTemperature.stale;
}

bool SensorManager::isHumidityValid()
{
    return systemState.sensor.airHumidity.valid && !systemState.sensor.airHumidity.stale;
}

bool SensorManager::isEggTemperatureValid()
{
    return systemState.sensor.eggTemperature.valid && !systemState.sensor.eggTemperature.stale;
}

bool SensorManager::isDoorOpen() { return systemState.sensor.doorOpen; }
bool SensorManager::isWaterLow() { return systemState.sensor.waterLow; }
