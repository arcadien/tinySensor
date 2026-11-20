<!-- @req PLAT-DECODE-004 -->
# Bench test PLAT-DECODE-004 — receiver drives the SSD1306 OLED

Requirement: [PLAT-DECODE-004](../../requirements/platform.md#plat-decode-004). Manual test, run before each release, at step **A3** ([bring-up.md](bring-up.md)); add a line to *Results* for each release.

## Equipment
- The receiver of [PLAT-DECODE-003](PLAT-DECODE-003.md), powered over USB.
- At least two sources (an Oregon and a Lacrosse or X10 sensor).

## Setup
1. Receiver flashed, OLED wired directly at 5 V (no level shifter).

## Steps
1. Power on: note what the screen shows before any frame.
2. Let both sources send; watch the pages for 1 minute.
3. While a page changes, check the RFLink lines keep coming on the serial port.

## Pass criteria
- Step 1: `Waiting for data` on the middle row, no garbage elsewhere.
- Step 2: one page per source every 10 s, big digits doubled cleanly, side column readable ([UI-DECODE-001](../../requirements/ui.md#ui-decode-001) to 003).
- Step 3: no gap in the lines while the screen redraws.

## Results
| Date | Release (tag / commit) | Board | Waiting page | Pages OK | Lines during redraw | Pass | By | Notes |
|---|---|---|---|---|---|---|---|---|
