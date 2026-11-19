# Wall box, radiation shield, table stand

| On the wall | On a stand |
|---|---|
| ![Overview, on the wall](rendu.png) | ![Overview, on a stand](rendu_pied.png) |

![Open box: board and 18650 cell in its trough](rendu_ouvert.png)

![Exploded view: base with the cell trough, board, lid](../../docs/img/wallbox_exploded_iso.png)

| File | Content |
|---|---|
| `tinysensor_box.scad` | Main wall box (base + lid) |
| `sensor_shield.scad` | Outdoor radiation shield for a BMx280 breakout + optional clip |
| `stand.scad` | Table stand: mast + foot, to stand the box + shield upright |
| `assembly.scad` | Overview (box + clipped shield, on the wall or on the stand), generates `rendu.png` and `rendu_pied.png` |

> Status: **not printed, not validated** (wall box, shield, stand). Clearances and fits to confirm on the first print.

Orca projects: `box.3mf`, `shield.3mf`, `stand.3mf` (see [export](../README.md#export)).

Open-box render: `openscad --preview -D 'part="open"' --camera=110,-230,420,110,30,0 --imgsize=1400,1000 -o rendu_ouvert.png tinysensor_box.scad`. Exploded view (`docs/img/wallbox_exploded_iso.png`): command in the `tinysensor_box.scad` header (`part="exploded"`).

## Main box (`tinysensor_box.scad`)

- Fitted to the v3.0 PCB (`hardware/tinysensor/tinysensor.kicad_pcb`): 94.81 × 39.33 mm, full board (not cut), held by its 3 M3 holes MH1–MH3 (all along the top edge) on 5 mm standoffs (Ø7, Ø2.5 pilot for M3 self-tapping screws). The v3.0 board has no 18650 holder.
- Clearance above the PCB: 25 mm (433 MHz module standing on J1), parameter `comp_h`.
- Layout, wall-mounted, from the bottom up: free 14 mm bay (cables, cable gland, antenna), 18650 cell trough, board.
- Cavity 96.81 × 77.93 mm. Base 117.61 × 122.74 × 36 mm (wall tabs included), lid 117.61 × 98.73 × 4 mm: both fit an Ender 3 (220 × 220).
- **Closure**: 6 M3 self-tapping screws, outside the seal area.
- **Sealing**: 2.2 × 1.5 mm groove for a Ø2 cord (printed TPU or EPDM), squeezed by a lid rib.
- **Wall mounting**: 2 tabs, Ø4 countersunk screws. The back stays solid.
- Variants (`usage`):
  - `ext`: Ø12.5 hole for a **PG7** cable gland (remote probe). By default in the **left side wall**, level with the bay, under the flange chamfer that keeps the rain off; `gland_side` = `left` / `right` / `bottom` (former position, bottom wall). The cable runs down along the box then up into the shield through its base: allow ~15 cm more and a drip loop.
  - `int`: top and bottom vent slots (convection).
- The bottom tab has two side notches for the shield clip (`shield_notch`).

### 18650 cell trough

Moulded on the base floor: lighter version of the mushroom cradle, same cell and contact parameters (`../cell_cradle.scad`).

- Trough 74.8 × 22.6 × 22.1 mm, centred in X, between the cable bay and the board; cell axis 12.8 mm above the floor. Bare 18650, flat top, 65 mm.
- Two **tab contacts** (13 × 11 mm plate, 3 × 7 tab) slid into the end grooves: spring on the "−" end, bump on the "+" end, tabs up. The wire notches face the board (J13).
- Two saddles under the cell, each with a 4 × 1.6 mm tunnel for a zip tie (≤ 3.6 mm wide): 2 zip ties hold the cell against the "−" spring.
- With an 18650 and a BME280, the **TPS63020** 3.3 V module on J2 is required (the cell voltage exceeds the BME280 maximum).
- No charger in the box: take the cell out to charge it.

Cell wiring (22–24 AWG; lengths with some slack, adjust on assembly):

| # | Colour | Signal | From (end) | To (end) | Length | Gauge |
|---|---|---|---|---|---|---|
| W1 | red | RAW_IN | trough "+" tab — soldered | J13 "RAW IN" pin 1 (RAW_IN) — JST-XH (F) 2p | ~70 mm | 22–24 AWG |
| W2 | black | GND | trough "−" tab — soldered | J13 pin 2 (GND) — JST-XH (F) 2p | ~90 mm | 22–24 AWG |

Solder the wires to the tabs, never to the cell. Check red = pin 1 in the JST-XH housing before plugging it into J13.

## Radiation shield (`sensor_shield.scad`)

Stacked-plate shield (Stevenson type): outdoors, a BMx280 shut inside the sealed box would measure the box air, not the ambient air.

- Solid cap + 4 louvred plates + base, Ø56 mm, 10 mm pitch, ~52 mm high.
- Assembly: 3 **M3** threaded rods through the spacers, nuts under the base.
- Base: vertical slide for the **15 × 11 mm, 6-pin** breakout (`BMx280_breakout` footprint), Ø6 cable pass underneath, 6 drain holes.
- Print in **white** (limits solar heating). Place in the shade if possible.

### Mounting (pick one)

1. **Wall**: plate with 2 Ø4 countersunk screws on the probe side. Screw the cap to the wall **before** stacking the plates.
2. **Clip under the box** (`part="clip"`): adapter bolted behind the plate (2 M4×8 countersunk screws + nuts sunk on the wall side). Its two prongs grip the bottom wall tab of the box (45° lips) and a bump snaps into each notch. Fitting: screw the box, slide the shield up until it clicks.
   - Adjustable fit: `bump_r` (grip), `prong_w` (stiffness).

3. **On the stand** (`stand.scad`): see below.

## Table stand (`stand.scad`)

To stand the box + shield upright on a table, with no wall.

- **Mast** 30 × 6 × 210 mm: it stands in for the wall. The box screws onto it by its two wall tabs (M4 countersunk, nuts sunk in the back of the mast); the shield stays clipped under the bottom tab, as on the wall.
- **Foot** 130 × 136 mm with a 25 mm socket and gussets: the mast fits in and is locked by a through M4 screw. It reaches 90 mm forward to balance the shield, which sticks out to ~72 mm from the mast.
- 30 mm of clearance under the shield (`under_clr`) for the loop of the cable that comes up through the base.
- The mast (210 mm) fits a 220 × 220 bed.
- The box and shield dimensions are copied at the top of the file: realign them if the other models change.

### Wiring

- I2C link < 1 m: 4.7 kΩ pull-ups, twisted cable (SDA/GND, SCL/GND).
- The cable leaves the cable gland and runs up into the shield through its base.

Board connectors and module wiring: [`hardware/tinysensor/WIRING.md`](../../hardware/tinysensor/WIRING.md).


## Screws and fixings

| Use | Qty | Reference |
|---|---|---|
| PCB → standoffs | 3 | M3 × 6 self-tapping |
| Lid | 6 | M3 × 6 self-tapping (blind pilot: 3 mm lid + 4.2 mm, an M3 × 8 would bottom out) |
| Box → wall | 2 | Ø4 countersunk + wall plugs |
| Cable gland | 1 | PG7 |
| Seal | ~45 cm | Ø2 TPU/EPDM cord |
| Shield: assembly | 3 | M3 threaded rod ~60 mm + 6 nuts |
| Shield → wall (option 1) | 2 | Ø4 countersunk |
| Clip → shield (option 2) | 2 | M4 × 8 countersunk + nuts |
| Box → mast (stand) | 2 | M4 × 10 countersunk + nuts |
| Mast → foot (stand) | 1 | M4 × 20 + nut |

## Cell parts and consumables

| Use | Qty | Reference |
|---|---|---|
| Cell | 1 | bare 18650, flat top, 65 mm |
| Trough contacts | 1 set | tab contacts ("−" spring + "+" bump, 13 × 11 mm plate) |
| Cell hold | 2 | zip ties, ≤ 3.6 mm wide |
| Cell → J13 | 1 | 2-pin JST-XH female housing + 2 crimp contacts |
| Wires W1 / W2 | ~0.2 m | 22–24 AWG red and black wire |
| 3.3 V supply (18650 + BME280) | 1 | TPS63020 module on J2 |
