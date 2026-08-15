#include "sensor_manager.h"
#include "pins.h"
#include "logger.h"
#include "system_state.h"

/******************************************************
 * Static Variable Initialization
 ******************************************************/

Adafruit_SHT31 SensorManager::sht31;

OneWire SensorManager::oneWire(PIN_I2C_SDA);  // OneWire on same pin for now

DallasTemperature SensorManager::ds18b20(&SensorManager::oneWire);

float SensorManager::airTemp = 0.0f;
float SensorManager::airHumidity = 0.0f;
float SensorManager::eggTemp = 0.0f;

bool SensorManager::sht31Ready = false;
bool SensorManager::ds18b20Ready = false;

unsigned long SensorManager::lastSHT31Update = 0;
unsigned long SensorManager::lastDS18B20Update = 0;

const uint32_t SensorManager::SHT31_UPDATE_INTERVAL = 2000;    // 2 seconds
const uint32_t SensorManager::DS18B20_UPDATE_INTERVAL = 3000;  // 3 seconds

/******************************************************
 * Begin - Initialize both sensors
 ******************************************************/

void SensorManager::begin()
{
    Logger::info("--------------------------------");
    Logger::info("Initializing Sensor Manager");
    Logger::info("--------------------------------");

    // Initialize SHT31 (Temperature & Humidity)
    if (sht31.begin(0x44))  // 0x44 is default I2C address for SHT31
    {
        sht31Ready = true;
        Logger::info("✓ SHT31 Initialized");
    }
    else
    {
        sht31Ready = false;
        Logger::error("✗ SHT31 NOT FOUND");
    }

    // Initialize DS18B20 (Egg Temperature)
    ds18b20.begin();
    
    if (ds18b20.getDS18Count() > 0)
    {
        ds18b20Ready = true;
        ds18b20.setResolution(12);  // 12-bit resolution
        Logger::info("✓ DS18B20 Initialized");
    }
    else
    {
        ds18b20Ready = false;
        Logger::error("✗ DS18B20 NOT FOUND");
    }

    Logger::info("--------------------------------");
}

/******************************************************
 * Update - Read both sensors with timing
 ******************************************************/

void SensorManager::update()
{
    uint32_t now = millis();

    // Update SHT31
    if (sht31Ready && (now - lastSHT31Update >= SHT31_UPDATE_INTERVAL))
    {
        lastSHT31Update = now;
        updateSHT31();
    }

    // Update DS18B20
    if (ds18b20Ready && (now - lastDS18B20Update >= DS18B20_UPDATE_INTERVAL))
    {
        lastDS18B20Update = now;
        updateDS18B20();
    }

    // Update system state
    systemState.sensor.airTemperature = airTemp;
    systemState.sensor.airHumidity = airHumidity;
    systemState.sensor.eggTemperature = eggTemp;
}

/******************************************************
 * Update SHT31 - Read temperature and humidity
 ******************************************************/

void SensorManager::updateSHT31()
{
    float temp = sht31.readTemperature();
    float humidity = sht31.readHumidity();

    // Check for valid readings
    if (!isnan(temp) && !isnan(humidity))
    {
        airTemp = temp;
        airHumidity = humidity;

        Logger::debug(
            ("SHT31: T=" + String(temp, 1) + "°C, H=" + String(humidity, 1) + "%").c_str());
    }
    else
    {
        Logger::warning("SHT31: Invalid reading");
    }
}

/******************************************************
 * Update DS18B20 - Read egg temperature
 ******************************************************/

void SensorManager::updateDS18B20()
{
    ds18b20.requestTemperatures();

    // Get temperature from first sensor
    float temp = ds18b20.getTempCByIndex(0);

    // DS18B20 returns -127 on error
    if (temp > -127.0f && temp < 85.0f)
    {
        eggTemp = temp;

        Logger::debug(
            ("DS18B20: T=" + String(temp, 1) + "°C").c_str());
    }
    else
    {
        Logger::warning("DS18B20: Invalid reading");
    }
}

/******************************************************
 * Get Air Temperature
 ******************************************************/

float SensorManager::getAirTemperature()
{
    return airTemp;
}

/******************************************************
 * Get Air Humidity
 ******************************************************/

float SensorManager::getAirHumidity()
{
    return airHumidity;
}

/******************************************************
 * Get Egg Temperature
 ******************************************************/

float SensorManager::getEggTemperature()
{
    return eggTemp;
}

/******************************************************
 * Check SHT31 Ready
 ******************************************************/

bool SensorManager::isSHT31Ready()
{
    return sht31Ready;
}

/******************************************************
 * Check DS18B20 Ready
 ******************************************************/

bool SensorManager::isDS18B20Ready()
{
    return ds18b20Ready;
}

/******************************************************
 * Check All Sensors Ready
 ******************************************************/

bool SensorManager::allReady()
{
    return sht31Ready && ds18b20Ready;
}
