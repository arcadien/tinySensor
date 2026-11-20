<!-- @req PLAT-POWER-008 -->
# Bench test PLAT-POWER-008 — awake: no internal pull-up on the sensor buses, the BME280 still reads

Requirement: [PLAT-POWER-008](../../requirements/platform.md#plat-power-008). Manual test, run before each release, at step **B3** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- The board under test: a v3.0 board, and the mushroom's v2.2 board once reflashed, each with its BME280.
- A scope (or a multimeter for a rough check), the receiver ([PLAT-DECODE-003](PLAT-DECODE-003.md)).

## Setup
1. Board flashed with the current firmware (PORTA without internal pull-up).

## Steps
1. Let the board run 10 cycles; read the values on the receiver.
2. Scope on SENSOR_VCC and SDA, triggered on SENSOR_VCC falling after the last transmission: watch until the board sleeps.

## Pass criteria
- Step 1: every cycle gives a plausible temperature, humidity and pressure: the external pull-ups alone carry the I2C bus.
- Step 2: SENSOR_VCC and SDA fall to 0 and stay below 0.1 V while PA2 is low (no back-powering through the pull-ups).

## Results
| Date | Release (tag / commit) | Board | 10 cycles read | SENSOR_VCC / SDA, PA2 low | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
