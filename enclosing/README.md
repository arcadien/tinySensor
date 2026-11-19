# 3D-printed enclosures — TinySensor v3.0

Parametric OpenSCAD models (tested with OpenSCAD 2021.01). Target material: **PETG**.

| Folder | Content |
|---|---|
| [`basic/`](basic/README.md) | Wall box, radiation shield, table stand, overview `assembly.scad` |
| [`flat/`](flat/README.md) | Flat indoor box for the CR2032 build, BME280 or DS18B20 soldered to the board (draft, not printed) |
| [`mushroom/`](mushroom/README.md) | Decorative fly-agaric enclosure, its wiring plan generator `harness.py` |
| [`receiver/`](receiver/README.md) | Desk enclosure of the tinyDecoders receiver |

Each folder holds its `.scad` files, renders and Orca projects. Shared at the root:

| File | Content |
|---|---|
| `pcb.scad` | Simplified boards for the renders: v3.0 (`pcb_v3`, cut `pcb_v3_cropped`), v2.2 kept for the boards still in use (`pcb_v22`, `pcb_v22_cropped`) |
| `cell_cradle.scad` | 18650 cell and tab-contact parameters, cradle cut modules: shared by the mushroom cradle and the wall-box trough |
| `euro2.scad` | €2 coin for render scale |
| `update_3mf.py`, `export.sh` | Update of every Orca project from the models (below) |

A €2 coin (Ø25.75 mm) appears in the renders for scale.

## Export

Ready-to-slice parts: **one OrcaSlicer project per model**, next to its `.scad`, one plate per use and every object named:

| Project | Plates | Objects |
|---|---|---|
| `basic/box.3mf` | Fond extérieur, Fond intérieur, Couvercle | print only one of the two bases: `ext` (cable gland, outdoor) or `int` (vents, indoor) |
| `basic/shield.3mf` | Abri, Assiettes | Abri: Chapeau, Fond, Clip (optional); Assiettes: Assiette × 4 |
| `basic/stand.3mf` | Mât, Pied | table stand, optional |
| `receiver/receiver.3mf` | Pupitre | Coque, Fond |
| `mushroom/mushroom.3mf` | Ailettes, Hat, White buttons, Ground, Divers, Foot, Support | Chapeau, Lamelles, Châssis, Pied, Sol, Berceau, Bouchon, Pois |

Plate and object names are the ones in the Orca projects and in `update_3mf.py` (French, kept as identifiers).

`mushroom/mushroom_cap.3mf`: mushroom cap as printed (Orca settings of that print). Frozen snapshot, outside `export.sh`.

After changing a `.scad` (about 30 s, close the projects in Orca first):

```bash
./export.sh                  # every project
./export.sh basic/box.3mf    # or just one
```

`export.sh` runs `update_3mf.py`, which re-exports each part and replaces its mesh in the project: plates, positions, rotations and Orca settings are kept, the previous project stays next to it as `<project>.3mf.bak` (ignored by git). A missing project is recreated, with the printer and filament settings of `mushroom.3mf` (Ender 3). Parts may only be rotated about Z in Orca, otherwise the script refuses to update them; a part deleted in Orca is reported, not put back. Per-object settings set when missing: mushroom cap outer brim 10 mm, 10 % infill; mushroom stem 6 walls; mushroom dots supports on from 45°, 5 mm brim. All the dots are one object, "Pois", on the White buttons plate.

Exporting a single part outside a project (STL, another format): `openscad -D 'part="lid"' -o out/lid.3mf basic/tinysensor_box.scad` (see each `.scad` header).

Every part is exported in its print orientation and prints **without support**, the mushroom dome included (the dots, printed standing, are the exception). `out/` is ignored by git.

## Power (to decide)

- No 18650 holder on the v3.0 PCB (removed); J13 "RAW IN" is a 2-pin JST-XH. Wall box: a bare 18650 in a trough on the base floor, wired to J13, no charger (the cell is taken out to charge); with a BME280 the TPS63020 module on J2 is required. Mushroom: the 18650 goes through a TP4056 to J13.
- Available AA/AAA holders: to fit in the bay or in a separate compartment (dimensions to provide).
- Solar: the TP4056 (linear charger, no MPPT) is a poor fit; prefer a CN3065 or a CN3791.
