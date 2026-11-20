<!-- @req PLAT-ISP-001 -->
# Bench test PLAT-ISP-001 — a v3.0 board is flashed with its BME280 fitted

Requirement: [PLAT-ISP-001](../../requirements/platform.md#plat-isp-001). Manual test, run before each release, at step **B2** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- A v3.0 board with a BME280 breakout on J3 or J14, JP6 bridged 1–2 (I2C 0x76).
- The ISP programmer on J6, the battery or a 3 V bench supply on J13.
- The receiver ([PLAT-DECODE-003](PLAT-DECODE-003.md)).

## Setup
1. BME280 fitted; programmer cable on J6.

## Steps
1. Flash the board's environment (`pio run -d emitter -e <env> --target upload`), avrdude verifying the flash.
2. Unplug the programmer and let the board run a few cycles.

## Pass criteria
- Step 1: avrdude writes and verifies with no error, sensor still fitted.
- Step 2: the receiver shows a plausible temperature, humidity and pressure (through R9/R10 330 Ω).

## Results
| Date | Release (tag / commit) | Board | Flash verified | Readings | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
