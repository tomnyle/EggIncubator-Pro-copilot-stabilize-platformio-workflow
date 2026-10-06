# EggIncubator Pro — Low-Voltage Wiring Schematic

This schematic documents the firmware pin map in `include/pins.h`. It covers ESP32 control wiring and module interfaces; it is not a mains-wiring or PCB-fabrication drawing.

```mermaid
flowchart LR
    subgraph MCU["ESP32 DevKit"]
        GND["GND"]
        V3["3V3"]
        I2C_SDA["GPIO21 / SDA"]
        I2C_SCL["GPIO22 / SCL"]
        DS_PIN["GPIO17 / 1-Wire"]
        HEAT["GPIO25"]
        HUM["GPIO26"]
        CIRC["GPIO27"]
        VENT["GPIO14"]
        RPWM["GPIO32 / RPWM"]
        LPWM["GPIO33 / LPWM"]
        EN["GPIO23 / enable"]
        HOME["GPIO18 / HOME"]
        LIMIT_END_PIN["GPIO19 / END"]
        DOOR["GPIO13 / door"]
        BUZZ["GPIO4 / buzzer"]
        LED["GPIO2 / status LED (optional)"]
    end

    subgraph SENSORS["Sensors"]
        SHT["SHT31<br/>I²C address 0x44"]
        DS["DS18B20"]
        RPU["4.7 kΩ pull-up"]
    end

    subgraph RELAYS["3.3 V-compatible, active-low relay/SSR inputs"]
        RH["Heater SSR input"]
        RHU["Humidifier relay IN"]
        RF["Circulation fan relay IN"]
        RV["Ventilation fan relay IN"]
    end

    subgraph MOTOR["BTS7960 motor driver"]
        BR["RPWM"]
        BL["LPWM"]
        BREN["R_EN"]
        BLEN["L_EN"]
        BG["Logic GND"]
        VM["Motor supply input<br/>per motor/driver rating"]
        BRIDGE["BTS7960 H-bridge"]
        MOUT["Motor output"]
    end

    subgraph INPUTS["Dry-contact inputs"]
        SH["HOME limit switch"]
        SE["END limit switch"]
        SD["Door switch"]
    end

    BUZZER["3.3 V logic buzzer module<br/>(or transistor driver)"]
    STATUS["Optional LED + series resistor"]
    MOTOR_SUPPLY["Separate DC motor supply"]
    MOTOR_LOAD["Egg-turner motor"]
    HEATER_LOAD["Heater/load side<br/>rated isolated output"]
    HUM_LOAD["Humidifier/load side"]
    FAN_LOAD["Fan/load side"]
    VENT_LOAD["Ventilation/load side"]

    I2C_SDA <-->|SDA| SHT
    I2C_SCL <-->|SCL| SHT
    V3 -->|VCC| SHT
    GND ---|GND| SHT

    DS_PIN <-->|DQ| DS
    DS_PIN --- RPU
    RPU --- V3
    V3 -->|VDD| DS
    GND ---|GND| DS

    HEAT --> RH --> HEATER_LOAD
    HUM --> RHU --> HUM_LOAD
    CIRC --> RF --> FAN_LOAD
    VENT --> RV --> VENT_LOAD

    RPWM --> BR --> BRIDGE
    LPWM --> BL --> BRIDGE
    EN --> BREN --> BRIDGE
    EN --> BLEN --> BRIDGE
    GND --- BG
    BG --- BRIDGE
    MOTOR_SUPPLY --> VM --> BRIDGE
    BRIDGE --> MOUT --> MOTOR_LOAD

    HOME --- SH --- GND
    LIMIT_END_PIN --- SE --- GND
    DOOR --- SD --- GND
    BUZZ --> BUZZER
    LED --> STATUS
```

## Connection and power notes

| Interface | ESP32 connection | Module/load connection |
|---|---|---|
| SHT31 | GPIO 21 SDA, GPIO 22 SCL, 3V3, GND | Use I²C address `0x44`; keep SDA/SCL separate from 1-Wire |
| DS18B20 | GPIO 17 DQ, 3V3, GND | Fit a 4.7 kΩ pull-up from DQ to 3V3 |
| Heater SSR | GPIO 25 to control input | Use an input-compatible, active-low module; load side must be rated and isolated for the heater |
| Humidifier relay | GPIO 26 to IN | Module supply and load rating per its datasheet |
| Circulation fan relay | GPIO 27 to IN | Module supply and load rating per its datasheet |
| Ventilation fan relay | GPIO 14 to IN | Module supply and load rating per its datasheet |
| BTS7960 | GPIO 32 to RPWM, GPIO 33 to LPWM, GPIO 23 to both R_EN and L_EN | Connect motor output and a separate motor supply according to the driver and motor ratings |
| Limit switches | GPIO 18 HOME, GPIO 19 END | Dry contacts to GND; firmware uses `INPUT_PULLUP` |
| Door switch | GPIO 13 | Dry contact to GND; firmware uses `INPUT_PULLUP` |
| Buzzer | GPIO 4 | Use a 3.3 V-logic buzzer module or a transistor driver; do not exceed ESP32 GPIO current limits |
| Status LED (optional) | GPIO 2 | Add a series resistor; GPIO 2 is a boot-strapping pin, so ensure external circuitry does not prevent boot |

- Power the ESP32 by USB. Power sensors from 3.3 V.
- Select relay/SSR modules whose control inputs are compatible with 3.3 V GPIO and the firmware's active-low relay logic (`RELAY_ON = LOW`). Follow each module's datasheet for its coil/control supply and whether grounds should be common; do not apply 5 V to an ESP32 GPIO.
- Keep the motor supply separate from the ESP32 USB supply. Connect the driver logic ground to ESP32 GND; observe the driver board's power-input and grounding instructions.
- Heater and other load-side wiring is intentionally not specified: the project does not identify load voltages, currents, or exact relay/SSR models. Use correctly rated, isolated switching devices, appropriate fusing/enclosures, and qualified mains wiring where applicable. Never connect mains voltage to the ESP32 or low-voltage side.
