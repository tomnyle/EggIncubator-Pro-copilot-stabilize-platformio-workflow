# PROJECT_STATUS

## Current snapshot (safety stabilization branch)

### Completed in code
- Unified pin mapping in `include/pins.h` with dedicated DS18B20 pin (GPIO16), no sharing with I2C/buzzer.
- Centralized relay polarity (`RELAY_ON/RELAY_OFF`) and safe output initialization at boot.
- Added system mode model: `OFF`, `MANUAL`, `AUTO`, `ALARM`.
- Added fault model and critical-fault policy (heater OFF, humidifier OFF, motor STOP, buzzer ON).
- Sensor manager now tracks validity/staleness/timestamps and door/water-low states.
- Temperature disagreement (SHT31 vs DS18B20) fault threshold implemented.
- Temperature control hardened with hysteresis + minimum switch interval + over-temp cutoff.
- Humidity control hardened with hysteresis + max runtime + cooldown interval.
- Egg turner refactored to explicit state machine with limit checks, timeout, and door interlock.
- Added NVS persistence for mode/profile/targets/turning/progress.
- MQTT topics normalized for mode/targets/relay/turner/fault/alarm and command validation.
- Home Assistant discovery payloads expanded for core sensors/states.
- Added native unit tests for safety decision/fault bit/day calc/turner transitions.
- Added GitHub Actions workflow for `pio run` and `pio test`.

### Still requires hardware verification (mandatory)
- Confirm exact active-low/active-high behavior of installed relay modules and BTS7960 wiring.
- Tune hysteresis/timeout/runtime values on real chamber thermal dynamics.
- Validate long-run watchdog behavior and brownout scenarios.
- Validate sensor mounting-induced offsets and calibration drift.
- Validate EMI/noise behavior with real heater/motor loads.

### Production readiness statement
This repository is **not automatically production-ready** from code build/tests alone.
A supervised 48–72h dry run and hardware safety validation are required before incubating real eggs.
