<!-- @req PLAT-POWER-001 -->
# Bench test PLAT-POWER-001 — hibernate length and sleep current

Requirement: [PLAT-POWER-001](../../requirements/platform.md#plat-power-001). Manual test, run before each release, at step **B6** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- The board under test, a multimeter in series in the J13 + wire (µA range), the receiver.

## Setup
1. Board flashed with its environment; note `SLEEP_TIME_IN_SECONDS` (`docs/environments.md`).

## Steps
1. Time 5 consecutive transmissions on the receiver.
2. Between two transmissions, read the current once it has settled.

## Pass criteria
- Step 1: the interval is 8 × ⌊s / 8⌋ s (at least 8 s) plus the cycle time, within the watchdog tolerance (± 10 %).
- Step 2: below 10 µA at 3 V with the sensors off (ATtiny84A power-down with watchdog and BOD off: about 5 µA; BOD on would add about 20 µA).

## Results
| Date | Release (tag / commit) | Board | Interval | Sleep current | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
