<!-- @req PLAT-POWER-006 -->
# Bench test PLAT-POWER-006 — switched 5 V module on PB2 (LM35 variant only)

Requirement: [PLAT-POWER-006](../../requirements/platform.md#plat-power-006). Manual test, run at bring-up step **B9** ([bring-up.md](bring-up.md)); add a line to *Results* every time it is run.

## Equipment
- A board built per `hardware/tinysensor/VARIANTE_LM35.md`, with `USE_SWITCHED_5V`; a multimeter.

## Setup
1. 5 V module and PNP on J8, LM35 on J7.

## Steps
1. Measure the 5 V module output during a cycle and between cycles.
2. Measure PB2 against GND between cycles.

## Pass criteria
- Step 1: 5 V present only during the measurement window, 0 V between cycles.
- Step 2: PB2 in high impedance (not driven), the PNP off.

## Results
| Date | Commit / build | Board | 5 V in / out of window | PB2 asleep | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
