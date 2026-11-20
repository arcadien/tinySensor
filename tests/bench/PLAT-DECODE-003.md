# Bench test PLAT-DECODE-003 — receiver captures the radio pulses and keeps time

Requirement: [PLAT-DECODE-003](../../requirements/platform.md#plat-decode-003). Manual test, run at bring-up step **A2** ([bring-up.md](bring-up.md)); add a line to *Results* every time it is run.

## Equipment
- The receiver (Pro Micro, production; Pro Mini, alternative) with its RXB6 and OLED, wired per `hardware/receiver/WIRING.md`.
- A sensor board in service, or the `devmodule` board, sending frames.
- A PC with `pio device monitor`.

## Setup
1. Flash the receiver (`pio run -d tinyDecoders -e promicro --target upload`, or `-e promini` through a USB-serial adapter).
2. Open `pio device monitor -d tinyDecoders -b 57600`.

## Steps
1. Put the sensor board 1–3 m from the receiver and let it send for 10 minutes.
2. Note the number of RFLink lines and the number of frames the sensor sent (each environment frame is sent twice, [FUNC-MAIN-001](../../requirements/functional.md#func-main-001)).
3. Switch the sensor off and time how long its page stays on the screen: a source leaves after 30 minutes without frames ([FUNC-DECODE-010](../../requirements/functional.md#func-decode-010)), which checks the time base.

## Pass criteria
- Step 2: at least one line per sensor cycle, and no corrupted line.
- Step 3: the source leaves the screen 30 min (± 1 min) after its last frame.

## Results
| Date | Commit / build | Board | Lines / frames sent | Expiry time | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
