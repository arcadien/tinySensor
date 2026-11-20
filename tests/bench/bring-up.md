# Release bench tests

Run by hand before each release, in this order, with the release candidate flashed on every board: a release is tagged only when every sheet has a passing line for it. A line ending in → links the bench test sheet that holds the procedure, the pass criteria and the results; the other lines are checks with no requirement behind them. Each sheet's *Results* table gets one line per release (tag, or commit of the candidate).

**A. Receiver** (needed to read the sensor boards)
1. Wiring checked against `hardware/receiver/WIRING.md`, receiver flashed.
2. Pulse capture and time base → [PLAT-DECODE-003](PLAT-DECODE-003.md)
3. OLED → [PLAT-DECODE-004](PLAT-DECODE-004.md)
4. RFLink serial lines → [PLAT-DECODE-005](PLAT-DECODE-005.md)

**B. Sensor board** (v3.0; the mushroom's v2.2 board after a reflash)
1. Before power: the continuity checks at the end of `hardware/tinysensor/WIRING.md`; JP6 bridged 1–2 for a BME280 in I2C.
2. Flash with the BME280 fitted (v3.0 only) → [PLAT-ISP-001](PLAT-ISP-001.md)
3. Awake pin states, BME280 on the external pull-ups alone → [PLAT-POWER-008](PLAT-POWER-008.md)
4. Bus pull-ups to SENSOR_VCC → [PLAT-POWER-007](PLAT-POWER-007.md)
5. BME280 readings → [FUNC-SENSOR-001](FUNC-SENSOR-001.md)
6. Hibernate length and sleep current → [PLAT-POWER-001](PLAT-POWER-001.md)
7. Pins during hibernate → [PLAT-POWER-003](PLAT-POWER-003.md)
8. Radio timings at 1 MHz → [PLAT-POWER-005](PLAT-POWER-005.md)
9. LM35 variant only: switched 5 V → [PLAT-POWER-006](PLAT-POWER-006.md)

For v2.0: every sheet on a v3.0 board, and B3 on the reflashed mushroom.
