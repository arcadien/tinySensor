# Revue du schéma (électronique numérique) — tinySensor v2.2, 2026-11-06

Rapport brut de l'agent de revue (KiCad 10.0.7). Les chemins `scratchpad/` renvoient à des fichiers temporaires ; les rapports utiles sont copiés à côté.

### 1. Summary

**Tools.** kicad-cli 10.0.7 ran directly from the AppImage (`kicad-10.0.7-x86_64.AppImage kicad-cli ...`), so no extraction was needed. I ran:
- `sch erc --severity-all`
- `sch export netlist`, `sch export pdf` and `sch export svg` (and rendered the PDF to PNG)
- `pcb drc --schematic-parity`, which I added to cross-check footprints against the schematic.

I also wrote small S-expression parsers for the netlist and the PCB pads. Nothing in the repo was modified.

**ERC: 0 errors, 47 warnings before triage.**
- 39 `lib_symbol_mismatch`: stale embedded symbols. Mostly power symbols, plus the rescue-library symbols U1, J6, C1/C3/C4/C6, JP1 and JP4.
- 3 `lib_symbol_issues`: Conn_01x0N_Male/Female no longer exist in the library (J1, J3, J4).
- 2 `footprint_link_issues`: J10 and J13.
- 1 `pin_to_pin`: real. U1 PA1 is tied to the +BATT power flag.
- 1 `multiple_net_names`: +BATT / Battery_Checker.
- 1 `unconnected_wire_endpoint`: a cosmetic wire stub.

Only the `pin_to_pin` and `multiple_net_names` warnings point at a design issue. Both show that PA1 is wired straight to the raw battery.

**PCB parity check: 39 issues.** The PCB is no longer generated from this schematic. Commit c8df224 "double I²C connector" changed only the .kicad_pcb.

**Verdict.** The MCU pinout, the ISP header and the firmware pin map all agree. I2C sits on the USI pins (PA6 SDA = MOSI, PA4 SCL = SCK), the radio is on PB0, sensor power on PA2, the battery ADC on PA1, soft serial on PA7, 1-Wire on PA3 and the LED on PB1.
- **Real schematic errors:** the LED is drawn reversed, and PA1 is wired to a raw battery that can exceed VCC.
- **Missing protection:** there is no reverse-polarity protection on any battery input.
- **Firmware/board interaction:** in hibernate, the internal pull-ups on SDA/SCL back-power the switched sensor rail, and the always-powered radio's data pin is left floating.
- **Out of sync:** the schematic and PCB have drifted apart (a second I2C connector, an 18650 holder and two jumpers exist only on the PCB), so the schematic cannot be trusted as the source for a v2.3.

### 2. Findings

**R1. Critical. Power / MCU pins. PA1 (battery sense) is hard-wired to raw +BATT with no series resistor or divider.**
- Evidence: net +BATT = J10.1, J11.1, J12.1, J13.1, J2.1 (DC-DC VIN), JP2.2 and U1.12 (PA1). This is ERC `pin_to_pin` U1 pin 12 against #FLG0102, plus `multiple_net_names` +BATT/Battery_Checker.
- Firmware reads it against the VCC reference: `Attiny84aHal.cpp:237` uses `AdcRead(_BV(MUX0))` with REFS=00.
- Why it matters: whenever the battery is above VCC + 0.5 V, the PA1 clamp diode conducts with no current limit. That happens with J13 RAW IN above 3.8 V, a Li cell (the PCB has an 18650 holder, and S_05 is commented "18650 powered"), or a fresh CR2032 above VCC. The result is a damaged MCU or a VCC rail back-powered through PA1. Any reading above VCC also saturates at 1023.
- Fix: put a series resistor of 10–100 kΩ on PA1, with about 100 nF from PA1 to GND so the ADC sample-and-hold sees a low source impedance. If the battery can exceed VCC, use a divider instead (for example 1 MΩ/1 MΩ plus 100 nF, or a divider switched from PA2 to avoid leakage), and update `ConvertAnalogValueToMv` accordingly.
- Effort: S.

**R2. Major. Power. No reverse-polarity protection, and the battery holders are paralleled with no isolation.**
- Evidence: J10 (CR2032), J11 (AAA), J12 (AA) and J13 (RAW IN screw terminal) all connect directly to +BATT/GND. The PCB also has an extra BH-18650 holder.
- Why it matters: a reversed RAW IN puts negative voltage on the DC-DC and, through the PA1 clamp to GND, on the MCU. With JP2 closed it goes straight onto VCC. If two cells are fitted at once, they cross-charge; charging a CR2032 is a safety hazard.
- Fix: add P-MOSFET reverse protection (low drop, which suits 1.5 V cells), or a Schottky diode on RAW IN only. Mark the holders as alternatives (DNP fields, or a note on the silkscreen).
- Effort: M.

**R3. Major. Power / firmware. Hibernate back-powers SENSOR_VCC through the I2C and 1-Wire pull-ups.**
- Evidence: R2 and R3 (4.7 k, SDA/SCL) and R6 (4.7 k, OneWire) are pulled up to /SENSOR_VCC, which is driven by PA2 (U1.11).
- During `sleep()`, `Attiny84aHal.cpp:85` sets `DDRA = 0`, so PA2 becomes a floating input. Line 89 then sets `PORTA |= 0b01110000`, which enables the internal pull-ups on PA4, PA5 and PA6. Line 42 also leaves the PA3 pull-up on from init.
- Why it matters: current flows VCC → internal pull-up → PA4/PA6 → R3/R2 → SENSOR_VCC, so the BME280, BH1750 and DS18B20 stay partly powered. Switching the sensor rail then saves nothing, and the sensors never get a clean power-on reset (this is a classic cause of BME280 lock-ups).
- Fix (firmware only): during sleep, keep PA2 as an output driven low, and leave PA3, PA4 and PA6 as inputs without pull-ups. The rest of the code already pulls those lines to 0 V through the resistors.
- Effort: S.

**R4. Major. Radio. The radio module is always powered, and its DATA pin floats in sleep.**
- Evidence: J1.2 RADIO is on VCC, not on SENSOR_VCC. J1.3 (/TX_RADIO) goes to PB0 with no pull-down. `sleep()` sets `DDRB = 0` (`Attiny84aHal.cpp:86`), so PB0 becomes high-impedance.
- The HAL comment at `Attiny84aHal.h:9` says "Power for radio is on PA2 (sensor_vcc)", which contradicts the schematic.
- Why it matters: a floating DATA input on an OOK transmitter (FS1000A-style) can key the transmitter. That wastes battery and emits spurious RF.
- Fix: keep PB0 as an output driven low during sleep (firmware). Optionally add a 100 k pull-down on TX_RADIO, and correct the HAL comment. A v2.3 option: power the radio from a switched rail.
- Effort: S.

**R5. Major. MCU pins / LED. D1 is reversed and will never light.**
- Evidence: in the netlist, D1.2 (A) is on GND and D1.1 (K) goes through R1 and JP1 to PB1. The PCB pads match: LED pad 2 = GND, pad 1 = Net-(D1-K).
- Firmware drives PB1 high to light it (`LedOn`, `TRANSMIT_WITH_LED` in `main.cpp`, the BADISR blink).
- Why it matters: there is no radio-activity indication and no fault-blink diagnostic, unless the LED was fitted against the silkscreen.
- Fix: swap D1 so the anode is on R1 and the cathode on GND.
- Effort: S.

**R6. Major. Libraries / ERC hygiene. The schematic and PCB are out of sync, so the schematic is not the source of truth.**
- Evidence: DRC schematic parity found 39 issues.
  - Footprints that exist only on the PCB: `I2C` and `I2C_SPI` (JST-EH 6-pin; I2C_SPI pin 5 = /MISO), `JP5` (GND–GND), `JP6` (MISO–MISO), and `REF**` BH-18650 holder (pad 2 has no net).
  - Missing from the PCB: J3, J11, J12 and MH3. MH1 is duplicated.
  - There are 2 unconnected items: the JP6 pad to its MISO track, and the I2C pad 6 to the GND zone.
  - The two I2C connectors were added in commit c8df224, which changed only the .kicad_pcb.
- Why it matters: any v2.3 work started from this schematic will drop the second I2C port. The 18650 negative terminal appears to be unconnected on the PCB, which you should verify.
- Fix: back-annotate the PCB features into the schematic (two I2C connectors, the 18650 holder, JP5, JP6), then run "Update PCB from Schematic" until the parity check is clean.
- Effort: M.

**R7. Minor. MCU pins. No external pull-up on RESET.**
- Evidence: the /RESET net contains only J6.5, SW1.1 and U1.4.
- Why it matters: the internal pull-up is weak (about 30–60 kΩ). On an outdoor board with a pushbutton, Microchip AN2519/AVR042 recommends 10 kΩ to VCC. Keeping it free of a capacitor stays compatible with ISP, and with debugWIRE, which the TinySensor/ Atmel Studio project uses.
- Fix: add 10 k from RESET to VCC, with no capacitor (a capacitor breaks debugWIRE).
- Effort: S.

**R8. Minor. Power. Bulk capacitance is small for a CR2032 feeding a radio during transmission.**
- Evidence: C1 and C4 are 10 µF, C2 is 100 nF, C5 is 1 nF, all on VCC. A CR2032 (J10) has about 10–20 Ω of internal resistance, and the radio draws tens of mA when keyed.
- Why it matters: VCC droops during transmission, which corrupts the measurement and can cause a brown-out reset.
- Fix: for CR2032 builds, add 100–470 µF of low-leakage bulk capacitance.
- Effort: S.

**R9. Minor. Power. JP2 "DIRECT Vcc" shorts the DC-DC module's VIN to its VOUT if both are fitted.**
- Evidence: JP2 connects VCC to +BATT, and J2 pin 1 is +BATT while pin 3 is VCC.
- Fix: add a schematic or silkscreen note "JP2 closed ⇒ J2 not fitted". Firmware also needs `BATTERY_IS_VCC` in that case, because GetRawBattery would otherwise read 1023.
- Effort: S.

**R10. Question. Power. The DC-DC module's quiescent current and enable pin.**
- Evidence: J2 uses the footprint "DCDC 0v9-3v3 to 3v3" with 3 pins (VIN, GND, VOUT) and no enable.
- Why it matters: cheap boost modules typically draw 10–100+ µA, which would dominate the ATtiny's few µA in power-down. Most of these modules are boost-only and pass VIN through when VIN > 3.3 V, which makes R1 worse.
- Suggestion: measure it. For v2.3, consider a TPS61099 or TPS610995-class boost (about 1 µA quiescent) with an enable pin.
- Effort: M.

**R11. Question. I2C. The BME280 CSB pin is left open.**
- Evidence: J3.5 (CSB) has a no-connect marker. J3.6 (SDO) is on GND, which gives address 0x76 and matches `include/BMx280.h:12`.
- Why it matters: the BME280 switches to SPI permanently if CSB is ever seen low. Floating CSB is only safe if the breakout pulls it up.
- Fix: tie CSB to SENSOR_VCC, or confirm the breakout has the pull-up. The PCB's I2C_SPI connector routes MISO to pin 5, so check which pin is intended for SPI.
- Effort: S.

**R12. Question. I2C / firmware. Where is the BH1750 connected?**
- Evidence: `USE_BH1750` is set in env SOLAR_TEST (`platformio.ini:157`), at address 0x23 (`include/LightSensor.h:20`). The schematic has only one I2C connector (J3), with no BH1750 and no ADDR strap. The second connector exists only on the PCB (see R6).
- Fix: document the BH1750 (ADDR tied low) on the second connector once it is back-annotated.
- Effort: S.

**R13. Question. Power. `USE_CHARGE_PUMP` and the TP4056 charger.**
- Evidence: `USE_CHARGE_PUMP` appears only in `platformio.ini:66` and `:81`; no code uses it. No charge-pump circuit exists in the schematic. D2 and D3 (1N914) form the "LM35 below 0°" offset on JP4. There is no TP4056 or 18650 in the schematic, only the 18650 holder on the PCB.
- Fix: either drop the flag, or document which hardware it refers to.
- Effort: S.

**R14. Question. Power. Brown-out detector fuses.**
- Evidence: there are no fuse settings in `platformio.ini`, and the firmware disables the BOD during sleep (`Attiny84aHal.cpp:59-61`).
- Fix: record the intended BODLEVEL fuse (1.8 V suits 1 MHz) in the repo.
- Effort: S.

**R15. Minor. Libraries. The library tables are broken and the symbols are stale.**
- Evidence:
  - `fp-lib-table` points "battery supp." to `battery.pretty` and "Chinese breakouts" to `Chinese breakouts.pretty`; neither directory exists. J10's footprint actually lives in `library/`.
  - `sym-lib-table` declares the "libraries" entry as a Legacy symbol library pointing at a `.kicad_mod` file that does not exist.
  - J13 uses `TerminalBlock_bornier-2_P5.08mm`, which has been removed from the KiCad library.
  - J1, J3 and J4 use the removed Conn_01x0N_Male/Female symbols.
  - There are 39 `lib_symbol_mismatch` warnings, with the MCU and ISP symbols coming from `tinySensorv1-rescue.lib`.
  - The file is format 20230121 (KiCad 7).
- Fix: point J10 at "TinySensor lib". Remove the dead library entries. Switch J13 to `TerminalBlock_Phoenix` or `TerminalBlock_MetzConnect` with 5.08 mm pitch. Replace the rescue symbols and the Conn_*_Male/Female symbols with stock `MCU_Microchip_ATtiny:ATtiny84A-P`, `Connector:AVR-ISP-6` and `Conn_01x0N_Pin/Socket`. Finally, run Tools > Update Symbols from Library.
- Effort: S–M.

**R16. Minor. Connectors. Pin-order and pin-1 risks.**
- Evidence:
  - J1 RADIO is ordered GND, VCC, DATA. A typical FS1000A module is DATA, VCC, GND, so this only works if the module is fitted rotated.
  - J5 is ordered VCC, GND, DQ, whereas a DS18B20 in TO-92 is GND, DQ, VDD.
  - The ISP footprint `ICSP_SOCKET_MINI_2x3` follows the standard 2x3 numbering (checked: pads 1/3/5 in one row, 2/4/6 in the other), but pad 1 is round, so there is no pin-1 marker on the copper.
- Fix: put pin-function labels on the silkscreen, and make pad 1 of the ISP footprint square.
- Effort: S.

**R17. Minor. ERC hygiene. Values and footprints.**
- Evidence:
  - R2 and R3 are "4,7k" while R6 is "4k7", so the BOM won't group them.
  - JP3 has the value "~" (flagged by the parity check).
  - C5 (1 nF, non-polarised symbol) and C6 (0.1 µF, polarised CP symbol) both use the tantalum footprint `CP_Radial_Tantal_D4.5mm`.
  - R4 is 75R, but its note says 18k for the LM35 build.
  - There is a dangling wire stub at (54.61, 120.65) under #PWR0119.
  - The title block says Rev 2.3, dated 2022-10-22, while the board is called v2.2.
- Fix: normalise the values, use `C_Disc` for C5, and delete the wire stub.
- Effort: S.

**R18. Upgrade (secondary). Points for v2.3.**
- Move to the ATtiny84A-SSU (SOIC) or an ATtiny1614 (UPDI programming frees PA4/PA6 from ISP sharing, has hardware TWI and a lower-current RTC sleep).
- Replace the FS1000A with a 1.8–3.6 V OOK module such as the STX882 or SYN115, so output power holds up at 3 V.
- Use a nano-power boost with an enable pin (see R10).
- Use an ideal-diode reverse protection (see R2).
- Use a switched, high-impedance battery divider (see R1).
- Effort: L.

### 3. Scratch outputs

All under ``:
- `erc.rpt`
- `drc_parity.rpt`
- `netlist.net`
- `netlist.txt` (flattened components and nets)
- `pcb_pads.txt` (net on each PCB pad)
- `schematic.pdf`
- `sch-1.png`
- `svg/tinySensorv1.svg`

**Not checked:** I did not review the PCB layout (only the footprint and net parity), the BME280 breakout's own pull-ups, or the DC-DC module's real quiescent current. The fuse values are not in the repo.

**Biggest risk:** R1 and R2 together. A Li cell or a reversed supply on J13 RAW IN can destroy the MCU through PA1.
