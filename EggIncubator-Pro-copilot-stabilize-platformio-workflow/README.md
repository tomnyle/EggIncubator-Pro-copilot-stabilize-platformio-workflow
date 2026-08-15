# 🥚 EggIncubator Pro

**Smart Egg Incubator with Home Assistant Integration**

A complete IoT solution for automated egg incubation with real-time monitoring and control via Home Assistant MQTT.

---

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Home Assistant Integration](#home-assistant-integration)
- [MQTT Topics](#mqtt-topics)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)

---

## ✨ Features

### 🎯 Core Functionality
- **Temperature Control**: Maintains optimal egg incubation temperature (37.5°C)
- **Humidity Monitoring**: Tracks air humidity levels for optimal conditions
- **Egg Rotation**: Automatic motor control for periodic egg turning
- **Real-time Sensors**:
  - 🌡️ **SHT31**: Air temperature & humidity
  - 🌡️ **DS18B20**: Egg temperature monitoring
- **Safety Features**: Door switch detection, buzzer alarm system

### 🏠 Smart Home Integration
- **Home Assistant Autodiscovery**: Auto-registers all entities
- **MQTT Communication**: Full publish/subscribe support
- **Remote Control**: Toggle heater, fans, humidifier via Home Assistant
- **Diagnostics**: WiFi RSSI, free heap memory, uptime tracking
- **Last Will Testament**: Device availability status

### ⚙️ Technical Features
- **Modular Architecture**: Clean separation of concerns
- **Task Scheduler**: Periodic sensor updates and system checks
- **Error Handling**: Sensor validation and safe fallback states
- **Logging**: Serial debug logs with different log levels
- **Low-Power Friendly**: Efficient WiFi reconnection logic

---

## 🔧 Hardware Requirements

### Microcontroller
- **ESP32 Dev Board** (or compatible ESP32 variant)

### Sensors
- **SHT31 Temperature/Humidity Sensor** (I2C, address 0x44)
- **DS18B20 Temperature Sensor** (1-Wire)

### Actuators
- **Heater Control**: SSR Relay (pin 25)
- **Humidifier**: Relay Module (pin 26)
- **Circulation Fan**: Relay Module (pin 27)
- **Ventilation Fan**: Relay Module (pin 14)
- **Egg Turner Motor**: BTS7960 Motor Driver (pins 32, 33, 23)

### Safety & Input
- **Door Switch**: Magnetic contact sensor (pin 13, INPUT_PULLUP)
- **Limit Switches**: Home/End positions for motor (pins 18, 19)
- **Buzzer**: Alarm output (pin 4)

### Power
- **ESP32**: 5V USB
- **Heater/Fans**: 12V DC
- **Motor**: 12V DC with BTS7960 driver

### Communication
- **WiFi**: Built-in ESP32 WiFi
- **MQTT Broker**: External MQTT server (e.g., Mosquitto, Home Assistant)

---

## 📐 Wiring Diagram

```
ESP32 Board
├── I2C Bus (pins 21, 22)
│   ├── SHT31 VCC → 3.3V
│   ├── SHT31 GND → GND
│   ├── SHT31 SDA → GPIO 21
│   └── SHT31 SCL → GPIO 22
│
├── 1-Wire (pin 21 currently, can be changed)
│   ├── DS18B20 VCC → 3.3V
│   ├── DS18B20 GND → GND
│   └── DS18B20 DQ → GPIO 21 (with 4.7K pullup)
│
├── Relay Outputs (GPIO)
│   ├── GPIO 25 → Heater SSR
│   ├── GPIO 26 → Humidifier
│   ├── GPIO 27 → Circulation Fan
│   ├── GPIO 14 → Ventilation Fan
│   └── GPIO 4 → Buzzer
│
├── Motor Driver (BTS7960)
│   ├── GPIO 32 → RPWM
│   ├── GPIO 33 → LPWM
│   ├── GPIO 23 → ENABLE
│
├── Input Sensors
│   ├── GPIO 18 → Limit Switch HOME
│   ├── GPIO 19 → Limit Switch END
│   └── GPIO 13 → Door Switch
│
└── Power
    ├── 5V → USB Power
    └── 12V → Motor/Relay Power (separate supply)
```

---

## 📦 Installation

### 1. Prerequisites
- Visual Studio Code with PlatformIO extension
- MQTT Broker (Mosquitto or Home Assistant)
- WiFi network access

### 2. Clone Repository
```bash
git clone https://github.com/tomnyle/EggIncubator-Pro.git
cd EggIncubator-Pro
```

### 3. Install Dependencies (Automatic)
PlatformIO will automatically install libraries from `platformio.ini`:
- PubSubClient (MQTT)
- Adafruit SHT31 Library (Temperature/Humidity)
- DallasTemperature (DS18B20)
- AccelStepper (Motor control)
- ArduinoJson (JSON serialization)

### 4. Configure WiFi & MQTT
Edit `include/app_config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID            "Your_WiFi_SSID"
#define WIFI_PASSWORD        "Your_WiFi_Password"

// MQTT Configuration
#define MQTT_HOST            "192.168.100.100"  // MQTT Broker IP
#define MQTT_PORT            1883
#define MQTT_USERNAME        "mqtt_user"
#define MQTT_PASSWORD        "mqtt_password"

// Device Configuration
#define DEVICE_ID            "eggincubator01"
```

### 5. Build & Upload
```bash
# In VS Code with PlatformIO
# Press Ctrl+Alt+U to upload
# Or use terminal:
pio run --target upload
```

### 6. Monitor Serial Output
```bash
pio device monitor --baud 115200
```

---

## ⚙️ Configuration

### Pin Configuration
Edit `include/pins.h` to change hardware pins:

```cpp
#define PIN_I2C_SDA          21    // I2C Data
#define PIN_I2C_SCL          22    // I2C Clock

#define PIN_HEATER_SSR       25    // Heater
#define PIN_HUMIDIFIER       26    // Humidifier
#define PIN_FAN_CIRCULATION  27    // Circulation Fan
#define PIN_FAN_VENT         14    // Ventilation Fan

#define PIN_BTS_RPWM         32    // Motor Forward
#define PIN_BTS_LPWM         33    // Motor Backward
#define PIN_BTS_ENABLE       23    // Motor Enable
```

### Temperature/Humidity Targets
Default targets in `include/system_state.h`:

```cpp
struct TargetState {
    float temperature = 37.5f;  // Optimal for chicken eggs
    float humidity = 60.0f;      // Relative humidity %
};
```

---

## 🚀 Usage

### Initial Boot
1. Power on ESP32
2. Check serial monitor for initialization messages
3. Device will attempt WiFi connection
4. Once WiFi connected, MQTT connection will start
5. Home Assistant discovery entities will be published

### Manual Control (Serial Monitor)
Currently controlled via MQTT. Future versions may add serial commands.

### Status Checking
Monitor serial output for:
```
[INFO] WiFi Connected
[INFO] MQTT Connected
[INFO] Discovery Published
```

---

## 🏠 Home Assistant Integration

### Automatic Setup (Recommended)
1. EggIncubator Pro automatically publishes MQTT discovery messages
2. Home Assistant detects the device automatically
3. All entities appear in Home Assistant without manual configuration

### Manual Setup (If needed)
Add to Home Assistant `configuration.yaml`:

```yaml
mqtt:
  broker: 192.168.100.100
  username: mqtt_user
  password: mqtt_password

homeassistant:
  unit_system: metric
```

### Available Entities in Home Assistant
| Entity | Type | Description |
|--------|------|-------------|
| `heater` | Switch | Control heating element |
| `humidifier` | Switch | Control humidifier |
| `fan` | Switch | Circulation fan |
| `ventilation` | Switch | Ventilation fan |
| `air_temperature` | Sensor | SHT31 temperature (°C) |
| `air_humidity` | Sensor | SHT31 humidity (%) |
| `egg_temperature` | Sensor | DS18B20 temperature (°C) |
| `firmware` | Sensor | Firmware version |
| `status` | Binary Sensor | Device online status |

### Example Automation
```yaml
automation:
  - alias: "Incubator - Heater Control"
    trigger:
      platform: numeric_state
      entity_id: sensor.eggincubator01_air_temperature
      below: 37.5
    action:
      service: switch.turn_on
      entity_id: switch.eggincubator01_heater
```

---

## 📡 MQTT Topics

### Device Status (Published)
```
eggincubator/status          → "online" / "offline"
eggincubator/firmware        → "0.3.0"
eggincubator/ip              → "192.168.1.100"
eggincubator/rssi            → "-45" (WiFi signal strength)
eggincubator/uptime          → "3600" (seconds)
eggincubator/free_heap       → "180000" (bytes)
```

### Sensor Data (Published every 5 seconds)
```
eggincubator/air_temperature     → "37.2" (°C)
eggincubator/air_humidity        → "60.5" (%)
eggincubator/egg_temperature     → "37.1" (°C)
```

### Relay States (Published)
```
eggincubator/heater/state        → "ON" / "OFF"
eggincubator/humidifier/state    → "ON" / "OFF"
eggincubator/fan/state           → "ON" / "OFF"
eggincubator/vent/state          → "ON" / "OFF"
```

### Control Commands (Subscribe)
```
eggincubator/heater/set          ← "ON" / "OFF"
eggincubator/humidifier/set      ← "ON" / "OFF"
eggincubator/fan/set             ← "ON" / "OFF"
eggincubator/vent/set            ← "ON" / "OFF"
```

### Home Assistant Discovery
```
homeassistant/switch/eggincubator01/heater/config
homeassistant/sensor/eggincubator01/air_temperature/config
homeassistant/sensor/eggincubator01/air_humidity/config
homeassistant/sensor/eggincubator01/egg_temperature/config
```

---

## 🔧 Troubleshooting

### WiFi Connection Issues
**Problem**: Device won't connect to WiFi
- Check WiFi SSID and password in `app_config.h`
- Verify WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check serial log: `[WARNING] WiFi Disconnected`

**Solution**:
```cpp
// Monitor WiFi status
[INFO] Connecting WiFi...
[INFO] WiFi Connected
[INFO] IP: 192.168.1.100
```

### MQTT Connection Issues
**Problem**: MQTT fails to connect
- Verify MQTT broker IP and port
- Check username/password credentials
- Ensure MQTT broker is running

**Check in serial log**:
```
[INFO] Connecting MQTT...
[INFO] MQTT Connected
[INFO] Discovery Published
```

### Sensor Reading Errors
**Problem**: Sensor values show 0 or invalid

**SHT31 (I2C)**:
- Verify I2C address is 0x44
- Check SDA/SCL connections (pins 21, 22)
- Look for: `✗ SHT31 NOT FOUND`

**DS18B20 (1-Wire)**:
- Verify 1-Wire pin connection (currently pin 21)
- Check 4.7K pullup resistor
- Look for: `✗ DS18B20 NOT FOUND`

### Motor Control Issues
**Problem**: Motor doesn't rotate
- Check BTS7960 enable pin (23)
- Verify PWM pins (32, 33)
- Check 12V motor power supply
- Inspect limit switch connections (18, 19)

### Home Assistant Discovery Issues
**Problem**: Entities don't appear in Home Assistant
- Check MQTT broker connection is working
- Verify Home Assistant MQTT integration is enabled
- Look for discovery messages in MQTT explorer
- Check Home Assistant logs for errors

---

## 📁 Project Structure

```
EggIncubator-Pro/
│
├── include/                    # Header files
│   ├── app_config.h           # WiFi & MQTT configuration
│   ├── pins.h                 # Hardware pin definitions
│   ├── system_state.h         # Global state structures
│   ├── gpio_manager.h         # GPIO initialization
│   ├── relay_controller.h     # Relay control (heater, fans)
│   ├── bts7960.h              # Motor driver class
│   ├── sensor_manager.h       # SHT31 & DS18B20 sensors
│   ├── network_service.h      # WiFi manager
│   ├── mqtt_service.h         # MQTT client wrapper
│   ├── discovery_service.h    # Home Assistant discovery
│   ├── incubator_controller.h # Main control logic
│   ├── scheduler.h            # Task scheduler
│   └── logger.h               # Serial logging
│
├── src/                        # Implementation files
│   ├── main.cpp               # Setup & main loop
│   ├── gpio_manager.cpp       # GPIO initialization
│   ├── relay_controller.cpp   # Relay control logic
│   ├── bts7960.cpp            # Motor driver implementation
│   ├── sensor_manager.cpp     # Sensor reading logic
│   ├── network_service.cpp    # WiFi connection logic
│   ├── mqtt_service.cpp       # MQTT publish/subscribe
│   ├── discovery_service.cpp  # HA discovery entities
│   ├── incubator_controller.cpp # Main control logic
│   ├── scheduler.cpp          # Task scheduler logic
│   └── logger.cpp             # Logging implementation
│
├── platformio.ini              # Build configuration
└── README.md                   # This file
```

---

## 🔄 System Flow

```
┌─────────────────────────────────────────┐
│         System Startup (setup)          │
├─────────────────────────────────────────┤
│ 1. Initialize Logger (Serial 115200)    │
│ 2. Initialize GPIO & Relays             │
│ 3. Initialize Motor Driver              │
│ 4. Initialize Sensors (SHT31, DS18B20)  │
│ 5. Connect to WiFi                      │
│ 6. Connect to MQTT                      │
│ 7. Publish Device Discovery             │
└─────────────────────────────────────────┘
           ⬇
┌─────────────────────────────────────────┐
│        Main Loop (every iteration)      │
├─────────────────────────────────────────┤
│ 1. Check WiFi connection                │
│ 2. Process MQTT messages                │
│ 3. Update Controller state              │
│ 4. Publish diagnostics (every 5s)       │
│ 5. Run scheduled tasks                  │
│   - Update sensors (every 2s-3s)        │
│   - Check system health                 │
└─────────────────────────────────────────┘
```

---

## 📊 Sensor Update Intervals

| Sensor | Interval | Purpose |
|--------|----------|---------|
| SHT31 (Air T/H) | 2 seconds | Monitor ambient conditions |
| DS18B20 (Egg T) | 3 seconds | Monitor egg temperature |
| MQTT Publish | 5 seconds | Send data to Home Assistant |
| WiFi Reconnect | 5 seconds | Maintain connectivity |

---

## 🛠️ Development Notes

### Adding New Features
1. Create header file in `include/`
2. Implement in `src/`
3. Add initialization to `main.cpp`
4. Update MQTT topics if data publishing
5. Add discovery entities if Home Assistant entity

### Extending Sensor Support
Add new sensor in `sensor_manager.h/.cpp`:
```cpp
static float getNewSensorValue();
static bool isNewSensorReady();
```

### Custom Control Logic
Modify `incubator_controller.cpp`:
- `applyOutputs()` - control decisions
- `requestHeater()` - heater control
- Add PID controller for temperature

---

## 📝 License

This project is open source and available under the MIT License.

---

## 👤 Author

**Le Danh** - Initial development and design

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

---

## 📞 Support

For issues and questions:
1. Check [Troubleshooting](#troubleshooting) section
2. Review serial logs for error messages
3. Check MQTT topics are being published
4. Verify Home Assistant MQTT integration

---

**Happy Incubating! 🐣**
