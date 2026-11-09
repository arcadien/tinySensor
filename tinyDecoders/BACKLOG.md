# tinyDecoders backlog

Ideas for later. None is a validated requirement yet: each goes through the RBD workflow (`/rbd`) before any code.

## RFLink-format UART output

**Goal:** the bench decoder writes one text line per decoded frame in RFLink format, so Home Assistant's `rflink` integration (or any RFLink consumer) can read tinySensors from it.

Example line (to be checked against the RFLink sources and the HA parser, see below):

```
20;0A;Oregon TempHygro;ID=1D21;TEMP=00ed;HUM=45;BAT=OK;
```

- `20` is the fixed prefix; `0A` is a two-digit hex counter, incremented on every line.
- `TEMP` is hex tenths of °C with bit 0x8000 for negative values; `HUM` is decimal; `BARO` is hex.

**To check first:**
- Copy the exact device names (`Oregon Temp`, `Oregon TempHygro`, `Oregon BTHR`…), the way `ID` is built, and the X10 and Lacrosse WS7000 formats from the RFLink source and from HA's `rflink` parser. Whether HA recognises the sensors depends on this.
- Best test reference: a few lines of real RFLink output captured from the tinySensors.

**Constraints:**
- **No hardware UART.** The ATtiny84A has none, and USI is used by I2C (OLED). Transmit in software.
- **Do not block.** A line is about 50 characters, about 9 ms at 57600 baud. A blocking send would overflow the 16-pulse capture buffer (PLAT-DECODE-001). Use Timer0, one bit per interrupt, sent from a line buffer.
- **Repo `SoftSerial` does not fit.** It is fixed at 9600 baud, blocks, and goes through the tinySensor `Hal`, not `ReceiverHal`.
- **Free pin:** PA7 is capture. PA0–PA3, PA5 and PB0–PB2 are free.
- **Clock accuracy.** The internal RC is only factory-calibrated to a few %, and a UART needs about 2 %. Add a per-chip `OSCCAL` build flag, like `INTERNAL_1v1` on the sensors.
- **Budget.** Static RAM is 323 / 384 B, so about 61 B are left for a line buffer of about 50 B. Flash is 6314 / 8192 B, and the formatter is estimated at about 1 KB.

**Proposed RBD split:**
1. RFLink line formatter (pure logic, loopback-tested) for X10, Lacrosse and Oregon.
2. Non-blocking software UART TX on Timer0, with its pin.
3. `OSCCAL` calibration build flag.
4. `Receiver` sends one line per decoded frame.
