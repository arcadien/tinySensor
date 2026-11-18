# tinySensor board: bill of materials (v3.0)

Generated from the v3.0 schematic (`tinysensor.kicad_sch`, rev 3.0), grouped by value and footprint. No part is marked DNP: which connectors and modules to fit depends on the sensor unit, see [`WIRING.md`](WIRING.md) and [`docs/environments.md`](../../docs/environments.md). v2.2 boards: see [`REWORK_v2.2.md`](REWORK_v2.2.md).

Regenerate after a schematic change:

```bash
kicad-cli sch export bom --fields 'Reference,Value,Footprint,${QUANTITY}' \
  --labels 'Refs,Value,Footprint,Qty' --group-by 'Value,Footprint' --ref-range-delimiter '' \
  -o bom.csv tinysensor.kicad_sch
```

## Components

| Refs | Qty | Value | Package |
|---|:-:|---|---|
| U1 | 1 | ATtiny84A-PU | DIP-14 |
| Q1 | 1 | AO3401A (P-MOSFET) | SOT-23 |
| D1 | 1 | LED | 5 mm THT |
| D2, D3 | 2 | 1N914 | DO-35 |
| C1 | 1 | electrolytic, value per supply: 10 µF (2×AA/AAA, or 18650 through the TPS63020), 220 µF **low-leakage** (CR2032) | radial Ø6.3, 2.5 mm pitch |
| C3 | 1 | 1 µF electrolytic | radial Ø4, 1.5 mm pitch |
| C2 | 1 | 100 nF film (global decoupling) | 7 × 2 mm, 5 mm pitch |
| C7 | 1 | 100 nF film (PA1 battery divider; no tantalum: its leakage would bias the 500 kΩ divider) | 7 × 2 mm, 5 mm pitch |
| C4 | 1 | 100 nF ceramic (radio, next to J1) | disc Ø3, 2.5 mm pitch |
| C6 | 1 | 100 nF ceramic | disc Ø3, 2.5 mm pitch |
| C5 | 1 | 1 nF ceramic | disc Ø3, 2.5 mm pitch |
| R1 | 1 | 1 kΩ | axial DIN0204, vertical, 1.9 mm pitch |
| R2, R3, R6 | 3 | 4.7 kΩ | axial DIN0204, 7.62 mm pitch |
| R4 | 1 | 75 Ω | axial DIN0204, 7.62 mm pitch |
| R5 | 1 | 10 kΩ (RESET pull-up) | axial DIN0204, vertical, 1.9 mm pitch |
| R7, R8 | 2 | 1 MΩ (PA1 battery divider) | axial DIN0204, vertical, 1.9 mm pitch |
| R9, R10 | 2 | 330 Ω (SCL / SDA series, ISP isolation) | axial DIN0204, vertical, 1.9 mm pitch |
| SW1 | 1 | RESET push button | 6 mm THT, H 4.3 mm |

## Connectors and holders

| Refs | Qty | Function | Part |
|---|:-:|---|---|
| J1 | 1 | RADIO (433 MHz emitter) | 1×3 pin header, 2.54 mm |
| J2 | 1 | 3V3 module (TPS63020) | 1×3 pin header, 2.54 mm |
| J5 | 1 | OneWire sensor | 1×3 pin header, 2.54 mm |
| J7 | 1 | Analog sensor | 1×3 pin header, 2.54 mm |
| J4 | 1 | Digital out | 1×2 pin header, 2.54 mm |
| J8 | 1 | EXT. INT0 | 1×2 pin header, 2.54 mm |
| JP1 | 1 | LED jumper | 1×2 pin header, 2.54 mm + jumper cap |
| J3 | 1 | I2C 1 (BME280) | JST-EH 6-pin vertical, B6B-EH-A |
| J14 | 1 | I2C 2 | JST-EH 6-pin vertical, B6B-EH-A |
| J6 | 1 | ISP | 2×3 mini ICSP socket (project library) |
| J13 | 1 | RAW IN | JST-XH 2-pin vertical, B2B-XH-A |
| J10 | 1 | CR2032 socket | Keystone 1060 |

## No part to buy

| Refs | Function |
|---|---|
| JP2 | DIRECT Vcc solder jumper (2 pads, open) |
| JP3 | solder jumper (2 pads, open) |
| JP4 | Analog ref solder jumper (3 pads, open) |
| JP6 | BME280 SDO solder jumper (3 pads, open: bridge 1–2 for I2C at 0x76, the firmware address, or 2–3 for SPI; choose at assembly) |
| MH1–MH3 | M3 mounting holes |

## Off-board modules (not on the schematic)

Fitted per sensor unit, wiring in [`WIRING.md`](WIRING.md): 433 MHz OOK emitter on J1, BME280 or BMP280 breakout on J3, TPS63020 3.3 V buck-boost module on J2, DS18B20 on J5, LM35 variant on J7 ([`VARIANTE_LM35.md`](VARIANTE_LM35.md)), CR2032 in J10, or 2×AA/AAA or an 18650 (through a TP4056) on J13: the board has no 18650 holder.
