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
limits. Auto does not schedule simulated turns or control fans. Physical relay
and motor outputs remain locked off in all three modes even if the
`REV3_ALLOW_*` flags are set. These modes do not make the firmware
production-ready.

With Wi-Fi, MQTT and Home Assistant MQTT discovery configured, select
**MANUAL**, **AUTO** or **DRYRUN** from the Simulation Mode Control entity.
Five separate **Simulated** switches control virtual heater, humidifier, fans
and turner requests in MANUAL. They become unavailable in AUTO or DRYRUN. In
DRYRUN, enter temperature and humidity in the two Dry-run Input entities; both
values are required before the controller simulates a response. AUTO uses the
real SHT31 readings but still only simulates control outputs. The Simulation
Mode and Simulated Heater Safety Inhibit entities show the current mode and
interlock; an inhibited heater request is rejected. Existing output switches
are labelled **locked off** and reject commands. Their OFF state is the
software-commanded output, not proof of actual relay or load feedback. Do not
retain command messages on the MQTT broker.

The experimental software heater interlock starts inhibited. It blocks and
clears the simulated heater request at 39.5 °C or on an invalid SHT31 reading,
and automatically clears only after a valid reading at or below 38.5 °C. In
DRYRUN it uses the simulated inputs; in AUTO and MANUAL it uses the actual
sensor. An inhibited manual request must be issued again after recovery.
These provisional limits are not validated safety thresholds, and this
software interlock does not replace an independent hardware cutoff.

## Configure and build

1. Copy `include/app_config.local.h.example` to `include/app_config.local.h`
   and enter Wi-Fi and MQTT settings locally. Do not commit the file.
2. Open this directory (the one containing `platformio.ini`) in VS Code with
   PlatformIO, then run `pio run -e esp32_upesy`.
3. If the build succeeds, `pio run -e esp32_upesy -t upload` uploads to the
   connected ESP32. Use `pio device monitor -b 115200` for logs.

This only builds experimental firmware; it does not validate hardware. Motor
and thermal outputs remain disabled in the default configuration.

## Select an incubation profile

Select `CHICKEN`, `QUAIL`, `DUCK` or `GOOSE` using the Home Assistant
**Incubation Profile** entity, or send e.g. `PROFILE DUCK` in the serial
monitor. Profile names are case-insensitive. The other serial commands above
are uppercase. The MQTT command topic is `eggincubator/profile/set` and the
state topic is `eggincubator/profile/state`.

Changing profile restarts the batch at day 1; selecting the current profile
does not. Reboot resets the selection to CHICKEN and the day to 1. The recipes,
setpoints and schedules are experimental and must be verified for the eggs
and equipment. Do not use this firmware for incubation.
