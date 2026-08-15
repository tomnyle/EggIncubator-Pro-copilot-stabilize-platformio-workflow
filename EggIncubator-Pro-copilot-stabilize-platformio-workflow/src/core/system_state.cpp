#include "system_state.h"
#include "logger.h"

/******************************************************
 * Static Member Initialization
 ******************************************************/

SystemState::TemperatureStatus SystemState::temperatureStatus;
SystemState::HumidityStatus SystemState::humidityStatus;
SystemState::EggTurnerStatus SystemState::eggTurnerStatus;
SystemState::SensorStatus SystemState::sensorStatus;
SystemState::SystemMetrics SystemState::systemMetrics;
SystemState::AlarmStatus SystemState::alarmStatus;
SystemState::IncubationStage SystemState::currentStage = SystemState::IDLE;
uint32_t SystemState::stateLastUpdated = 0;

/******************************************************
 * Initialize System State
 ******************************************************/

void SystemState::begin()
{
    Logger::info("System State Manager Initialized");

    // Set default values
    currentStage = IDLE;
    stateLastUpdated = millis();

    // Initialize temperature status
    temperatureStatus.target = 37.5f;      // Chicken incubation temperature
    temperatureStatus.current = 0.0f;
    temperatureStatus.isStable = false;

    // Initialize humidity status
    humidityStatus.target = 60.0f;         // 60% for first 18 days
    humidityStatus.current = 0.0f;
    humidityStatus.isStable = false;

    // Initialize egg turner status
    eggTurnerStatus.turnInterval = 120;    // 2 minutes
    eggTurnerStatus.turnDuration = 5;      // 5 seconds
    eggTurnerStatus.isEnabled = false;

    // Initialize alarm status
    alarmStatus.alarmActive = false;
    alarmStatus.currentAlarmType = ALARM_NONE;
}

/******************************************************
 * Temperature Status: Set
 ******************************************************/

void SystemState::setTemperatureStatus(const TemperatureStatus& status)
{
    temperatureStatus = status;
    temperatureStatus.lastUpdateTime = millis();
}

/******************************************************
 * Temperature Status: Get
 ******************************************************/

SystemState::TemperatureStatus SystemState::getTemperatureStatus()
{
    return temperatureStatus;
}

/******************************************************
 * Update Current Temperature
 ******************************************************/

void SystemState::updateCurrentTemperature(float temp)
{
    temperatureStatus.current = temp;
    temperatureStatus.lastUpdateTime = millis();

    // Update min/max
    if (temperatureStatus.current < temperatureStatus.min || temperatureStatus.min == 0.0f)
    {
        temperatureStatus.min = temperatureStatus.current;
    }
    if (temperatureStatus.current > temperatureStatus.max)
    {
        temperatureStatus.max = temperatureStatus.current;
    }
}

/******************************************************
 * Update Target Temperature
 ******************************************************/

void SystemState::updateTargetTemperature(float temp)
{
    temperatureStatus.target = temp;
}

/******************************************************
 * Set Temperature Stability
 ******************************************************/

void SystemState::setTemperatureStability(bool stable)
{
    temperatureStatus.isStable = stable;
}

/******************************************************
 * Set Heater Active
 ******************************************************/

void SystemState::setHeaterActive(bool active)
{
    temperatureStatus.heaterActive = active;
}

/******************************************************
 * Humidity Status: Set
 ******************************************************/

void SystemState::setHumidityStatus(const HumidityStatus& status)
{
    humidityStatus = status;
    humidityStatus.lastUpdateTime = millis();
}

/******************************************************
 * Humidity Status: Get
 ******************************************************/

SystemState::HumidityStatus SystemState::getHumidityStatus()
{
    return humidityStatus;
}

/******************************************************
 * Update Current Humidity
 ******************************************************/

void SystemState::updateCurrentHumidity(float humidity)
{
    humidityStatus.current = humidity;
    humidityStatus.lastUpdateTime = millis();

    // Update min/max
    if (humidityStatus.current < humidityStatus.min || humidityStatus.min == 0.0f)
    {
        humidityStatus.min = humidityStatus.current;
    }
    if (humidityStatus.current > humidityStatus.max)
    {
        humidityStatus.max = humidityStatus.current;
    }
}

/******************************************************
 * Update Target Humidity
 ******************************************************/

void SystemState::updateTargetHumidity(float humidity)
{
    humidityStatus.target = humidity;
}

/******************************************************
 * Set Humidity Stability
 ******************************************************/

void SystemState::setHumidityStability(bool stable)
{
    humidityStatus.isStable = stable;
}

/******************************************************
 * Set Humidifier Active
 ******************************************************/

void SystemState::setHumidifierActive(bool active)
{
    humidityStatus.humidifierActive = active;
}

/******************************************************
 * Egg Turner Status: Set
 ******************************************************/

void SystemState::setEggTurnerStatus(const EggTurnerStatus& status)
{
    eggTurnerStatus = status;
}

/******************************************************
 * Egg Turner Status: Get
 ******************************************************/

SystemState::EggTurnerStatus SystemState::getEggTurnerStatus()
{
    return eggTurnerStatus;
}

/******************************************************
 * Update Turner Count
 ******************************************************/

void SystemState::updateTurnerCount(uint32_t count)
{
    eggTurnerStatus.totalTurns = count;
}

/******************************************************
 * Update Time Until Next Turn
 ******************************************************/

void SystemState::updateTimeUntilNextTurn(uint32_t timeMs)
{
    eggTurnerStatus.timeUntilNextTurn = timeMs;
}

/******************************************************
 * Set Turner Enabled
 ******************************************************/

void SystemState::setTurnerEnabled(bool enabled)
{
    eggTurnerStatus.isEnabled = enabled;
}

/******************************************************
 * Set Turner Running
 ******************************************************/

void SystemState::setTurnerRunning(bool running)
{
    eggTurnerStatus.isRunning = running;
}

/******************************************************
 * Sensor Status: Set
 ******************************************************/

void SystemState::setSensorStatus(const SensorStatus& status)
{
    sensorStatus = status;
}

/******************************************************
 * Sensor Status: Get
 ******************************************************/

SystemState::SensorStatus SystemState::getSensorStatus()
{
    return sensorStatus;
}

/******************************************************
 * Set Sensor Health
 ******************************************************/

void SystemState::setSensorHealthy(bool tempOk, bool humOk, bool motorOk)
{
    sensorStatus.temperatureSensorOk = tempOk;
    sensorStatus.humiditySensorOk = humOk;
    sensorStatus.motorControlOk = motorOk;
    sensorStatus.lastDiagnosticTime = millis();
}

/******************************************************
 * System Metrics: Set
 ******************************************************/

void SystemState::setSystemMetrics(const SystemMetrics& metrics)
{
    systemMetrics = metrics;
}

/******************************************************
 * System Metrics: Get
 ******************************************************/

SystemState::SystemMetrics SystemState::getSystemMetrics()
{
    return systemMetrics;
}

/******************************************************
 * Update Metrics
 ******************************************************/

void SystemState::updateMetrics(uint32_t uptimeSeconds, float avgTemp, float avgHumidity)
{
    systemMetrics.uptimeSeconds = uptimeSeconds;
    systemMetrics.averageTemperature = avgTemp;
    systemMetrics.averageHumidity = avgHumidity;
}

/******************************************************
 * Update Incubation Day
 ******************************************************/

void SystemState::updateIncubationDay(uint32_t day)
{
    systemMetrics.incubationDayNumber = day;
}

/******************************************************
 * Alarm Status: Set
 ******************************************************/

void SystemState::setAlarmStatus(const AlarmStatus& status)
{
    alarmStatus = status;
}

/******************************************************
 * Alarm Status: Get
 ******************************************************/

SystemState::AlarmStatus SystemState::getAlarmStatus()
{
    return alarmStatus;
}

/******************************************************
 * Trigger Alarm
 ******************************************************/

void SystemState::triggerAlarm(AlarmType type, const String& message)
{
    alarmStatus.alarmActive = true;
    alarmStatus.currentAlarmType = type;
    alarmStatus.alarmMessage = message;
    alarmStatus.alarmStartTime = millis();
    alarmStatus.alarmCount++;
    alarmStatus.buzzerActive = true;

    Logger::error(("ALARM: " + getAlarmTypeName(type) + " - " + message).c_str());
}

/******************************************************
 * Clear Alarm
 ******************************************************/

void SystemState::clearAlarm()
{
    alarmStatus.alarmActive = false;
    alarmStatus.currentAlarmType = ALARM_NONE;
    alarmStatus.alarmMessage = "";
    alarmStatus.buzzerActive = false;
}

/******************************************************
 * Set Buzzer Active
 ******************************************************/

void SystemState::setBuzzerActive(bool active)
{
    alarmStatus.buzzerActive = active;
}

/******************************************************
 * Set Incubation Stage
 ******************************************************/

void SystemState::setIncubationStage(IncubationStage stage)
{
    if (currentStage != stage)
    {
        currentStage = stage;
        Logger::info(("Incubation Stage Changed: " + getStageName(stage)).c_str());
    }
}

/******************************************************
 * Get Incubation Stage
 ******************************************************/

SystemState::IncubationStage SystemState::getIncubationStage()
{
    return currentStage;
}

/******************************************************
 * Get Incubation Stage Name
 ******************************************************/

String SystemState::getIncubationStageName()
{
    return getStageName(currentStage);
}

/******************************************************
 * Is Incubating
 ******************************************************/

bool SystemState::isIncubating()
{
    return (currentStage == INCUBATION || currentStage == LOCKDOWN || currentStage == HATCHING);
}

/******************************************************
 * Get Stage Name String
 ******************************************************/

String SystemState::getStageName(IncubationStage stage)
{
    switch (stage)
    {
        case IDLE:
            return "IDLE";
        case SETUP:
            return "SETUP";
        case INCUBATION:
            return "INCUBATION";
        case LOCKDOWN:
            return "LOCKDOWN";
        case HATCHING:
            return "HATCHING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

/******************************************************
 * Get Alarm Type Name String
 ******************************************************/

String SystemState::getAlarmTypeName(AlarmType type)
{
    switch (type)
    {
        case ALARM_TEMP_HIGH:
            return "Temperature Too High";
        case ALARM_TEMP_LOW:
            return "Temperature Too Low";
        case ALARM_HUMIDITY_HIGH:
            return "Humidity Too High";
        case ALARM_HUMIDITY_LOW:
            return "Humidity Too Low";
        case ALARM_SENSOR_ERROR:
            return "Sensor Error";
        case ALARM_MOTOR_ERROR:
            return "Motor Error";
        case ALARM_POWER_LOSS:
            return "Power Loss Detected";
        case ALARM_NONE:
        default:
            return "No Alarm";
    }
}

/******************************************************
 * Print Full System Status
 ******************************************************/

void SystemState::printFullStatus()
{
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║         FULL SYSTEM STATUS REPORT           ║");
    Serial.println("╠════════════════════════════════════════╣");

    // Incubation Stage
    Serial.print("║ Stage: ");
    Serial.print(getIncubationStageName());
    Serial.println(" ║");

    // Temperature Status
    Serial.print("║ Temperature: ");
    Serial.print(temperatureStatus.current, 1);
    Serial.print("°C (Target: ");
    Serial.print(temperatureStatus.target, 1);
    Serial.print("°C, Stable: ");
    Serial.print(temperatureStatus.isStable ? "YES" : "NO");
    Serial.println(") ║");

    // Humidity Status
    Serial.print("║ Humidity: ");
    Serial.print(humidityStatus.current, 1);
    Serial.print("% (Target: ");
    Serial.print(humidityStatus.target, 1);
    Serial.print("%, Stable: ");
    Serial.print(humidityStatus.isStable ? "YES" : "NO");
    Serial.println(") ║");

    // Egg Turner Status
    Serial.print("║ Turner: ");
    Serial.print(eggTurnerStatus.isEnabled ? "ENABLED" : "DISABLED");
    Serial.print(" (Turn #");
    Serial.print(eggTurnerStatus.totalTurns);
    Serial.println(") ║");

    // Sensor Health
    Serial.print("║ Sensors: Temp=");
    Serial.print(sensorStatus.temperatureSensorOk ? "OK" : "ERROR");
    Serial.print(" Hum=");
    Serial.print(sensorStatus.humiditySensorOk ? "OK" : "ERROR");
    Serial.print(" Motor=");
    Serial.print(sensorStatus.motorControlOk ? "OK" : "ERROR");
    Serial.println(" ║");

    // Alarm Status
    if (alarmStatus.alarmActive)
    {
        Serial.print("║ ALARM: ");
        Serial.print(getAlarmTypeName(alarmStatus.currentAlarmType));
        Serial.println(" ║");
    }
    else
    {
        Serial.println("║ Status: All Systems Normal ║");
    }

    Serial.println("╚════════════════════════════════════════╝\n");
}

/******************************************************
 * Reset System State
 ******************************************************/

void SystemState::reset()
{
    Logger::info("System State Reset");

    currentStage = IDLE;
    temperatureStatus = TemperatureStatus();
    humidityStatus = HumidityStatus();
    eggTurnerStatus = EggTurnerStatus();
    sensorStatus = SensorStatus();
    systemMetrics = SystemMetrics();
    alarmStatus = AlarmStatus();

    // Reset to defaults
    temperatureStatus.target = 37.5f;
    humidityStatus.target = 60.0f;
    eggTurnerStatus.turnInterval = 120;
    eggTurnerStatus.turnDuration = 5;
}
