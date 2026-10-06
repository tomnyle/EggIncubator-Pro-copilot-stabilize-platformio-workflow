# EggIncubator Pro — Low-Voltage Wiring Schematic

This schematic is derived from the firmware pin map in `include/pins.h` and the hardware interfaces described by the project; it does not depend on the reference image. U5 is shown as a symbolic ESP32 DevKit V1 module interface, with project-used GPIO functions and a 5 V VIN connection; it is not a bare ESP32 chip or the exact physical pinout of every DevKit variant. The drawing also covers four PC817 relay-control channels and output pull-ups, a buzzer transistor driver, a fused 12 V-to-5 V converter interface with bulk capacitors, and an auxiliary sensor header. It remains a module-level reference, not a production PCB or mains-wiring drawing.

```mermaid
flowchart LR
    subgraph MCU["U5: ESP32 DevKit V1"]
        GND["GND"]
        V3["3V3"]
        MCU_VIN["VIN / 5V"]
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
        SHT["J2 SHT31<br/>I²C address 0x44"]
        DS["J3 DS18B20"]
        DS_RPU["R1 4.7 kΩ pull-up"]
        RSDA["R11 optional 4.7 kΩ SDA pull-up"]
        RSCL["R12 optional 4.7 kΩ SCL pull-up"]
    end

    subgraph RELAYS["J4: External relay / SSR interface"]
        RH["Heater module IN"]
        RHU["Humidifier module IN"]
        RF["Circulation fan module IN"]
        RV["Ventilation fan module IN"]
        RGV["External relay supply"]
        RGG["External relay ground"]
        RPU_H["R7 4.7 kΩ heater CTRL pull-up"]
        RPU_HU["R8 4.7 kΩ humidifier CTRL pull-up"]
        RPU_CF["R9 4.7 kΩ circulation fan CTRL pull-up"]
        RPU_VF["R10 4.7 kΩ vent fan CTRL pull-up"]
    end

    subgraph OPTO["U1-U4: PC817 channels, open-collector outputs"]
        OH["U1 heater"]
        OHU["U2 humidifier"]
        OF["U3 circulation fan"]
        OV["U4 ventilation fan"]
    end

    subgraph MOTOR["J5: BTS7960 motor-driver interface"]
        BR["RPWM"]
        BL["LPWM"]
        BREN["R_EN"]
        BLEN["L_EN"]
        BG["Logic GND"]
        BTS_5V["Logic 5V"]
        VM["Motor supply input<br/>per motor/driver rating"]
        BRIDGE["BTS7960 H-bridge"]
        MOUT["Motor output"]
    end

    subgraph INPUTS["J6: Dry-contact inputs"]
        SH["HOME limit switch"]
        SE["END limit switch"]
        SD["Door switch"]
    end

    BUZZER["J7 5 V buzzer module"]
    QBUZ["Q1 PN2222A low-side switch<br/>R6 1 kΩ base resistor"]
    BUCK["U9 12 V to 5 V buck module"]
    VIN["J9 12 V DC input"]
    FUSE["F1 load-rated fuse"]
    MOTOR_VIN["J11 motor supply input"]
    MOTOR_FUSE["F2 motor-rated fuse"]
    CIN["C1 470 µF / 25 V"]
    COUT["C2 470 µF / 10 V"]
    POWER_GND["Power return / GND"]
    V5["J10 DEVKIT_5V"]
    AUX["J8 auxiliary sensor header<br/>I²C + reserved ADC nets"]
    STATUS["D1 + R13 status LED"]
    MOTOR_SUPPLY["Separate DC motor supply"]
    MOTOR_CAP["Optional 470 µF input capacitor<br/>only if driver board lacks bulk capacitance"]
    MOTOR_LOAD["Egg-turner motor"]
    HEATER_LOAD["Heater/load side<br/>rated isolated output"]
    HUM_LOAD["Humidifier/load side"]
    FAN_LOAD["Fan/load side"]
    VENT_LOAD["Ventilation/load side"]

    V5 -->|DEVKIT_5V| MCU_VIN
    I2C_SDA <-->|SDA| SHT
    I2C_SCL <-->|SCL| SHT
    V3 --> RSDA --> I2C_SDA
    V3 --> RSCL --> I2C_SCL
    V3 -->|VCC| SHT
    GND ---|GND| SHT

    DS_PIN <-->|DQ| DS
    DS_PIN --- DS_RPU
    DS_RPU --- V3
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
    RGV --> RPU_H --> RH
    RGV --> RPU_HU --> RHU
    RGV --> RPU_CF --> RF
    RGV --> RPU_VF --> RV
    RGG --- OH
    RGG --- OHU
    RGG --- OF
    RGG --- OV

    RPWM --> BR --> BRIDGE
    LPWM --> BL --> BRIDGE
    EN --> BREN --> BRIDGE
    EN --> BLEN --> BRIDGE
    GND --- BG
    V5 -->|DEVKIT_5V| BTS_5V
    BTS_5V --> BRIDGE
    BG --- BRIDGE
    MOTOR_VIN --> MOTOR_FUSE --> MOTOR_SUPPLY --> VM --> BRIDGE
    GND --- MOTOR_SUPPLY
    MOTOR_SUPPLY --- MOTOR_CAP
    MOTOR_CAP --- GND
    BRIDGE --> MOUT --> MOTOR_LOAD

    HOME --- SH --- GND
    LIMIT_END_PIN --- SE --- GND
    DOOR --- SD --- GND
    V5 -->|DEVKIT_5V| BUZZER
    BUZZ --> QBUZ --> BUZZER
    LED --> STATUS
    VIN --> FUSE --> BUCK --> V5
    BUCK --- CIN
    CIN --- POWER_GND
    COUT --- V5
    COUT --- POWER_GND
    V3 --> AUX
    I2C_SDA --- AUX
    I2C_SCL --- AUX
```

## Connection and power notes

| Interface | ESP32 connection | Module/load connection |
|---|---|---|
| SHT31 (J2) | GPIO 21 SDA, GPIO 22 SCL, 3V3, GND | Use I²C address `0x44`; keep SDA/SCL separate from 1-Wire |
| DS18B20 (J3) | GPIO 17 DQ, 3V3, GND | R1 is the 4.7 kΩ pull-up from DQ to 3V3 |
| Relay interface (J4) | GPIO 25/26/27/14 drive U1-U4 PC817 LEDs through 330 Ω | R7-R10 pull up isolated CTRL outputs; pins 5-6 provide `RELAY_VCC`/`RELAY_GND` |
| BTS7960 interface (J5) | GPIO 32 to RPWM, GPIO 33 to LPWM, GPIO 23 to both R_EN and L_EN | Connect motor output and a separate motor supply according to the driver and motor ratings |
| Limit switches (J6) | GPIO 18 HOME, GPIO 19 END | Dry contacts to GND; firmware uses `INPUT_PULLUP` |
| Door switch (J6) | GPIO 13 | Dry contact to GND; firmware uses `INPUT_PULLUP` |
| Buzzer (J7) | GPIO 4 through R6 (1 kΩ) to Q1 PN2222A base | J7 supplies the buzzer from `DEVKIT_5V`; confirm module voltage/current rating |
| Auxiliary sensors (J8) | J8 exposes 3V3, GND, SDA, SCL, and two reserved ADC nets | ADC nets are not assigned to firmware GPIOs yet; check sensor voltage before connecting |
| Status LED (D1/R13) | GPIO 2 through R13 (1 kΩ) and D1 to GND | GPIO 2 is a boot-strapping pin; ensure the LED circuit does not prevent boot |

- U5 is a schematic symbol for an ESP32 DevKit V1 module interface, not a board-specific footprint. GPIO assignments follow the firmware; verify the physical board's pin labels and VIN/USB power arrangement before wiring.
- J9/F1/U9/J10 show a 12 V DC input, series fuse, generic buck module, and 5 V `DEVKIT_5V` input. C1/C2 show illustrative 470 µF bulk decoupling to GND; validate voltage, ripple, inrush, fuse, and converter current ratings for the actual design. Do not connect USB and external 5 V together unless the selected DevKit allows it.
- Relay channels are active-low: a LOW GPIO turns on its PC817 LED. Each open-collector output has a 4.7 kΩ pull-up to the isolated `RELAY_VCC` rail and is pulled low by its PC817. J4 pins 5–6 are external supply input/return connections for `RELAY_VCC` and `RELAY_GND`; the board does not generate this isolated supply. Use only a voltage accepted by the selected relay module (no single relay-side voltage is specified here). Verify optocoupler sink current and logic thresholds, and keep `RELAY_GND` separate from MCU GND.
- The 330 Ω input resistors provide roughly 6 mA LED current from a 3.3 V rail with a typical PC817 LED drop. Verify worst-case PC817 CTR/output sink current against the actual relay/SSR input; use an additional suitable driver if it is insufficient. The schematic does not guarantee compatibility with arbitrary modules.
- Sensor power is 3.3 V only for sensors rated for it. J8's ADC nets are reserved; select safe ESP32 ADC pins and update firmware before using them.
- R11/R12 provide optional 4.7 kΩ I²C pull-ups and are marked DNP; populate only if the connected sensor modules do not already provide suitable pull-ups.
- J11/F2 are an explicit, separately fused motor-supply input to the BTS7960 `VMOT_PLUS` pin. Choose the fuse from motor stall/inrush current, wiring, and module ratings; the motor positive rail is not supplied by the 5 V buck.
- Keep the motor supply positive separate from the ESP32 USB/5 V rail. A typical BTS7960 module has common logic and motor return; J5 therefore connects both grounds to system GND. Confirm this against the exact driver board before wiring.
- C3 is optional and DNP by default; populate a correctly rated bulk capacitor near the BTS7960 supply only if the selected module does not already provide adequate input capacitance. Do not place a capacitor across switched motor outputs unless the exact driver manufacturer recommends it.
- Heater and other load-side wiring is intentionally not specified: the project does not identify load voltages, currents, or exact relay/SSR models. Use correctly rated, isolated switching devices, appropriate fusing/enclosures, and qualified mains wiring where applicable. Never connect mains voltage to the ESP32 or low-voltage side.
- Converter and connector footprints, load-dependent fuse ratings, PCB placement/routing, creepage/clearance, thermal design, and DRC remain to be completed after exact modules and loads are selected. This schematic is not ready for PCB fabrication.
