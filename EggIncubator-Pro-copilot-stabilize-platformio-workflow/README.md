# EggIncubator Pro (Safety-Oriented Firmware)

## Important status
This firmware now focuses on safe local control first (AUTO keeps running without Wi‑Fi/MQTT), but it is **not declared production-ready** until hardware calibration and 48–72h dry-run validation are completed.

## Project directory
PlatformIO project is in:
`EggIncubator-Pro-copilot-stabilize-platformio-workflow/`

Run commands from that nested directory.

## Wiring (single source of truth)
From `include/pins.h`:

- I2C SHT31: SDA GPIO21, SCL GPIO22
- DS18B20 (1-Wire): **GPIO16** (dedicated, not shared with I2C or buzzer)
- Heater SSR: GPIO25
- Humidifier relay: GPIO26
- Circulation fan relay: GPIO27
- Ventilation fan relay: GPIO14
- BTS7960: RPWM GPIO32, LPWM GPIO33, ENABLE GPIO23
- Limit HOME GPIO18, Limit END GPIO19
- Door switch GPIO13 (INPUT_PULLUP, active-low)
- Water-low switch GPIO17 (INPUT_PULLUP, active-low)
- Buzzer GPIO4

Relay logic is centralized:
- `RELAY_ON = LOW`
- `RELAY_OFF = HIGH`

## Safety behavior
- Boot: outputs forced safe OFF/STOP before control loop.
- Critical fault policy (`ALARM`): heater OFF, humidifier OFF, motor STOP, buzzer ON.
- Invalid/stale primary temperature or over-temp: heater OFF immediately.
- Invalid/stale humidity: humidifier OFF.
- Door open: turner motor stop and turn command blocked.
- Turner timeout / limit inconsistency: fault raised and motor forced stop.

## Modes
- `OFF`
- `MANUAL`
- `AUTO`
- `ALARM`

MQTT commands are requests only; safety layer can reject/override unsafe actions.

## Persistence (NVS/Preferences)
Stored and restored:
- mode
- incubation running
- turning enabled
- targets (temperature/humidity)
- over-temperature limit
- profile code
- incubation progress (elapsed seconds/day)

## MQTT topic layout (root: `eggincubator/...`)
State/telemetry examples:
- `status`
- `mode/state`, `start/state`
- `temperature/air`, `temperature/egg`
- `humidity/air`
- `target/temperature/state`, `target/humidity/state`
- `relay/heater/state`, `relay/humidifier/state`, `relay/fan/state`, `relay/vent/state`
- `turner/state`, `turner/enabled`
- `fault/state`, `alarm/state`
- `incubation/day`, `uptime`, `rssi`

Commands:
- `mode/set`
- `start/set`
- `target/temperature/set`
- `target/humidity/set`
- `profile/set`
- `relay/*/set` (manual mode only)
- `turner/set`, `turner/command`

## Home Assistant
MQTT Discovery publishes sensors for:
- air/egg temperature
- humidity
- incubation day
- mode
- turner state
- fault state

## Configuration and secrets
Tracked file `include/app_config.h` contains placeholders only.

1. Copy `include/app_config.example.h` to `include/app_config.local.h`
2. Fill real credentials locally
3. `include/app_config.local.h` is gitignored

If previous credentials were real, rotate them.

## Build & test
```bash
~/.local/bin/pio run -e esp32_upesy
~/.local/bin/pio test -e native
```

CI workflow (`.github/workflows/platformio-ci.yml`) runs the same build + native tests.

## Electrical safety caveats
- Use proper SSR/contactor and thermal fuse for heater path.
- Keep AC mains isolated from ESP32 low-voltage side.
- Add independent over-temperature cutoff hardware.
- Use proper fusing and grounding.

## Pilot checklist (before real eggs)
- [ ] 48–72h empty-incubator dry run
- [ ] Validate DS18B20 vs reference thermometer offset
- [ ] Validate SHT31 humidity offset and response
- [ ] Confirm relay polarity on real hardware (no unintended ON at reboot)
- [ ] Test sensor unplug/stale behavior (heater/humidifier stop)
- [ ] Test motor timeout and door-open interlock
- [ ] Test power-cycle restore of mode/day/progress
- [ ] Test Wi‑Fi/MQTT outage while AUTO continues local control
- [ ] Verify alarm buzzer and fault topics in Home Assistant

## TLS note
MQTT currently assumes LAN-only usage (no TLS in this firmware by default). Architecture/topic model is compatible with future TLS enablement.
