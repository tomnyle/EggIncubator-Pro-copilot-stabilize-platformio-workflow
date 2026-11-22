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

Profile day resets after reboot, and heater control lacks production-grade
independent overtemperature and watchdog protection. Do not connect heaters
or humidifiers.

## Safe test modes

The firmware starts in **MANUAL**, with all simulated requests off. In the
serial monitor at 115200 baud, send uppercase commands terminated by Enter:

* `MODE MANUAL`: use `SET HEATER ON`, `SET HUMIDIFIER ON`, `SET FAN ON`,
  `SET VENT ON` or `SET TURNER ON`; replace `ON` with `OFF` to clear a request.
* `MODE AUTO`: run the temperature and humidity control logic with real SHT31
  readings.
* `MODE DRYRUN`: run that same logic using simulated inputs, for example
  `INPUT 35.0 45.0` (temperature in °C, relative humidity in %). No control
  request is made until a valid pair has been entered. Allowed ranges are
  -40..125 °C and 0..100%.

Switching modes clears all simulated requests and dry-run inputs. Status and
`[SIMULATION]` messages distinguish requests from real outputs. The manual
turner request is only a displayed state: it does not simulate movement or
limits. Auto does not schedule simulated turns or control fans. MQTT output
commands are disabled in this build; the MQTT relay states report the actual,
locked-off outputs. Physical relay and motor outputs remain locked off in all
three modes even if the `REV3_ALLOW_*` flags are set. These modes do not make
the firmware production-ready.

## Configure and build

1. Copy `include/app_config.local.h.example` to `include/app_config.local.h`
   and enter Wi-Fi and MQTT settings locally. Do not commit the file.
2. Open this directory (the one containing `platformio.ini`) in VS Code with
   PlatformIO, then run `pio run -e esp32_upesy`.
3. If the build succeeds, `pio run -e esp32_upesy -t upload` uploads to the
   connected ESP32. Use `pio device monitor -b 115200` for logs.

This only builds experimental firmware; it does not validate hardware. Motor
and thermal outputs remain disabled in the default configuration.
