# EggIncubator Pro — REV3 development build

**Not production-ready.** Heater, humidifier and DM542 motion are disabled by
default. Disconnect all power loads during initial checks. Do not use this
build for incubation.

## REV3 signal mapping

| Function | ESP32 pin |
|---|---:|
| SHT31 SDA / SCL | GPIO21 / GPIO22 |
| DS18B20 data | GPIO16 |
| Heater / humidifier | GPIO25 / GPIO26 |
| Circulation / ventilation fan | GPIO27 / GPIO14 |
| DM542 STEP / DIR / ENA | GPIO32 / GPIO33 / GPIO23 |
| HOME / END limit switches | GPIO18 / GPIO19 |
| Door switch / buzzer | GPIO13 / GPIO4 |

Board J9 goes to the same named terminals on the DM542H: PUL+, PUL−, DIR+,
DIR−, ENA+, ENA−. The board supplies +5 V to the positive terminals and
its ULN2003A sinks the negative terminals. Do **not** connect the negative
terminals directly to ESP32 GPIOs. Verify the actual wiring, driver input
current and ENA polarity against the specific driver documentation.

The reported machine mapping is DIR HIGH toward HOME, and the limit switches
are normally open to ground. The experimental timer stops motion at the target
switch or after 15 seconds. The 15-second timeout is provisional and must be
calibrated against the real mechanism. A broken normally-open switch wire is
indistinguishable from an open switch; independent hardware protection is
required. The timer, input polarities, movement and mechanical safety are not
validated on the physical board.

Profile day resets after reboot, and heater control lacks production-grade independent overtemperature and
watchdog protection. Do not connect heaters or humidifiers.

MQTT discovery exposes `Operating Mode` with OFF, MANUAL and AUTO. The device
always starts in OFF. MANUAL accepts individual output commands; the turner ON
command requests a single trip to a limit. OFF stops all outputs. AUTO requests
automatic temperature, humidity, turning and circulation fan, but is rejected
while either REV3 safety lock is active. The default build keeps both locks
active; it cannot operate the heater, humidifier or motor.

## Configure and build

1. Copy `include/app_config.local.h.example` to `include/app_config.local.h`
   and enter Wi-Fi and MQTT settings locally. Do not commit the file.
2. Open this directory (the one containing `platformio.ini`) in VS Code with
   PlatformIO, then run `pio run -e esp32_upesy`.
3. If the build succeeds, `pio run -e esp32_upesy -t upload` uploads to the
   connected ESP32. Use `pio device monitor -b 115200` for logs.

This only builds experimental firmware; it does not validate hardware. Motor
and thermal outputs remain disabled in the default configuration.
