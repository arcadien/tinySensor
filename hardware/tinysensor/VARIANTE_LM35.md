# LM35 variant: analog sensor powered at 5 V only while measuring

Valid for v2.2 and v3.0. The ATtiny runs straight from 2 AA or AAA cells. The LM35 (4 V minimum) is powered by a 5 V boost module without an EN pin. That module is powered only while measuring, through a BC327 PNP driven by PB2 (J8 "EXT. INT0"): output low while measuring, high-impedance input the rest of the time (build option `USE_SWITCHED_5V`).

## Board supply

- 2×AA or 2×AAA cells on J13 (terminal 1 = +, square; terminal 2 = GND).
- **JP2 "DIRECT Vcc" closed**, nothing on J2: VCC = cell voltage (2.0–3.2 V). The ATtiny84A at 1 MHz runs from 1.8 V.
- No BME280 (3.6 V max, but mostly useless here). Radio range drops below 3 V.

## Switching the 5 V module (point-to-point wiring)

```
+BATT ──┬────────── E  BC327 (PNP)
       10k             C ──── 5 V module VIN
        │              B
        └──── 1k ──────┤
                       │
              J8 pin 1 (PB2)
```

| Part | Role |
|---|---|
| BC327 | switch on the +BATT side of the 5 V module |
| 10 kΩ base–emitter | keeps the BC327 off while PB2 is high impedance (idle, sleep, start-up) |
| 1 kΩ to PB2 | ≈ 2 mA of base current while measuring: the BC327 passes ≈ 40 mA, the module starts a bit slower, no consequence |

PB2 is never driven high. +BATT must not exceed VCC, otherwise a leakage current flows through the PB2 clamp diode: guaranteed with JP2 closed (VCC = cells).

TO-92 pinout, flat face towards you, legs down, left to right:
- BC327: C, B, E;
- LM35: +Vs, Vout, GND.

Check it on the maker's datasheet.

## LM35

| LM35 | Wired to |
|---|---|
| +Vs | 5 V module VOUT |
| Vout | J7 pin 2 (analog input, PA0 through R4 75 Ω) |
| GND | board GND (J2 pin 2, or J7 pin 3 if JP4 is soldered 2-3) |

5 V module GND: board GND.

Why switch +BATT and not ground: with the ground switched, the LM35 ground would be switched too. The transistor saturation voltage (10–30 mV) would add to the reading, i.e. 1 to 3 °C of error. The LM35 must stay on the true ground. But the module passes VIN to VOUT when its ground is cut, and the LM35 would then stay powered during sleep.

## Firmware

- Build with `BATTERY_IS_VCC`, `ANALOG1_X10_ID` and `USE_SWITCHED_5V`.
- The firmware sends mV: the temperature is **mV ÷ 10** (°C), to convert on the receiving side.
- At least 30 ms elapse between sensor power-on and the analog reading. Enough for the module to start (a few ms) and for the LM35 to settle: to confirm with a scope on VOUT.
- The reading is relative to VCC, corrected by the measured VCC: resolution is about 0.3 °C at 3 V. Doing better needs the internal 1.1 V reference. That is a firmware change, through an RBD requirement.

## Checks

1. Without LM35, cells in: during sleep, module VIN at 0 V and a total current of a few µA.
2. During a measurement, module VOUT at 5 V. Hard to see with a meter: for the test, tie J8 pin 1 to GND with a wire.
3. With the LM35: compare with a thermometer. A constant 1 to 2 °C error is corrected on the receiving side.
