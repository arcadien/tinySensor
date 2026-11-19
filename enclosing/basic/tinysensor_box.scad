// TinySensor v3.0 — screwed wall box with seal, 18650 cell trough on the floor
// Units: mm. Target material: PETG.
//
// Export:
//   openscad -D 'part="base"' -o base.3mf tinysensor_box.scad
//   openscad -D 'part="lid"'  -o lid.3mf  tinysensor_box.scad
// Open-box render (board, cell in its trough, lid turned over next to it):
//   openscad --preview -D 'part="open"' --camera=110,-230,420,110,30,0 --imgsize=1400,1000 -o rendu_ouvert.png tinysensor_box.scad
// Exploded isometric view (README header):
//   openscad --preview -D 'part="exploded"' --camera=0,0,0,55,0,25,0 --viewall --autocenter --projection=o --imgsize=1400,1000 -o ../../docs/img/wallbox_exploded_iso.png tinysensor_box.scad
//
// Frame: origin = "top-left" corner of the PCB seen from above (as in KiCad),
// X to the right, Y down (KiCad) turned into Y up here.
// Wall-mounted, the "bottom" side (Y=0, cable bay and cell trough side) faces the ground.
//
// The v3.0 board has no 18650 holder: the cell lies in a trough moulded on the floor, between the cable
// bay and the board (lighter variant of the mushroom cradle, same tab contacts: ../cell_cradle.scad),
// held by the "−" spring and two zip ties through the saddles. Contact wires (red "+", black "−") end in a
// JST-XH 2-pin female housing plugged into J13 "RAW IN" (pin 1 RAW_IN, pin 2 GND; +BATT is behind the Q1 reverse-polarity MOSFET).
// 18650 + BME280: the TPS63020 3.3 V module on J2 is required (the cell voltage exceeds the BME280 maximum).

/* [Render] */
part = "assembly"; // [base, lid, assembly, open, exploded]
explode = 35;      // exploded view: gap between levels (part = "exploded")
usage = "ext";     // [int, ext]
gland_side = "left"; // [left, right, bottom] cable gland wall (usage ext)

/* [PCB v3.0 — from hardware/tinysensor/tinysensor.kicad_pcb] */
pcb_w = 94.81;
pcb_h = 39.334;
pcb_t = 1.6;
pcb_k0 = [113.04016, 48.02632];   // top-left corner of the outline, absolute KiCad coordinates
// M3 mounting holes MH1, MH2, MH3 (all along the top edge), absolute KiCad coordinates
pcb_holes_abs = [[203, 52], [151.35016, 52.11032], [117.85016, 52.86032]];

/* [Volumes] */
clr = 1.0;            // PCB / wall clearance
bay = 14;             // cable bay on the bottom side (cable gland, antenna)
standoff_h = 5;       // height under the PCB (THT legs)
comp_h = 25;          // tallest part above the PCB: 433 MHz module (FS1000A ~19 mm) standing on J1; C1 is 11.5
top_clr = 2;          // clearance under the lid

/* [Walls] */
wall = 2.4;           // 6 perimeters, 0.4 nozzle
floor_t = 2.4;
lid_t = 3;
r_out = 3;            // outer corner radius

/* [Seal and lid screws] */
flange = 8;           // outer flange width
flange_t = 5;         // flange thickness
groove_w = 2.2;       // groove for a 2 mm cord/seal (TPU, EPDM foam)
groove_d = 1.5;
rib_w = 1.2;          // lid rib squeezing the seal
rib_h = 1.0;
lid_screw_d_pilot = 2.5;  // M3 self-tapping in PETG
lid_screw_d_clear = 3.4;

/* [Fixings] */
pcb_screw_d_pilot = 2.5;  // M3 self-tapping
standoff_d = 7;
gland_d = 12.5;       // PG7
wall_screw_d = 4.5;   // Ø4 wall screw
ear_l = 12;           // tab overhang beyond the flange
shield_notch = true;  // notches to clip the radiation shield under the box
shield_notch_y = 8;   // notch position from the end of the bottom tab

/* [Cell trough — cell and contacts in ../cell_cradle.scad] */
trough_w = 22.6;      // end walls and saddles width (cell_d + 2 x 2)
trough_lift = 3.5;    // cell underside above the floor: zip-tie tunnel + 1.6 roof
saddle_l = 8;         // two saddles under the cell (length along the cell)
saddle_at = [0.3, 0.7];  // saddle positions, fraction of the trough length
tie_w = 4;            // zip-tie tunnel through each saddle (ties <= 3.6 wide)
tie_h = 1.6;

use <../pcb.scad>
use <../euro2.scad>
include <../cell_cradle.scad>

$fn = 48;
eps = 0.01;
// Side cable gland: in the bay (below the PCB in Y), high enough for the
// inner nut (PG7, 15 across flats) to clear the PCB edge, and
// under the flange chamfer that keeps the rain off.
gland_z = gland_side == "bottom" ? 11.5 : 15;
gland_x = 22;        // position in the bottom wall: offset from the central wall tab
gland_y = 8;         // position in a side wall: middle of the bay

// ---------- derived dimensions ----------
pcb_holes_kicad = [for (h = pcb_holes_abs) h - pcb_k0];   // relative to the PCB top-left corner
tr_cl_z = trough_lift + cell_d / 2;  // cell axis above the floor
tr_end_h = tr_cl_z + cell_d / 2;     // trough end walls: up to the top of the cell
cav_w = pcb_w + 2 * clr;
tr_x0 = (cav_w - cr_len) / 2;        // trough centred in X, between the cable bay and the board
pcb_x0 = clr;                        // PCB corner in the cavity frame
pcb_y0 = bay + trough_w + clr;
cav_h = pcb_y0 + pcb_h + clr;
cav_d = standoff_h + pcb_t + comp_h + top_clr;
base_h = floor_t + cav_d;
assert(tr_x0 >= 8, "Cell trough too long for the cavity (side gland nut)");
assert(tr_end_h + 2 <= cav_d, "Cell trough higher than the cavity");
groove_off = wall + 1.8;     // groove axis from the cavity edge
screw_off = wall + flange - 2.8; // screw axis from the cavity edge

function holes() = [for (h = pcb_holes_kicad) [pcb_x0 + h[0], pcb_y0 + pcb_h - h[1]]];

module rrect(w, h, r) {
  translate([r, r]) offset(r = r) square([w - 2 * r, h - 2 * r]);
}
// cavity rectangle offset by d (d>0 outwards)
module ring_rect(d, r = 1) {
  translate([-d, -d]) rrect(cav_w + 2 * d, cav_h + 2 * d, max(r, 0.5));
}
module groove_path(off, w) {
  difference() { ring_rect(off + w / 2, 1 + off); ring_rect(off - w / 2, 1); }
}
function lid_screws() = let(o = screw_off) [
  [-o, -o], [cav_w + o, -o], [-o, cav_h + o], [cav_w + o, cav_h + o],
  [cav_w / 2, -o], [cav_w / 2, cav_h + o]
];

// ---------- base ----------
module base() {
  difference() {
    union() {
      // body
      linear_extrude(base_h) ring_rect(wall, r_out);
      // top flange
      translate([0, 0, base_h - flange_t]) linear_extrude(flange_t) ring_rect(wall + flange, r_out);
      // 45° chamfer under the flange: printable without support
      hull() {
        translate([0, 0, base_h - flange_t - flange]) linear_extrude(eps) ring_rect(wall, r_out);
        translate([0, 0, base_h - flange_t - eps]) linear_extrude(eps) ring_rect(wall + flange, r_out);
      }
      // wall tabs (top and bottom)
      for (s = [-1, 1]) ear(s);
    }
    // cavity
    translate([0, 0, floor_t]) linear_extrude(cav_d + eps) ring_rect(0, 1);
    // seal groove
    translate([0, 0, base_h - groove_d]) linear_extrude(groove_d + eps) groove_path(groove_off, groove_w);
    // lid screw holes (blind)
    for (p = lid_screws()) translate([p[0], p[1], base_h - flange_t + 0.8]) cylinder(d = lid_screw_d_pilot, h = flange_t);
    // cable gland: side wall (default) or bottom wall
    if (usage == "ext") {
      if (gland_side == "bottom")
        translate([gland_x, -wall - flange - eps, gland_z]) rotate([-90, 0, 0]) cylinder(d = gland_d, h = wall + flange + 2 * eps);
      else
        translate([gland_side == "left" ? -wall - eps : cav_w - eps, gland_y, gland_z]) rotate([0, 90, 0]) cylinder(d = gland_d, h = wall + 2 * eps);
    }
    // indoor vents (slots in the bottom and top walls)
    if (usage == "int") vents();
  }
  translate([0, 0, floor_t - eps]) trough_placed() trough();
  // PCB standoffs
  for (h = holes()) translate([h[0], h[1], floor_t - eps]) difference() {
    cylinder(d = standoff_d, h = standoff_h + eps);
    cylinder(d = pcb_screw_d_pilot, h = standoff_h + 1);
  }
}

module ear(s) {
  // the tab sticks out of the flange to let the screwdriver through
  reach = wall + flange + ear_l;
  y = s < 0 ? -reach : cav_h;
  hy = s < 0 ? -(wall + flange + ear_l / 2) : cav_h + wall + flange + ear_l / 2;
  difference() {
    translate([cav_w / 2 - 10, y, 0]) cube([20, reach + eps, floor_t + 1.6]);
    translate([cav_w / 2, hy, -eps]) {
      cylinder(d = wall_screw_d, h = 10);
      translate([0, 0, floor_t + 1.6 - 2.2]) cylinder(d1 = wall_screw_d, d2 = wall_screw_d + 4.4, h = 2.2 + eps);
    }
    // side notches of the bottom tab: catch for the shield clip (sensor_shield.scad, part="clip")
    if (s < 0 && shield_notch)
      for (sx = [-1, 1]) translate([cav_w / 2 + sx * 10, -reach + shield_notch_y, -eps]) cylinder(r = 1, h = 10);
  }
}

module vents() {
  n = 7; sw = 2; sl = 14; pitch = (cav_w - 20) / (n - 1);
  for (yy = [-wall - eps, cav_h - eps]) for (i = [0 : n - 1])
    translate([10 + i * pitch - sw / 2, yy, floor_t + 3]) cube([sw, wall + 2 * eps, sl]);
}

// ---------- cell trough (cradle frame of ../cell_cradle.scad: X = cell axis, "−" at X = 0) ----------
// open trough: two end walls with the contact grooves and wire notches, two saddles with a zip-tie tunnel
module trough() {
  difference() {
    union() {
      for (x = [0, cr_len - cr_end]) translate([x, -trough_w / 2, 0]) cube([cr_end, trough_w, tr_end_h]);
      for (f = saddle_at) translate([f * cr_len - saddle_l / 2, -trough_w / 2, 0]) cube([saddle_l, trough_w, tr_cl_z]);
    }
    cradle_cell_cuts(tr_cl_z, tr_end_h);
    for (f = saddle_at) translate([f * cr_len - tie_w / 2, -trough_w / 2 - 1, -eps]) cube([tie_w, trough_w + 2, tie_h + eps]);
    cradle_polarity(tr_cl_z);
  }
}
// trough in the cavity, turned 180°: wire notches towards the board (J13), "−" end on the right
module trough_placed() { translate([tr_x0 + cr_len, bay + trough_w / 2, 0]) rotate([0, 0, 180]) children(); }

// ---------- lid ----------
module lid() {
  difference() {
    linear_extrude(lid_t) ring_rect(wall + flange, r_out);
    for (p = lid_screws()) translate([p[0], p[1], -eps]) cylinder(d = lid_screw_d_clear, h = lid_t + 2 * eps);
  }
  // rib squeezing the seal (inner face = z<0, turned over for printing)
  translate([0, 0, -rib_h]) linear_extrude(rib_h + eps) groove_path(groove_off, rib_w);
}

if (part == "base") base();
else if (part == "lid") translate([0, 0, lid_t]) mirror([0, 0, 1]) lid();  // flat face on the bed, rib up
else if (part == "open") {
  color("DimGray") base();
  translate([pcb_x0, pcb_y0, floor_t + standoff_h]) pcb_v3();
  translate([0, 0, floor_t]) trough_placed() cradle_cell_mock(tr_cl_z);
  // lid turned over next to it, inner face (rib) visible
  color("Gray") translate([cav_w + 2 * (wall + flange) + 15, 0, lid_t]) mirror([0, 0, 1]) lid();
  translate([cav_w / 2, -wall - flange - 25, 0]) euro2();
}
else if (part == "exploded") {   // render only: base, cell + contacts, board, lid (rib down), lifted apart
  color("DimGray") base();
  translate([0, 0, floor_t + explode]) trough_placed() cradle_cell_mock(tr_cl_z);
  translate([pcb_x0, pcb_y0, floor_t + standoff_h + 2 * explode]) pcb_v3();
  color("Gray") translate([0, 0, base_h + 3 * explode + comp_h]) lid();  // + comp_h: clear of the board parts
}
else {
  color("SteelBlue", 0.9) base();
  color("Orange", 0.6) translate([0, 0, base_h + 8]) lid();
  color("Green", 0.5) translate([pcb_x0, pcb_y0, floor_t + standoff_h]) cube([pcb_w, pcb_h, pcb_t]);
}
