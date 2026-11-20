<!-- @req PLAT-POWER-007 -->
# Bench test PLAT-POWER-007 — sensor bus pull-ups go to the switched sensor rail

Requirement: [PLAT-POWER-007](../../requirements/platform.md#plat-power-007). Manual test, run before each release, at step **B4** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- The board under test, unpowered, a multimeter (Ω range).

## Setup
1. No battery, no sensor fitted.

## Steps
1. Measure the resistance from SCL, SDA and the 1-Wire line (sensor connector side) to SENSOR_VCC, then to VCC.

## Pass criteria
- About 4.7 kΩ to SENSOR_VCC on each line on v3.0 (R3, R2, R6); no direct path to VCC (much higher than to SENSOR_VCC).
- v2.2: SCL and SDA pulled to SENSOR_VCC (note the values).

## Results
| Date | Release (tag / commit) | Board | SCL / SDA / 1-Wire to SENSOR_VCC | To VCC | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
