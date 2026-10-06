# EggIncubator Pro — Hardware Wiring

This is a low-voltage wiring reference for the current firmware pin map, not a
certified circuit schematic. Confirm the exact ESP32 board and every module's
datasheet before wiring. The current firmware uses GPIO 16 for the DS18B20;
GPIO 16 may be reserved on ESP32 modules with PSRAM.

## Connection diagram

```mermaid
flowchart LR
    subgraph ESP32["ESP32 Dev Board"]
        V33["3V3"]
        GND["GND"]
        SDA["GPIO 21 / SDA"]
        SCL["GPIO 22 / SCL"]
        OW["GPIO 16 / 1-Wire"]
        HEAT["GPIO 25"]
        HUM["GPIO 26"]
        FAN["GPIO 27"]
        VENT["GPIO 14"]
        RPWM["GPIO 32"]
        LPWM["GPIO 33"]
        EN["GPIO 23"]
        HOME["GPIO 18 / INPUT_PULLUP"]
        END["GPIO 19 / INPUT_PULLUP"]
        DOOR["GPIO 13 / INPUT_PULLUP"]
        BUZZ["GPIO 4"]
    end

    subgraph SENSORS["Low-voltage sensors"]
        SHT["SHT31"]
        DS["DS18B20"]
        PULL["4.7 kΩ pull-up"]
    end

    subgraph INPUTS["Dry-contact switches"]
        SW_HOME["Home limit switch"]
        SW_END["End limit switch"]
        SW_DOOR["Door switch"]
    end

    subgraph RELAYS["Relay / SSR input modules — active LOW"]
        RHEAT["Heater SSR input"]
        RHUM["Humidifier relay input"]
        RFAN["Circulation fan relay input"]
        RVENT["Ventilation fan relay input"]
    end

    subgraph MOTOR["BTS7960 motor driver"]
        MRPWM["RPWM"]
        MLPWM["LPWM"]
        MEN["R_EN and L_EN"]
        MBP["B+ motor supply"]
        MBN["B− motor supply"]
        MOUTP["M+"]
        MOUTN["M−"]
        MGND["Logic GND"]
    end

    subgraph MOTORPSU["Motor supply — voltage per motor/driver ratings"]
        PSU_POS["Supply +"]
        PSU_NEG["Supply −"]
    end

    subgraph LOADS["Loads — voltage/current per nameplate"]
        HEATER["Heater"]
        HUMLOAD["Humidifier"]
        FANLOAD["Circulation fan"]
        VENTLOAD["Ventilation fan"]
        MOTORLOAD["Egg-turner motor"]
    end

    V33 -->|"3.3 V"| SHT
    GND -->|"GND"| SHT
    SDA <-->|"SDA"| SHT
    SCL <-->|"SCL"| SHT

    V33 -->|"3.3 V"| DS
    GND -->|"GND"| DS
    OW <-->|"DQ / data"| DS
    V33 --- PULL
    PULL --- OW

    HOME --> SW_HOME
    END --> SW_END
    DOOR --> SW_DOOR
    SW_HOME -->|"other contact to GND"| GND
    SW_END -->|"other contact to GND"| GND
    SW_DOOR -->|"other contact to GND"| GND

    HEAT -->|"GPIO 25"| RHEAT
    HUM -->|"GPIO 26"| RHUM
    FAN -->|"GPIO 27"| RFAN
    VENT -->|"GPIO 14"| RVENT
    GND -.->|"logic ground; if module requires it"| RELAYS
    RHEAT --> HEATER
    RHUM --> HUMLOAD
    RFAN --> FANLOAD
    RVENT --> VENTLOAD

    RPWM --> MRPWM
    LPWM --> MLPWM
    EN -->|"GPIO 23 to both enables"| MEN
    GND --> MGND
    PSU_POS --> MBP
    PSU_NEG --> MBN
    MOUTP -->|"motor lead 1"| MOTORLOAD
    MOUTN -->|"motor lead 2"| MOTORLOAD

    BUZZ -->|"through transistor driver; not directly to a high-current buzzer"| BUZZER["Buzzer"]
```

## Pin-to-wire reference

| ESP32 pin | Connect to | Notes |
|---|---|---|
| 3V3, GND | SHT31 VIN/VCC, GND | Use the sensor breakout's supported supply voltage. |
| GPIO 21, GPIO 22 | SHT31 SDA, SCL | I²C; SHT31 address is configured as `0x44`. |
| GPIO 16, 3V3, GND | DS18B20 DQ, VDD, GND | Add a 4.7 kΩ pull-up from DQ to 3V3. Confirm GPIO 16 is available on the specific board. |
| GPIO 25, 26, 27, 14 | Heater SSR, humidifier, circulation fan, ventilation relay inputs | Firmware configures these as active-low (`RELAY_ON=LOW`). Use 3.3 V-compatible inputs; verify the module's input polarity and isolation. |
| GPIO 32, GPIO 33 | BTS7960 RPWM, LPWM | Firmware drives these as direction outputs, not variable-speed PWM. |
| GPIO 23 | BTS7960 R_EN and L_EN | Tie both enable inputs together only if the driver's datasheet permits it. |
| GPIO 18, 19, 13 | Home limit, end limit, door switch | Each normally-open dry contact goes between its GPIO and ESP32 GND; firmware uses `INPUT_PULLUP`. Check switch polarity and desired fail-safe behavior. |
| GPIO 4 | Buzzer driver input | Use a transistor/MOSFET driver if the buzzer current exceeds the GPIO rating; add suppression for an inductive load. |

## Power and safety

- Power the ESP32 from its rated USB/5 V input. Power the motor and loads from
  supplies rated for their voltage and current; do not power them from ESP32
  GPIO pins or the 3.3 V rail.
- Join grounds for the low-voltage ESP32, sensor, relay logic, and motor-driver
  logic where required by the module datasheets. Keep relay isolation barriers
  intact.
- The diagram does not specify heater or mains wiring. Use properly rated
  enclosures, fusing, strain relief, clearances, and an independent thermal
  cutoff. Have mains wiring designed and checked by a qualified electrician.
- Test relay polarity and motor direction with loads disconnected first.
  Confirm all outputs stay off at boot, reset, and power loss. The software
  safe state cannot guarantee the electrical behavior of the relay hardware.
- Confirm the selected ESP32 board exposes every listed GPIO and that none are
  reserved by flash/PSRAM or board peripherals before connecting hardware.
- The current firmware configures the limit-switch and door GPIOs, but does not
  yet read them or apply safety logic based on their state.
