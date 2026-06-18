# PlatformIO environments

Each physical prototype board has `S_XX` written on its PCB. That label is the PlatformIO environment name. Every board gets its own environment because the ATtiny84a's internal 1.1 V reference (`INTERNAL_1v1`) varies chip-to-chip and must be calibrated individually — this is the only reason two otherwise identical boards need separate environments.

## How to calibrate `INTERNAL_1v1` for a new board

1. Power the board from a known stable voltage (e.g. bench supply or fresh alkaline).
2. Measure VCC accurately with a multimeter.
3. Flash any build and read the `Vbatt` value logged over serial (or from Domoticz).
4. Adjust `INTERNAL_1v1` until the reported value matches the measured VCC.
5. Reflash and verify.

## Physical boards

| Env | Protocol | Sensor | Battery | Notes |
|-----|----------|--------|---------|-------|
| `S_02` | Oregon v3, channel 1, rolling code `0x20` | BMP280 (temp + pressure) | 2× AA via charge pump | Prototype rev 2 |
| `S_03` | Lacrosse WS7000, address THREE | BMP280 (temp + pressure) | 2× AA via charge pump | Prototype rev 3 |
| `S_04` | Oregon v3, channel 1, rolling code `0x04` | BME280 (temp + humidity + pressure) | 2× AA | Prototype rev 4 |
| `S_05` | Oregon v3, channel 2, mode 2 | BME280 (temp + humidity + pressure) | 18650 Li-ion | Prototype rev 5 — older HW design, higher `INTERNAL_1v1=1343` reflects this chip |

## Special environments

| Env | Purpose |
|-----|---------|
| `robot` | A robot device reworked to share the TinySensor PCB interface. Lacrosse WS7000 address THREE, BME280. |
| `devmodule` | Bench development board. `BATTERY_IS_VCC` — no battery divider, reports VCC directly. Lacrosse WS7000 address ONE, BME280. |
| `SOLAR_TEST` | CI simulation target via `simavr`. Lacrosse WS7000 address TWO, BME280 + BH1750 light sensor. Not a physical board — used to run on-device tests in the AVR simulator. |

## Battery voltage thresholds

`LOW_BATTERY_VOLTAGE` is set per environment to match the chemistry:

| Chemistry | Typical threshold |
|-----------|------------------|
| 2× AA alkaline (via charge pump) | 2200 mV |
| 2× AA alkaline (direct) | 2200 mV |
| 18650 Li-ion | 3400 mV |
| USB / bench (`BATTERY_IS_VCC`) | 3300 mV (informational) |

When the measured battery voltage falls below the threshold, the X10 battery meter frame is suppressed and `SetBatteryLow(true)` is sent in the Oregon frame.

## Adding a new board

1. Copy an existing `[env:S_XX]` block in `platformio.ini`.
2. Choose the protocol (`USE_OREGON` / `USE_LACROSSE`) and sensor (`USE_BMP280` / `USE_BME280` / `USE_BH1750` / `USE_DS18B20`).
3. Set `BATTERY_VOLTAGE_X10_ID` to a unique integer not used by any other board.
4. Set `INTERNAL_1v1=1100` as a starting value, then calibrate as above.
5. Assign the next `S_XX` label, write it on the PCB.
