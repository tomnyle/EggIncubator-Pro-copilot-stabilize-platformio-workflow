# EggIncubator Pro — Low-Voltage Wiring Schematic

This schematic documents the firmware pin map in `include/pins.h` and expands it with four PC817 relay-control channels, a buzzer transistor driver, a generic 12 V-to-5 V converter interface, and an auxiliary sensor header. It remains a module-level reference, not a production PCB or mains-wiring drawing.

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

    subgraph RELAYS["External relay / SSR modules"]
        RH["Heater module IN"]
        RHU["Humidifier module IN"]
        RF["Circulation fan module IN"]
        RV["Ventilation fan module IN"]
        RGV["External relay supply"]
        RGG["External relay ground"]
    end

    subgraph OPTO["PC817 channels (4x), open-collector outputs"]
        OH["Heater PC817"]
        OHU["Humidifier PC817"]
        OF["Circulation fan PC817"]
        OV["Ventilation fan PC817"]
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

    BUZZER["5 V buzzer module"]
    QBUZ["PN2222A low-side switch<br/>1 kΩ base resistor"]
    BUCK["12 V to 5 V buck module"]
    VIN["12 V DC input"]
    V5["5 V to ESP32 DevKit VIN"]
    AUX["Auxiliary sensor header<br/>I²C + reserved ADC nets"]
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

    V3 -->|330 Ω| OH -->|active-low GPIO25| HEAT
    V3 -->|330 Ω| OHU -->|active-low GPIO26| HUM
    V3 -->|330 Ω| OF -->|active-low GPIO27| CIRC
    V3 -->|330 Ω| OV -->|active-low GPIO14| VENT
    OH -->|open collector| RH
    OHU -->|open collector| RHU
    OF -->|open collector| RF
    OV -->|open collector| RV
    RGV --- RH
    RGV --- RHU
    RGV --- RF
    RGV --- RV
    RGG --- OH
    RGG --- OHU
    RGG --- OF
    RGG --- OV

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
    BUZZ --> QBUZ --> BUZZER
    LED --> STATUS
    VIN --> BUCK --> V5
    V3 --> AUX
    I2C_SDA --- AUX
    I2C_SCL --- AUX
```

## Connection and power notes

| Interface | ESP32 connection | Module/load connection |
|---|---|---|
| SHT31 | GPIO 21 SDA, GPIO 22 SCL, 3V3, GND | Use I²C address `0x44`; keep SDA/SCL separate from 1-Wire |
| DS18B20 | GPIO 17 DQ, 3V3, GND | Fit a 4.7 kΩ pull-up from DQ to 3V3 |
| Heater SSR | GPIO 25 drives a PC817 LED through 330 Ω | PC817 collector sinks the external module IN; supply it from its own relay-side rail |
| Humidifier relay | GPIO 26 drives a PC817 LED through 330 Ω | Check optocoupler sink-current capability against module input current |
| Circulation fan relay | GPIO 27 drives a PC817 LED through 330 Ω | Check optocoupler sink-current capability against module input current |
| Ventilation fan relay | GPIO 14 drives a PC817 LED through 330 Ω | Check optocoupler sink-current capability against module input current |
| BTS7960 | GPIO 32 to RPWM, GPIO 33 to LPWM, GPIO 23 to both R_EN and L_EN | Connect motor output and a separate motor supply according to the driver and motor ratings |
| Limit switches | GPIO 18 HOME, GPIO 19 END | Dry contacts to GND; firmware uses `INPUT_PULLUP` |
| Door switch | GPIO 13 | Dry contact to GND; firmware uses `INPUT_PULLUP` |
| Buzzer | GPIO 4 through R6 (1 kΩ) to PN2222A base | J7 supplies the buzzer from 5 V; confirm module voltage/current rating |
| Auxiliary sensors | J8 exposes 3V3, GND, SDA, SCL, and two reserved ADC nets | ADC nets are not assigned to firmware GPIOs yet; check sensor voltage before connecting |
| Status LED (optional) | GPIO 2 | Add a series resistor; GPIO 2 is a boot-strapping pin, so ensure external circuitry does not prevent boot |

- J9/U9/J10 show an optional 12 V DC input, generic buck module, and 5 V DevKit input. Verify the selected converter's voltage, current, polarity, and fuse requirements. Do not connect USB and external 5 V together unless the selected DevKit allows it.
- Relay channels are active-low: a LOW GPIO turns on its PC817 LED. The output side is an open-collector sink; connect each external relay input and its relay-side supply/ground as required by that module. Keep `RELAY_GND` separate from MCU GND if isolation is required.
- The 330 Ω input resistors provide roughly 6 mA LED current from a 3.3 V rail with a typical PC817 LED drop. Verify worst-case PC817 CTR/output sink current against the actual relay/SSR input; use an additional suitable driver if it is insufficient. The schematic does not guarantee compatibility with arbitrary modules.
- Sensor power is 3.3 V only for sensors rated for it. J8's ADC nets are reserved; select safe ESP32 ADC pins and update firmware before using them.
- Keep the motor supply separate from the ESP32 USB supply. Connect the driver logic ground to ESP32 GND; observe the driver board's power-input and grounding instructions.
- Heater and other load-side wiring is intentionally not specified: the project does not identify load voltages, currents, or exact relay/SSR models. Use correctly rated, isolated switching devices, appropriate fusing/enclosures, and qualified mains wiring where applicable. Never connect mains voltage to the ESP32 or low-voltage side.
- Converter and connector footprints, load-dependent fuse ratings, PCB placement/routing, creepage/clearance, thermal design, and DRC remain to be completed after exact modules and loads are selected. This schematic is not ready for PCB fabrication.
