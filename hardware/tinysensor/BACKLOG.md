# Upgrade backlog — tinySensor board

Review of 2026-11-06 on branch `hw/schematic-upgrade`, done with KiCad 10.0.7 (`kicad-cli`): ERC, netlist, DRC with schematic parity, and comparison with the firmware (`lib/hal/Attiny84aHal*`).
Severities: **Critical** / **Major** / **Minor** / **Question** / **Evolution**. Effort: S (≈ 1 h), M (half a day), L (a redesign).

ERC state: 0 errors and 47 warnings. 39 are outdated embedded symbols. Only two point at a real defect, and that defect is R1.
Schematic ↔ PCB parity reports 39 mismatches (see R6).

Full reports, ERC/DRC and images: `review/2026-11-06/`. Rework of the already-built v2.2 boards: `REWORK_v2.2.md`. Connectors and off-board wiring: `WIRING.md`.

## Schematic

| ID | Sev. | Area | Finding | Fix | Effort |
|---|---|---|---|---|---|
| R1 | ✅ v3.0 (Critical (v2.2: rework)) | Power / MCU | PA1 (U1.12, battery sense) is tied straight to +BATT, with no resistor or divider. With a battery above VCC + 0.5 V (Li-ion, 18650, J13 RAW IN above 3.8 V), the clamp diode conducts with no current limit: the MCU can be destroyed, or VCC fed through PA1. The reading also saturates at 1023. ERC: `pin_to_pin` and `multiple_net_names` +BATT/Battery_Checker. | 10–100 k series resistor and 100 nF to GND. If the battery can exceed VCC, a divider, possibly switched by PA2, and adapt `ConvertAnalogValueToMv`. | S |
| R2 | ◐ v3.0 (Major) | Power | No reverse-polarity protection. J10 (CR2032), J11 (AAA), J12 (AA), J13 (RAW IN) and the PCB 18650 holder are all in parallel. A reversed terminal block puts a negative voltage on the DC-DC and the MCU. Two cells fitted together charge each other, which is dangerous for the CR2032. | P-MOSFET ideal diode, or Schottky on RAW IN. Mark the holders as exclusive (DNP, silkscreen note). | M |
| R3 | ✅ Fixed | Power / firmware | In hibernation, `sleep()` makes PA2 a floating input (`DDRA = 0`, `Attiny84aHal.cpp:85`). The same function enables the internal pull-ups of PA4/PA5/PA6 (l.89), and PA3 keeps its own since init (l.42). SENSOR_VCC is then fed through R2/R3/R6: the sensors are never really off, and the BME280 never gets a clean power-on reset. | Firmware: during sleep, PA2 output low, PA3/PA4/PA6 inputs without pull-up. | S |
| R4 | ✅ Fixed | Radio | The radio module is always powered (J1.2 on VCC), and its DATA pin (PB0) floats during sleep (`DDRB = 0`). An OOK emitter can key itself. The comment `Attiny84aHal.h:9` says the opposite of the schematic. | Firmware: PB0 output low during sleep. Option: a 100 k pull-down on TX_RADIO. Fix the comment. | S |
| R5 | ✅ v3.0 (Major) | LED | D1 is fitted backwards (anode on GND, cathode towards R1/PB1), and the PCB footprint matches that mistake. The firmware drives PB1 high to light it, so the LED never lights. | Flip D1 in the schematic and the PCB. | S |
| R6 | ✅ v3.0 (Major) | Sync | The schematic is no longer the reference. Commit c8df224 added I2C and I2C_SPI (6-pin JST-EH), JP5, JP6 and the BH-18650 holder in the PCB only. J3, J11, J12 and MH3 are missing from the PCB, and MH1 is duplicated there. Two unconnected pads remain: JP6 to MISO, and I2C.6 to the GND zone. The 18650 − pole seems to have no net. | Carry these items into the schematic, then run "Update PCB from Schematic" until parity is clean. Check the 18650. | M |
| R7 | ✅ v3.0 (Minor) | MCU | RESET has no external pull-up: only the weak internal one and a button. | 10 k to VCC, no capacitor, to stay debugWIRE compatible. | S |
| R8 | Minor | Power | The global decoupling (2 × 10 µF) is weak for a CR2032, whose internal resistance is 10–20 Ω, against the radio current peak. | 100–470 µF low-leakage for the CR2032 versions. | S |
| R9 | ✅ v3.0 (Minor) | Power | JP2 "DIRECT Vcc" shorts the DC-DC input to its output if J2 is fitted. | Note "JP2 closed ⇒ J2 not fitted". The firmware must then use `BATTERY_IS_VCC`. | S |
| R10 | Question | Power | The J2 DC-DC module has no enable pin. Its quiescent current (often 10–100 µA) probably exceeds the sleeping ATtiny's. | **Decided (2026-11-07):** TPS63020 buck-boost module (Tars stock), wired to the J2 pads as in `REWORK_v2.2.md`. ◐ v3.0: J2 becomes a 1×03 header (1 +BATT, 2 GND, 3 VCC) on the same pads; v3.0 choice (2026-11-07): 2 AA or AAA cells on J13 and a 3.3 V boost module (in stock) on J2. A boost does not regulate if Vin > 3.3 V: with an 18650 (BT1), the TPS63020 buck-boost is needed. Without BME280 (DS18B20 on J5 or 3-pin analog sensor on J7, both up to 5.5 V), an 18650 with a boost is fine. v3.0 silkscreen: "18650: TPS63020, or boost without BME280". Measure its idle current (< 0.1 mA). | M |
| R11 | ✅ v3.0 (Question) | I2C | The BME280 CSB pin (J3.5) is floating; if seen low, the sensor goes into SPI. SDO is on GND, hence address 0x76, matching the firmware. | Tie CSB to SENSOR_VCC, or check that the breakout pulls it up. SDO selects the address in I2C (GND = 0x76, the firmware's), and is the data output in SPI: decided, it is an assembly-time choice through JP6 (see P4), with a note on the schematic. Beware: at init the firmware enables the PA5 pull-up, so SDO wired to MISO in I2C would give 0x77. | S |
| R12 | Question | I2C | The BH1750 (`USE_BH1750`, 0x23, env SOLAR_TEST) does not appear in the schematic. | Document it on the second I2C connector once R6 is done, with ADDR at 0. | S |
| R13 | Question | Power | `USE_CHARGE_PUMP` (`platformio.ini:66`, `:81`) is used by no code and matches no circuit. The TP4056 charger is not in the schematic. | Drop the flag or document the hardware concerned. | S |
| R14 | Question | MCU | The fuses are not recorded, BODLEVEL in particular. The firmware turns the BOD off during sleep. | Record the intended fuses in the repository (BOD 1.8 V at 1 MHz). | S |
| R15 | ◐ v3.0 (Minor) | Libraries | `fp-lib-table` points to `battery.pretty` and `Chinese breakouts.pretty`, which are missing. `sym-lib-table` declares a Legacy library that does not exist. J13 uses a footprint removed from KiCad, and J1/J3/J4 removed `Conn_*_Male/Female` symbols. U1 and J6 come from `-rescue.lib`. The file is in KiCad 7 format. | Clean the tables. Use standard symbols and footprints (`ATtiny84A-P`, `AVR-ISP-6`, `Conn_01x0N_Pin/Socket`, `TerminalBlock_Phoenix` 5.08), then "Update Symbols from Library". | S–M |
| R16 | ◐ v3.0 (Minor) | Connectors | J1 is wired GND/VCC/DATA, while an FS1000A is DATA/VCC/GND. J5 is wired VCC/GND/DQ, while a TO-92 DS18B20 is GND/DQ/VDD. ISP pad 1 is round, so nothing marks it. | Pin functions on the silkscreen, and a square pad 1 on the ISP. | S |
| R17 | ◐ v3.0 (Minor) | Hygiene | Values "4,7k" and "4k7" mixed. JP3 has value "~". C5, a 1 nF ceramic, uses a tantalum footprint, and C6 (0.1 µF) a polarised symbol. The R4 note (75R or 18k) is unclear. A wire dangles at (54.61 ; 120.65). The title block says Rev 2.3, the board v2.2. | Normalise values, switch C5 to `C_Disc`, delete the wire. | S |
| R18 | Evolution | v2.3 | Ideas for a v2.3: ATtiny84A-SSU or ATtiny1614 (UPDI, hardware TWI, low-power RTC), OOK emitter for 1.8–3.6 V (STX882, SYN115), nano-power boost with enable, ideal-diode protection, switched divider. | — | L |

Not checked: the breakouts' internal pull-ups, the real DC-DC quiescent current, the fuse values.

## PCB

The board is 2 layers, 1.6 mm, 93.98 × 63.75 mm, with a GND plane on B.Cu. All parts are through-hole, except JP2–JP6 (SMD on B.Cu).
DRC, with parity and zone refill, reports 42 violations, 2 unconnected items and 39 parity mismatches. About 70 of these messages come from libraries or fields.
The cropped variant for the mushroom was checked on a copy cut at x 147 / y 87: ground stays continuous, and MH1, MH5, J1, I2C and J13 fall exactly on the `pcb.scad` dimensions.
The TP4056 is an external module wired to J13: the charge current does not cross the board.

| ID | Sev. | Area | Finding | Fix | Effort |
|---|---|---|---|---|---|
| P1 | ✅ v3.0 (Critical) | Ground | The GND pads of the "I2C" connector (pads 2 and 6) and of ISP J6 (pad 6) sit on an isolated B.Cu copper island, surrounded by the VCC, SENSOR_VCC and MISO tracks and the copper text. DRC: `unconnected_items` I2C.6 ↔ GND zone, `starved_thermal` I2C.2. The mushroom BME280 plugs in right there: **it has no ground**. | GND track on F.Cu from I2C.6 to J6.6 then to the plane, or a GND via in the island. Rerun DRC until 0 unconnected. | S |
| P2 | ✅ v3.0 (Critical) | Power | The − pole of the 18650 holder (`REF**` BH-18650, pad 2) has no net, and the holder does not exist in the schematic. The whole board cannot run on an 18650. | Add it to the schematic (BT1, − on GND), update the PCB, silkscreen +/−. | S |
| P3 | ✅ v3.0 (Major) | Sync | Same mismatches as R6. MH1 is duplicated, at (203 ; 52) and at (203.75 ; 105.51), which makes the hole naming used by the enclosure ambiguous. | Handled with R6, re-annotating the mounting holes too. | M |
| P4 | ✅ v3.0 (Major) | Netlist | JP6 ties MISO to MISO, and JP5 GND to GND: these jumpers are useless. I2C pad 5 has no net. I2C_SPI puts MISO on pin 5, while a BMx280 breakout pinout is VCC/GND/SCL/SDA/CSB/SDO. | Decided: JP6 becomes a 3-pad selector for sensor-connector pin 5 (SDO), GND side for I2C at 0x76, MISO side for SPI. Add a note on the schematic for whoever builds the board. JP5 (GND–GND) to delete or justify. Linked to R11. | S |
| P5 | ◐ v3.0 (Major) | Cut | The cut lines cross copper: the GND plane, and two F.Cu +BATT tracks crossing x = 147 (near y 59.3) and y = 87. A sawn edge bares +BATT and GND, and a burr can short them outdoors. | 1.5 mm copper keep-outs on both layers along both lines. Route +BATT through a bridge or a jumper outside the kept part. | S |
| P6 | ✅ v3.0 (Major) | Fab | `min_copper_edge_clearance` is 0.05 mm, and the GND plane comes 0.05 mm from the edge. Fabs ask for at least 0.2–0.3 mm, and bare copper oxidises outdoors. | Set the clearance to 0.5 mm, refill, rerun DRC. | S |
| P7 | Major | Mech | MH5 (150.89 ; 51.84) is under the J2 DC-DC module, while the mushroom screws the board through MH5. Once J2 is soldered, the screwdriver cannot reach it. | Move MH5 away from J2 (and update `pcb.scad`) or move J2. To check: does the module have an aligned hole? ✅ v3.0: J2 is now only a 1×03 header, the MH5 screw stays reachable, `pcb.scad` unchanged. | M |
| P8 | ✅ v3.0 (Major) | Mech | C6 (through-hole tantalum) is on B.Cu, while `pcb.scad` models it on top. Standing, it touches the floor (`pcb_gap` = 5 mm). | Move C6 to F.Cu, lay it down, or fix the model. | S |
| P9 | ◐ v3.0 (Major) | Assembly | Nothing marks the pins on the silkscreen: no +/− on J13 RAW IN, no pin functions on J1, J5 or J7. All texts are on copper, not silkscreen. | F.Silkscreen texts (at least 1 mm). +/− on J13, G/V/D on J1, V/G/Q on J5. Overlaps R16. | S |
| P10 | ✅ v3.0 (Major) | LED | Same finding as R5: the D1 anode is on GND. | See R5. | — |
| P11 | ✅ v3.0 (Minor) | Mech | Mounting holes are 2.5 mm in diameter, too tight for M2.5. `pcb.scad` models them at Ø2.7. | `MountingHole_2.7mm_M2.5`. | S |
| P12 | Minor | Cut | J2 (x 147.57) and J13 (≈ 147.7) graze the x = 147 line: a 1 mm saw kerf can bite into them. | Line at x ≈ 145.5 (the CR2032 holder stops near 141) and update `k_crop`, or move J2/J13. To check: do the J13 wires enter from the cut side? | S |
| P13 | ✅ v3.0 (Minor) | Power | The radio decoupling is far: C4 (10 µF) is about 11 mm from J1, and there is no 100 nF. | 100 nF between J1.1 and J1.2, and C4 within 5 mm. | S |
| P14 | ✅ v3.0 (Minor) | Ground | U1 pin 14 is not tied to the plane (`zone_connect 0`). Its ground goes only through C2 and JP5. | Thermal connection to the plane. | S |
| P15 | Minor | Power | The +BATT tracks are 0.25–0.3 mm, about 0.09 Ω over 45 mm. Enough for about 100 mA. | Widen them to 0.5–0.8 mm, for margin. | S |
| P16 | ✅ v3.0 (Minor) | Assembly | A 2.54 mm female header in a 2.50 mm JST-EH footprint accumulates 0.2 mm of error over 6 pins, with 0.95 mm drills. | 1.0–1.1 mm drills, or a dual-pitch footprint. | S |
| P17 | ◐ v3.0 (Minor) | Hygiene | Hidden references: U1, C2, C3, C6, D2, D3, R6, J2, J7, I2C, I2C_SPI. C5 (1 nF) is on a tantalum footprint marked "+". The mask clips the SW1 and J10 silkscreen, and `min_silk_clearance` is 0. Orphan track stubs remain: GND (202.8 ; 97.0) and VCC (162.7–163.1 ; 61.7). Libraries are broken (see R15). | Clean up. | S |
| P18 | Question | Radio | Which module goes into J1, and which way round? A 19 mm module standing up may get in J5's way. There is no RF track, so the ground plane is not at stake. | — | — |
| P19 | Question | Sensor | In the box variant, the sensor connector is 7 mm from U1, over the GND plane. Negligible given the duty cycle. In the mushroom, the BME280 hangs under the board: good. | — | — |
| P20 | Evolution | v2.3 | Ideas: real tabs or mouse-bites with copper keep-out, mounting holes away from modules, a single protected battery input (see R2: CR2032, 18650 at 4.2 V and boost input are in parallel), BH1750 footprint, thermal isolation slot around the sensor. | — | L |

Not checked: nothing was opened in the GUI, Gerbers and the drill file were not exported, and 3D collisions between modules and headers were not checked. The radio module dimensions and the J13 wire entry side are assumed.

## Suggested order, schematic and PCB together

1. **R3, R4 and R5/P10**, the firmware and the LED: rule PLAT-POWER-003 to reread.
2. **P1**: without ground, the mushroom BME280 does not work. Fix before any manufacturing.
3. **R1 and R2/P2**, power safety.
4. **R6/P3/P4**: make the schematic the reference again before any other change.
5. **P5, P6, P7, P8, P12**, the mushroom manufacturing and mechanical items.
6. The rest.

## Firmware, follow-up of the review

| ID | Sev. | Finding | Fix | Effort |
|---|---|---|---|---|
| F1 | Major | With the divider added by the v2.2 rework (R1), and in v3.0, PA1 only gets half the battery voltage. | Add a divider-ratio setting (for example `BATTERY_DIVIDER`) in `main.cpp`, or in `ConvertAnalogValueToMv`, through the RBD workflow. | S |

## Revision 3.0 (2026-11-06)

Schematic and PCB are stamped **Rev 3.0** (title block and "TinySensor v3.0" silkscreen). They were generated from v2.2 by `scripts/make_v3_pcb.py`, `scripts/make_v3_sch.py` and `scripts/sync_paths.py` (KiCad 10, `pcbnew` in Python).
DRC with schematic parity: 0 footprint errors, 0 unconnected pads. The only remaining error is the J10 courtyard, malformed in the library. ERC: no errors, only inherited library warnings.

**Fixed:**
- **R5/P10:** D1 rotated.
- **R1:** PA1 goes through the R7/R8 1 MΩ divider, with C7 100 nF; the pin reads Vbatt / 2 (see F1).
- **R2:** Q1 (AO3401A) protects against reversal on J13 and BT1.
- **P1:** the sensor connectors' ground is tied to the plane.
- **P2:** BT1 18650 is wired, its − pole on GND.
- **R6/P3:** J3 and J14 replace the PCB "I2C" and "I2C_SPI" connectors. J11, J12 and JP5 are deleted, and the duplicate MH1 becomes MH3.
- **R11/P4:** JP6 selects SDO (pin 6), with 1-2 = GND by default and 2-3 = MISO.
- **R7:** R9 10 kΩ pull-up on RESET.
- **P13:** C8 100 nF at the radio.
- **P8:** C6 becomes an 0805 on top.
- **P11:** 2.7 mm M2.5 holes.
- **P16:** 1.05 mm drills.
- **P6:** 0.5 mm between copper and edge.
- **P5 (partly):** no ground plane along the cut lines. Two GND bridges cross these strips, and two +BATT tracks still run through them.
- **P9 (partly):** texts moved to silkscreen, with "+" on J13, G/V/D on J1, V/G/Q on J5 and the note "1 battery source only".
- **R15 (partly):** library tables cleaned, and the J13 and BT1 footprints copied into `library/`.

**Still open for v3.0:**
- **P7:** ✅ J2 is a 1×03 header (`scripts/v3_j2_header.py`), MH5 stays reachable.
- **P12:** J2 and J13 stay close to x = 147.
- **P15:** +BATT tracks to widen.
- **R8, R12, R13, R14, R18/P20:** not handled.
- **R16:** square pad 1 on the ISP.
- **R17:** R4 note and dangling wire.
- **Hygiene:** 28 footprints no longer match their library version, and the inherited (rescue) symbols are to replace.
- **Manufacturing:** GUI review, Gerber and drill file export. Neither has been done.
- **Schematic:** a few power symbols overlap texts (readability only).
- **F1:** the firmware must multiply the PA1 reading by 2.
