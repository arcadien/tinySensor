// TinySensor v2.2 — screwed wall box with seal
// Units: mm. Target material: PETG.
//
// Export:
//   openscad -D 'part="base"' -o base.3mf tinysensor_box.scad
//   openscad -D 'part="lid"'  -o lid.3mf  tinysensor_box.scad
// Open-box render (board + cell, lid turned over next to it):
//   openscad --preview -D 'part="open"' --camera=110,-230,420,110,30,0 --imgsize=1400,1000 -o rendu_ouvert.png tinysensor_box.scad
//
// Frame: origin = "top-left" corner of the PCB seen from above (as in KiCad),
// X to the right, Y down (KiCad) turned into Y up here.
// Wall-mounted, the "bottom" side (Y=0, 18650 holder side) faces the ground.

/* [Render] */
part = "assembly"; // [base, lid, assembly, open]
usage = "ext";     // [int, ext]
gland_side = "left"; // [left, right, bottom] cable gland wall (usage ext)

/* [PCB — from hardware/tinysensor/tinySensorv1.kicad_pcb] */
pcb_w = 93.98;
pcb_h = 63.754;
pcb_t = 1.6;
// M2.5 mounting holes (4 corners), KiCad coordinates relative to the PCB corner
pcb_holes_kicad = [[89.96, 3.97], [5.01, 4.79], [4.57, 57.40], [90.71, 57.48]];

/* [Volumes] */
clr = 1.0;            // PCB / wall clearance
bay = 14;             // cable bay on the bottom side (cable gland, antenna)
standoff_h = 5;       // height under the PCB (THT legs)
comp_h = 25;          // tallest component above the PCB (large capacitors)
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
pcb_screw_d_pilot = 2.1;  // M2.5 self-tapping
standoff_d = 6;
gland_d = 12.5;       // PG7
wall_screw_d = 4.5;   // Ø4 wall screw
ear_l = 12;           // tab overhang beyond the flange
shield_notch = true;  // notches to clip the radiation shield under the box
shield_notch_y = 8;   // notch position from the end of the bottom tab

use <pcb.scad>
use <euro2.scad>

$fn = 48;
eps = 0.01;
// Side cable gland: in the bay (below the PCB in Y), high enough for the
// inner nut (PG7, 15 across flats) to clear the PCB edge, and
// under the flange chamfer that keeps the rain off.
gland_z = gland_side == "bottom" ? 11.5 : 15;
gland_x = 22;        // position in the bottom wall: offset from the central wall tab
gland_y = 8;         // position in a side wall: middle of the bay

// ---------- derived dimensions ----------
cav_w = pcb_w + 2 * clr;
cav_h = pcb_h + 2 * clr + bay;
cav_d = standoff_h + pcb_t + comp_h + top_clr;
base_h = floor_t + cav_d;
pcb_x0 = clr;                // PCB corner in the cavity frame
pcb_y0 = clr + bay;
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
  translate([pcb_x0, pcb_y0, floor_t + standoff_h]) pcb_v22();
  // lid turned over next to it, inner face (rib) visible
  color("Gray") translate([cav_w + 2 * (wall + flange) + 15, 0, lid_t]) mirror([0, 0, 1]) lid();
  translate([cav_w / 2, -wall - flange - 25, 0]) euro2();
}
else {
  color("SteelBlue", 0.9) base();
  color("Orange", 0.6) translate([0, 0, base_h + 8]) lid();
  color("Green", 0.5) translate([pcb_x0, pcb_y0, floor_t + standoff_h]) cube([pcb_w, pcb_h, pcb_t]);
}
