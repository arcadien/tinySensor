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
| `S_02` | Oregon, channel 1, rolling code `0x20` | BME280 (temp + humidity + pressure) | 2× AA via charge pump | Prototype rev 2 |
| `S_03` | Lacrosse WS7000, address THREE | BME280 (temp + humidity + pressure) | 2× AA via charge pump | Prototype rev 3 |
| `S_04` | Oregon, channel 3, rolling code `0x04` | BME280 (temp + humidity + pressure) | 2× AA | Prototype rev 4 |
| `S_05` | none: X10 battery meter only | BME280 (read, not transmitted) | 18650 Li-ion | Prototype rev 5 — older HW design, higher `INTERNAL_1v1=1343` reflects this chip |
| `mushroom` | Oregon, channel 3, rolling code `0x3C` | BME280 (temp + humidity, no pressure: THGR228N frame) | 18650 Li-ion via TP4056, 3.3 V buck-boost, PA1 divider /2 | Cut-down v2.2 in the mushroom enclosure; X10 battery meter 7; `INTERNAL_1v1=1104` calibrated on the battery reading |

## Special environments

| Env | Purpose |
|-----|---------|
| `robot` | A robot device reworked to share the TinySensor PCB interface. Lacrosse WS7000 address FOUR, BME280. |
| `devmodule` | Bench development board. `BATTERY_IS_VCC` — no battery divider, reports VCC directly. No radio protocol: BME280 read, only the X10 battery meter is sent. |
| `SOLAR_TEST` | Solar test board: Lacrosse WS7000 address TWO, BH1750 light sensor, analog X10 meter (`ANALOG1_X10_ID=9`), no temperature sensor. Has a `simavr` test setup, but CI only builds it. |

## Battery voltage thresholds

`LOW_BATTERY_VOLTAGE` (mV) is set per environment, from the battery and the way it is measured:

| Env | Battery | Threshold |
|-----|---------|-----------|
| `S_02` | 2× AA via charge pump | 2200 mV |
| `S_03` | 2× AA via charge pump | 1100 mV |
| `S_04`, `robot` | 2× AA | 2200 mV |
| `S_05` | 18650 Li-ion | 3400 mV |
| `mushroom` | 18650 Li-ion via TP4056, 3.3 V buck-boost | 3000 mV |
| `devmodule`, `SOLAR_TEST` | bench / solar (`BATTERY_IS_VCC` on `devmodule`) | 3300 mV |

When the measured battery voltage falls below the threshold, the X10 battery and analog meter frames and the BH1750 luminosity frames are suppressed, and `SetBatteryLow(true)` is sent in the Oregon frame.

## Adding a new board

1. Copy an existing `[env:S_XX]` block in `emitter/platformio.ini`.
2. Choose the protocol (`USE_OREGON` / `USE_LACROSSE`) and sensor (`USE_BMP280` / `USE_BME280` / `USE_BH1750` / `USE_DS18B20`).
   `USE_BH1750` needs `LACROSSE_ID` whatever the protocol (the luminosity always goes out as Lacrosse frames). `USE_SWITCHED_5V` powers an off-board 5 V module through PB2 during measurements (LM35 variant, PLAT-POWER-006).
3. Set `BATTERY_VOLTAGE_X10_ID` to a unique integer not used by any other board.
4. Set `INTERNAL_1v1=1100` as a starting value, then calibrate as above.
5. Assign the next `S_XX` label, write it on the PCB.

## tinyDecoders receiver (`tinyDecoders/platformio.ini`)

The 433 MHz receiver is a separate PlatformIO project (`pio run -d tinyDecoders -e <env>`). It needs no per-board calibration.

| Env | Board | Purpose |
|-----|-------|---------|
| `promini` | Arduino Pro Mini 5 V / 16 MHz (ATmega328P) | Alternative receiver build, a standalone OLED viewer: RXB6 DATA on D8 (ICP1/PB0), SSD1306 on A4 (SDA) / A5 (SCL), debug UART TX on D1 at 57600 baud. Uploaded with the `arduino` protocol through a USB-serial adapter on the programming header (CONF-DECODE-004) |
| `promicro` | SparkFun Pro Micro 5 V / 16 MHz (ATmega32U4) | Production receiver, a standalone OLED viewer: RXB6 on pin 4 (ICP1/PD4), SSD1306 on D2 / D3, RFLink lines on USB CDC; powered and flashed over its micro-USB (avr109), no USB-serial adapter (CONF-DECODE-003) |
| `native` | host | Unity tests of the decoders, `Receiver` and the formatters against the repository encoders |

Both boards share `AtmegaReceiverHal` and `src/main.cpp`.
