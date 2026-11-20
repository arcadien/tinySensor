# Bench test PLAT-POWER-003 — pins and rails during hibernate

Requirement: [PLAT-POWER-003](../../requirements/platform.md#plat-power-003). Manual test, run at bring-up step **B7** ([bring-up.md](bring-up.md)); add a line to *Results* every time it is run.

## Equipment
- The board under test asleep, a multimeter (V range).

## Setup
1. As [PLAT-POWER-001](PLAT-POWER-001.md), between two transmissions.

## Steps
1. Measure against GND: SENSOR_VCC (PA2), the radio DATA pin (PB0), SDA, SCL, the 1-Wire line.
2. Check the radio stays silent between transmissions (receiver: no stray frame, or a 433 MHz receiver output on a scope).

## Pass criteria
- Step 1: all below 0.1 V.
- Step 2: no transmission outside the cycles.

## Results
| Date | Commit / build | Board | PA2 / PB0 / SDA / SCL / 1-Wire | Radio silent | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
