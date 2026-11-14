# PCB review — tinySensor v2.2, 2026-11-06

Raw report from the review agent (KiCad 10.0.7). The `scratchpad/` paths point to temporary files; the useful reports are copied alongside.

### 1. Summary

**Tools:** kicad-cli 10.0.7 ran from the AppImage. It produced a DRC with schematic parity, a DRC with zone refill, SVG and PDF layer exports and 3D renders. I also wrote small Python S-expression scripts for the netlist, track and zone-island checks. For the cropped variant I made a scratch copy of the board, cut the outline to x 147–207.02 / y 48.03–87, deleted the footprints outside it and ran DRC with refill on that copy. Nothing in the repo was touched.

**Board:**
- 2 copper layers, 1.6 mm FR4, 1 oz, finish "None".
- Outline 93.98 × 63.75 mm, one closed Edge.Cuts polygon (113.04–207.02 × 48.03–111.78).
- One GND pour on B.Cu covering the whole board. All parts are through-hole except the solder jumpers JP2–JP6 on B.Cu.

**DRC before triage:** 42 violations, 2 unconnected items, 39 parity problems. Breakdown:
- 31 lib_footprint_mismatch, 3 lib_footprint_issues, 3 malformed_courtyard (J10)
- 3 silk_over_copper, 1 silk_overlap
- 1 starved_thermal (I2C pad 2)
- 28 footprint_symbol_field_mismatch, 5 extra_footprint, 4 missing_footprint, 1 duplicate_footprints

About 70 of these are library or field noise.

**Scope differs from the brief:** the board has no TP4056, no BH1750, no DS18B20 footprint (only the J5 header) and no charge pump IC. Per the enclosure README the TP4056 is an external module wired to J13, so 1 A charge current never crosses this board.

**Verdict:** routing and fab rules are mostly fine for a cheap 2-layer fab, but the board is not ready to order:
- The GND of the "I2C" connector and of the ISP connector sits on an isolated copper island. The BME280 in the mushroom build would have no ground.
- The 18650 holder's negative contact has no net.
- The PCB has drifted from the schematic: about 6 footprints exist only on the board, and the jumpers JP5/JP6 connect a net to itself.

The cropped variant itself is sound: its GND stays connected after the cut, and the hole, J1, I2C and J13 coordinates match `pcb.scad` exactly. The cut does go through copper.

### 2. Findings

**P1 — critical — ground — "I2C" and ISP J6 GND pads are on an isolated B.Cu island**
- Evidence: DRC `unconnected_items` between I2C pad 6 [GND] at (186.85, 77.11) and the GND zone, plus `starved_thermal` on I2C pad 2 (196.85, 77.11). My zone-island check puts I2C pads 2 and 6 and J6 pad 6 (173.53, 80.29) in fill island #1 (bbox 170.5–198.6 × 71.9–80.9). That island is fenced off by B.Cu tracks: VCC at y = 73.05, SENSOR_VCC at y = 73.80 and x = 199.21, MISO at y = 79.99 / 81.51, the copper text "GND SCL SDA" and JP6. No GND track reaches it.
- Why it matters: the mushroom build plugs the BME280 into this exact footprint (README step 1), so the sensor floats. The ISP has no ground reference either, unless the programmer finds ground through another path.
- Fix: run a GND track (F.Cu) from I2C pad 6 to J6 pad 6 and on to the main pour, or add a GND via inside the island, or move one fence track to F.Cu. Re-run DRC until it reports 0 unconnected.
- Effort: S

**P2 — critical — power / parity — 18650 holder negative is unconnected and the holder is not in the schematic**
- Evidence: footprint `REF**` BatteryHolder_MPD_BH-18650-PC2 at (124.72, 99.08). Pad 1 is +BATT; pad 2 at (196.72, 99.08) has no net. Parity reports `extra_footprint REF**`, and the pad shows as isolated in the B.Cu plot.
- Why it matters: the uncropped board cannot run from the 18650. A plain "REF**" designator also means nothing ties it to the BOM.
- Fix: add the holder to the schematic (e.g. BT1) with − on GND, update the PCB, and put +/− marks on the silkscreen.
- Effort: S

**P3 — major — DRC / parity — schematic and PCB have diverged**
- Evidence:
  - Missing on the board: J3 (BMx280_breakout), J11 (AAA), J12 (AA), MH3.
  - Only on the board: I2C, I2C_SPI, JP5, JP6, REF**.
  - MH1 appears twice, at (203, 52) and (203.75, 105.51).
  - JP3 value mismatch.
- Why it matters: a Netlist update from the schematic would delete the I2C connectors and jumpers or re-add removed parts, and the BOM is wrong. The "MH1 top row" naming the enclosure relies on is ambiguous.
- Fix: make the schematic the source again: add the JST/header symbols, JP5, JP6 and BT1, remove J11/J12/MH3 or mark them DNP, and re-annotate the mounting holes.
- Effort: M

**P4 — major — DRC / netlist — JP6 and JP5 short a net to itself; I2C pad 5 has no net**
- Evidence: JP6 (189.35, 80.71) has MISO on both pads and causes DRC `unconnected_items` on MISO. JP5 (166.13, 61.23) has GND on both pads. I2C pad 5 (189.35, 77.11) has no net. I2C_SPI uses pin 5 = MISO and pin 6 = GND, whereas a BMx280 breakout is VCC, GND, SCL, SDA, CSB, SDO.
- Why it matters: the jumpers do nothing, and the intent (CSB or SDO select, a ground split?) is invisible. SPI on I2C_SPI probably cannot work with a standard BMx280 breakout.
- Fix: question for the schematic reviewer. Give each jumper side its own net (e.g. MISO_EXT) and decide what pin 5 of each connector should carry.
- Effort: S

**P5 — major — cropped variant — the cut lines go through copper**
- Evidence: the B.Cu GND fill spans 113.09–206.97 × 48.08–111.73, so it crosses both x = 147 and y = 87. Two F.Cu +BATT tracks cross the lines: (130.06, 76.72)–(149.53, 57.25) crosses x = 147 at y ≈ 59.3, and (148.35, 81.32)–(148.35, 88.16) crosses y = 87. The cropped-copy DRC shows 2 `copper_edge_clearance` (0 mm) and 2 `track_dangling` on these stubs.
- Why it matters: a hand-sawn or filed edge exposes the GND pour and +BATT stubs, and copper smear can bridge the battery to GND on an outdoor board. Connectivity of the cropped part is fine (no new unconnected items).
- Fix: add 1.5 mm copper keep-out rule areas on both layers along both cut lines. Route the +BATT feeds to the holders through a narrow neck or solder jumper placed outside the kept part.
- Effort: S

**P6 — major — fab — copper reaches 0.05 mm from the board edge**
- Evidence: `min_copper_edge_clearance` = 0.05 mm in `tinySensorv1.kicad_pro`. The GND fill reaches x = 113.09 / 206.97 against edges at 113.04 / 207.02.
- Why it matters: JLCPCB/PCBWay want at least 0.2–0.3 mm. Edge routing exposes copper, which then corrodes outdoors.
- Fix: set the edge clearance to 0.5 mm, refill, re-run DRC.
- Effort: S

**P7 — major — mechanical / cropped — MH5 sits under the DC-DC module (J2)**
- Evidence: J2 silk box covers x 147.57–166.57, y 49.79–58.79. MH5 at (150.89, 51.84) is inside it. `pcb.scad` puts the module PCB 5 mm above the board, and `mushroom.scad` screws the board down with M2.5 through MH5.
- Why it matters: once J2 is soldered, a screwdriver cannot reach MH5. Rework or disassembly means desoldering the module.
- Fix: move MH5 out of the J2 outline (and update `pcb.scad`), or move J2. Question: does the module have its own hole aligned with MH5?
- Effort: M

**P8 — major — mechanical — C6 is a through-hole tantalum on B.Cu, but the enclosure model puts it on top**
- Evidence: C6 is CP_Radial_Tantal_D4.5mm on layer B.Cu at (164.09, 54.63). `pcb.scad` uses `kfp(...)` (top side) with an 8 mm body. `mushroom.scad` has `pcb_gap = 5` (deck to board underside).
- Why it matters: standing upright, C6 would hit the deck.
- Fix: move C6 to F.Cu (it is far from the module pins), or lay it flat, or fix the model.
- Effort: S

**P9 — major — assembly — connector pins and polarity are not labelled on the silkscreen**
- Evidence: there are no +/− marks on J13 "RAW IN" (pad 1 +BATT is square). J1 RADIO (GND / VCC / DATA), J5 OneWire (VCC / GND / DQ, which does not match the DS18B20 TO-92 order GND / DQ / VDD) and J7 have no pin labels. All board labels ("TinySensor v2.2", "Signal output", "OneWire", "i²c", "DC-DC", "Analog", "GND SCL SDA") are copper text on F.Cu/B.Cu, not silkscreen.
- Why it matters: battery wires on J13 come from an external TP4056 in the mushroom build, and a reversed J13, J1 or J5 is easy to wire.
- Fix: move the labels to F.Silkscreen at 1 mm or more, add +/− at J13, G/V/D at J1 and V/G/Q at J5, and label the cable side.
- Effort: S

**P10 — major (cross-check) — netlist — LED D1 anode is on GND**
- Evidence: D1 pad 2 (A) = GND at (158.00, 79.02). Pad 1 (K) = Net-(D1-K) → R1 → JP1 → LED_OUT.
- Why it matters: a driven pin can never forward-bias it, so the LED never lights.
- Fix: for the schematic reviewer: swap to MCU → R → A, K → GND, or anode to VCC with active-low drive.
- Effort: S

**P11 — minor — mechanical — 2.5 mm NPTH for M2.5 screws**
- Evidence: all MountingHole_2.5mm have a 2.5 mm drill. `pcb.scad` models them as d = 2.7.
- Why it matters: no clearance for M2.5, and the board and model disagree.
- Fix: use MountingHole_2.7mm_M2.5 (2.7–2.9 mm).
- Effort: S

**P12 — minor — cropped variant — J2 and J13 sit close to the x = 147 cut**
- Evidence: J2 silk starts at x = 147.57, the J13 body at about 147.7.
- Why it matters: a 1 mm saw kerf can nick them.
- Fix: move the line to x ≈ 145.5 (the CR2032 holder ends near 141), or shift J2/J13 right, and update `k_crop`.
- Effort: S
- Question: does J13's wire entry face −X (the cut edge)?

**P13 — minor — power — radio decoupling is far from J1 and has no ceramic**
- Evidence: J1 VCC (193, 59.4) reaches C4 (10 µF electrolytic, 194, 70) through about 11 mm of 0.25 mm B.Cu. There is no 100 nF at the radio.
- Fix: add a 100 nF across J1 pins 1–2 and keep C4 within 5 mm.
- Effort: S

**P14 — minor — ground — MCU ground reaches the pour only through C2**
- Evidence: U1 pin 14 has `zone_connect 0` (no zone connection). It reaches the pour via F.Cu to C2 pad 1 (165.5, 64) and the redundant JP5. Decoupling itself is fine: C2 sits about 3–4 mm from U1 pins 1 and 14.
- Fix: let pin 14 use a thermal connection to the pour.
- Effort: S

**P15 — minor — power — +BATT tracks are 0.25–0.3 mm**
- Evidence: the 18650 → J13 → J2 path is about 45 mm long, roughly 0.09 Ω. That is fine at about 100 mA boost-input peaks.
- Fix: widen +BATT/GND feeds to 0.5–0.8 mm for margin. Not needed for TP4056 current, which stays off-board.
- Effort: S

**P16 — minor — assembly — 2.54 mm female header in the 2.50 mm JST-EH footprint**
- Evidence: 6 pins accumulate 0.2 mm of error, and the 0.95 mm drill leaves about 0.03 mm margin on a 0.64 mm square pin (diagonal 0.9 mm).
- Fix: use 1.0–1.1 mm drills or a dual-pitch footprint.
- Effort: S

**P17 — minor — assembly / fab — silkscreen and library hygiene**
- Hidden references: U1, C2, C3, C6, D2, D3, R6, J2, J7, I2C, I2C_SPI.
- C5 ("1n", unpolarised) sits on a tantalum footprint with a "+" mark.
- Silk is clipped by mask on SW1 and J10. `min_silk_clearance` is 0.
- `fp-lib-table` points to "Chinese breakouts.pretty" and "battery.pretty", which do not exist, hence the J10 lib issues and malformed courtyard.
- Junk B.Cu stubs: GND at (202.8, 97.0) and VCC at (162.67–163.05, 61.74).
- Effort: S

**P18 — question — radio — module orientation on J1**
- Evidence: J1 at (193, 56.86–61.94). A vertical 433 MHz module about 19 mm wide, standing along y, may come close to the J5 header (y = 51.18) depending on the module.
- What I need: which module, and which way does its face point?
- Antenna: no RF trace on the board, so the pour does not matter here. Placement is about 15 mm from the I2C sensor and far from J2.

**P19 — question — sensor — thermal path in the box variant**
- Evidence: the sensor connector sits 7 mm from U1, with the pour running under it. Heat is negligible at this duty cycle. In the mushroom build the BME280 hangs below the board, which is good.

**Upgrades for v2.3 (secondary):**
- Proper cut tabs or mouse-bites with copper keep-outs.
- Mounting holes clear of modules.
- One labelled battery input with reverse protection.
- Note that +BATT ties the CR2032, the 18650 (4.2 V) and the 0.9–3.3 V boost input in parallel. That is a schematic issue: it reverse-charges the CR2032 and gives a boost converter a higher input than its output.
- An optional slot around the sensor corner.
- A footprint for the BH1750.

### 3. Scratch outputs

All under ``:

- Full-board DRC (parity, all severities): `drc.rpt` and `drc.json`
- Full board after zone refill: `drc_full_refill.rpt`
- Cropped variant board and DRC: `tinySensorv1.kicad_pcb` and `drc_crop.rpt`
- Images in `img/`:
  - `render_top.png`, `render_bottom.png`
  - `F.Cu-1.png`, `B.Cu-1.png`, `F.Silkscreen-1.png`
  - `B.Cu_i2c_zoom-1.png` (shows the isolated island)
  - `.svg` and `.pdf` versions of the same layers
- Scripts: (tools not kept) (`sx.py`, `fps.py`, `trk.py`, `gr.py`, `sch.py`, `crop.py`)

**Not checked:** I did not open the GUI or export Gerbers or a drill file. I did not look at the 3D clash between real module models and the headers. The radio module's dimensions and the J13 wire-entry side are inferred, not verified.
