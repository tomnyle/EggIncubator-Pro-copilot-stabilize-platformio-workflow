#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SHT31.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/******************************************************
 * Sensor Manager - Handles SHT31 and DS18B20
 ******************************************************/

class SensorManager
{
public:

    static void begin();

    static void update();

    // SHT31 (Air Temperature & Humidity)
    static float getAirTemperature();

    static float getAirHumidity();

    static bool isSHT31Ready();

    // DS18B20 (Egg Temperature)
    static float getEggTemperature();

    static bool isDS18B20Ready();

    // Both sensors
    static bool allReady();

private:

    static Adafruit_SHT31 sht31;

    static OneWire oneWire;

    static DallasTemperature ds18b20;

    // Sensor state
    static float airTemp;
    static float airHumidity;
    static float eggTemp;

    static bool sht31Ready;
    static bool ds18b20Ready;

    static unsigned long lastSHT31Update;
    static unsigned long lastDS18B20Update;

    static const uint32_t SHT31_UPDATE_INTERVAL;
    static const uint32_t DS18B20_UPDATE_INTERVAL;

    // Private methods
    static void updateSHT31();

    static void updateDS18B20();

};

#endif
