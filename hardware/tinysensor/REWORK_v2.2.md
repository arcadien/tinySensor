# Rework of the already-built v2.2 boards — mushroom build

The v2.2 boards are already manufactured. Version 3.0 fixes the defects at the source (see `BACKLOG.md`).
This sheet says how to make a v2.2 usable in the mushroom. It covers the board cut at x = 147 / y = 87, powered by an 18650 through a TP4056 wired to J13, with the BME280 plugged into the "I2C" footprint.
Coordinates are KiCad's, in mm, board not rotated, seen from the top (component side).

## Must do

### 1. BME280 ground (P1): a strap

The GND pads of the "I2C" footprint (pads 2 and 6) and the ISP GND pin (J6.6) sit on an isolated copper island.
Without a fix, the BME280 has no ground. Its SDO pin, wired to GND, floats too, and the sensor I2C address is no longer fixed.

- The strap: a 7 mm wire between **I2C pad 6** (186.85 ; 77.11) and **I2C_SPI pad 6** (186.85 ; 83.91), just below, on the top side of the board. The I2C_SPI holes are free since its connector is not fitted, and its pad 6 is tied to the ground plane.
- Then, with a meter: beep between I2C pad 2, I2C pad 6, J6 pad 6 and J13 terminal 2.

### 2. LED D1 (R5): fit it against the silkscreen

The footprint is reversed. Put the **anode (long leg) in the square pad** (pad 1, R1 side) and the cathode in the round one (pad 2, GND). No cut needed.

### 3. Battery sense on PA1 (R1): one cut and a divider

PA1 (U1 pin 12) is tied straight to +BATT. With a full 18650 (4.2 V), the pin gets more than VCC: the reading saturates at 1023, and the MCU clamp diode may conduct.

1. **Cut** the 0.3 mm B.Cu track leaving U1 pin 12 (174.00 ; 62.50) diagonally towards (170.19 ; 66.31), 1–2 mm from the pin. Make two cutter strokes and lift the copper between them. Check there is no continuity left between U1.12 and J13 terminal 1.
2. **Scrape** the track solder mask over 2 mm, just after the cut on the +BATT side.
3. **High resistor, 1 MΩ:** from U1.12 to the scraped copper, across the cut, bottom side.
4. **Low resistor, 1 MΩ, and 100 nF:** from U1.12 to U1.14 (GND), bottom side. The legs straddle pin 13 (PA0): sleeve them.
5. The divider draws about 2 µA all the time. The reading is half the battery voltage, and the firmware must multiply it by 2. That setting does not exist yet (firmware backlog F1). Until then, do not enable `BATTERY_VOLTAGE_X10_ID`, or build with `BATTERY_IS_VCC` (which measures VCC only).

### 4. Board cut (P5): do not leave +BATT against ground

The x = 147 and y = 87 lines cut the ground plane (B.Cu) and two +BATT tracks (F.Cu): the first near y ≈ 59.3, the second near x = 148.35.

- Saw slightly outside the line, then file down to the line.
- On both +BATT track ends, remove the last millimetre of copper at the edge with a cutter.
- Lightly chamfer the B.Cu side of the edge with the file to break the ground-plane burr.
- Check: J13 terminal 1 and J13 terminal 2 must **not** beep together.
- Varnish the edges (nail varnish or conformal coating): the board lives outdoors.

### 5. 3.3 V supply: a TPS63020 instead of the J2 module

The module planned for J2 ("0.9–3.3 V to 3.3 V") is a boost. With an 18650 at 4.2 V, these modules usually let the input voltage through (≈ 3.9 V), while the BME280 is rated 3.6 V max.
It is replaced by the **TPS63020** 3.3 V buck-boost module in stock from the Tars project (26.7 × 17.7 × 1.6 mm, too big for the J2 spot). It is wired:

| Wire | Colour | TPS63020 module | Board | Length |
|---|---|---|---|---|
| W7 | red | VIN | J2 pad 1, +BATT (149.53 ; 57.25) | ~40 mm |
| W8 | black | GND | J2 pad 2, GND (152.07 ; 57.25) | ~40 mm |
| W9 | white | VOUT 3.3 V | J2 pad 3, VCC (154.61 ; 57.25) | ~40 mm |

- On the module: **EN tied to VIN**, and **PS/SYNC tied to GND** (power-save mode). Otherwise it stays in PWM and draws a few mA.
- The module lies flat on top of the board, on double-sided foam tape, checking that no solder joint runs under it. There is plenty of room under the cap. The mushroom model does not plan its spot yet: to add.
- Bonus: the MH5 screw is no longer hidden (P7).
- Check, without BME280 or ATtiny, feeding J13 from a bench supply limited to 100 mA:
  - at 3.7 V and at 4.2 V, VCC (C2 pad 2) must be between **3.20 and 3.40 V**;
  - the idle current must be **below 0.1 mA**.
  Also log the result in the Tars test bench `tests/bench/PLAT-POWER-REG-001.md`, which has not been run yet.
- Consumption: the sleeping ATtiny draws a few µA; the module adds about 25–50 µA idle. That still means several years on a 2500 mAh 18650.

## Adapted assembly

- **J2 (P7):** fit nothing in the footprint, only wires W7–W9 (see item 5).
- **C6 (P8):** do not fit. It only serves the analog sensor (J7 / JP4), absent from the mushroom.
- **JP2 "DIRECT Vcc" (R9):** leave it open: otherwise it shorts the TPS63020 input to its output.
- **BME280 CSB (R11):** position 5 is not connected. Check with a meter that the breakout pulls CSB up to VIN (≈ 10 kΩ): otherwise the sensor goes into SPI mode.
- **J13 polarity (R2, P9):** no reverse protection. Follow the colour code W5 red = terminal 1 (+, square pad) and W6 black = terminal 2. Mark "+" with a felt pen next to J13.

## Recommended, cheap

- **P13:** a 100 nF capacitor, bottom side, between J1 pin 1 (GND, 193.00 ; 56.86) and J1 pin 2 (VCC), as close as possible to the radio module.
- **R7:** a 10 kΩ resistor between J6 pin 5 (RESET) and J6 pin 2 (VCC), bottom side. No capacitor, to stay ISP and debugWIRE compatible.

## Firmware

The sleep fix (R3, R4) is on branch `hw/schematic-upgrade`: flash that version.
