<!-- @req PLAT-DECODE-005 -->
# Bench test PLAT-DECODE-005 — receiver writes RFLink lines without blocking

Requirement: [PLAT-DECODE-005](../../requirements/platform.md#plat-decode-005). Manual test, run before each release, at step **A4** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- The receiver of [PLAT-DECODE-003](PLAT-DECODE-003.md); on the Pro Mini, a USB-serial adapter on TX D1.
- A PC with `pio device monitor`.

## Setup
1. `pio device monitor -d tinyDecoders -b 57600`.

## Steps
1. With frames arriving, read the lines.
2. Close the monitor (Pro Micro: leave the USB host without a reader; Pro Mini: unplug the adapter) for 2 minutes, then open it again.
3. Watch the OLED during step 2.

## Pass criteria
- Step 1: each line is `20;…;` and ends with `\r\n`; no startup banner.
- Steps 2–3: the screen keeps updating while nothing reads the serial port; lines resume when the monitor is back.

## Results
| Date | Release (tag / commit) | Board | Line format | Screen without reader | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
