#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class SensorManager
{
public:
    static void begin();
    static void update();

    static float getAirTemperature();
    static float getAirHumidity();
    static float getEggTemperature();

    static bool isSHT31Ready();
    static bool isDS18B20Ready();

    static bool isAirTemperatureValid();
    static bool isHumidityValid();
    static bool isEggTemperatureValid();

    static bool isDoorOpen();
    static bool isWaterLow();

private:
    static Adafruit_SHT31 sht31;
    static OneWire oneWire;
    static DallasTemperature ds18b20;

    static float airTemp;
    static float airHumidity;
    static float eggTemp;

    static bool sht31Ready;
    static bool ds18b20Ready;

    static unsigned long lastSHT31Update;
    static unsigned long lastDS18B20Update;

    static const uint32_t SHT31_UPDATE_INTERVAL;
    static const uint32_t DS18B20_UPDATE_INTERVAL;
    static const uint32_t SENSOR_STALE_MS;

    static void updateSHT31();
    static void updateDS18B20();
};

#endif
