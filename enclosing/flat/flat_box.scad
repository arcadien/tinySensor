// TinySensor v3.0 — flat indoor box for the CR2032 build
// Units: mm. Target material: PETG (PLA fine indoors). Status: first draft, not printed, not validated.
//
// Export:
//   openscad -D 'part="base"' -o base.stl flat_box.scad
//   openscad -D 'part="lid"'  -o lid.stl  flat_box.scad
//   (-D 'sensor="ds18b20"' for the DS18B20 variant)
// Open-box render (board, radio, sensor, lid turned over next to it):
//   openscad --preview -D 'part="open"' --camera=0,0,0,40,0,15,0 --viewall --autocenter --imgsize=1400,1000 -o rendu_ouvert.png flat_box.scad
// Exploded isometric view:
//   openscad --preview -D 'part="exploded"' --camera=0,0,0,55,0,25,0 --viewall --autocenter --projection=o --imgsize=1400,1000 -o rendu_eclate.png flat_box.scad
//
// Assumptions (parameters below):
// - Indoor thermometer: no seal, no louvres, plain vent slots next to the sensor (wall + lid).
// - Full (uncut) v3.0 board, powered by the CR2032 in J10. No module on J2 (JP2 closed); J13, J4, J7, J8, JP1
//   (LED off) and the J6 ISP header not fitted. C1 = 220 µF low-leakage, standing: it cannot be laid over
//   (C2, C5, C7 and SW1 around it, all >= 4.3 mm high), so C1 (11.5 mm) sets comp_h.
// - 433 MHz module soldered straight in J1, pins bent 90°, lying flat over U1 (component side up), clear of
//   R9/R10 (5.3 mm). Antenna: 17.3 cm wire in the groove along the inner wall (antenna_groove).
// - Sensor soldered straight to the board, no chamber (self-heating negligible, the board sleeps):
//     bme280: breakout on its 6-pin header in the J3 holes (J3/J14 JSTs not fitted), lying flat towards the
//             board edge over the empty J14 footprint; vents in the bottom and right walls and the lid above it.
//     ds18b20: TO-92 in J5, legs bent so it lies flat, flat face down, past the top board edge (MH side);
//              vents in the top wall and the lid above it.
// - Lid flush with the walls, on 4 M3 x 6 self-tapping screws in pillars inside the cavity corners, past both board
//   ends: the CR2032 comes out without touching the board.
//
// Frame: origin = bottom-left corner of the cavity, X along the board, Y up (KiCad Y down), floor underside Z = 0.
// The board lies with its KiCad bottom edge (J3/J14 side) towards Y = 0.

/* [Render] */
part = "assembly"; // [base, lid, assembly, open, exploded]
sensor = "bme280"; // [bme280, ds18b20]
explode = 25;      // exploded view: gap between levels

/* [PCB v3.0 — from hardware/tinysensor/tinysensor.kicad_pcb] */
pcb_w = 94.81;
pcb_h = 39.334;
pcb_t = 1.6;
pcb_k0 = [113.04016, 48.02632];   // top-left corner of the outline, absolute KiCad coordinates
pcb_holes_abs = [[203, 52], [151.35016, 52.11032], [117.85016, 52.86032]];   // MH1, MH2, MH3

/* [Heights] */
standoff_h = 3;       // under the PCB: THT legs cut to 1.5-2 mm
c1_h = 11.5;          // C1 220 µF Ø6.3, tallest part on the board
radio_flat = true;    // false: module standing on J1 (FS1000A ~19 mm + 2.5)
radio_lift = 5.8;     // flat module underside above the PCB (clears R9/R10, 5.3)
radio_size = [19, 19, 1.6];   // FS1000A board
radio_parts_h = 3.5;  // tallest part on the module (SAW resonator)
radio_y0 = 54;        // KiCad Y of the module edge nearer to J5 (J1 pins at 56.86-61.94)
top_clr = 1;          // clearance under the lid

/* [Walls] */
clr = 1.0;            // PCB / wall clearance
wall = 2.0;           // 5 perimeters, 0.4 nozzle
floor_t = 2.4;        // room for the PCB screw pilots
lid_t = 2.0;
r_out = 3;

/* [Screws and fixings] */
pillar_d = 7;             // lid screw pillars in the cavity corners, merged with the walls
lid_screw_d_pilot = 2.5;  // M3 self-tapping in PETG
lid_screw_d_clear = 3.4;
lid_screw_depth = 6;
pcb_screw_d_pilot = 2.5;  // M3 x 6 self-tapping
standoff_d = 7;
wall_screw_d = 4.5;       // Ø4 countersunk wall screw
ear_l = 12;               // wall tab length beyond the end wall
ear_w = 16;

/* [Vents and antenna] */
vent_w = 1.6;
vent_pitch = 3.6;
antenna_groove = true;
antenna_groove_h = 1.2;
antenna_groove_d = 0.8;

use <../pcb.scad>
use <../euro2.scad>

$fn = 48;
eps = 0.01;

// ---------- derived dimensions ----------
is_bme = sensor == "bme280";
clr_top = is_bme ? clr : 4.5;   // ds18b20: the TO-92 sticks out 2.8 mm past the top board edge
cav_w = pcb_w + 2 * (pillar_d + clr);   // pillars past both board ends
cav_h = pcb_h + clr + clr_top;
pcb_x0 = pillar_d + clr;
pcb_y0 = clr;
z_pcb = floor_t + standoff_h;  // PCB underside
z_top = z_pcb + pcb_t;
radio_top = radio_flat ? radio_lift + radio_size[2] + radio_parts_h : 21.5;
comp_h = max(c1_h, radio_top);
base_h = z_top + comp_h + top_clr;

// KiCad absolute -> cavity frame
function k2c(p) = [pcb_x0 + p[0] - pcb_k0[0], pcb_y0 + pcb_h - (p[1] - pcb_k0[1])];
function holes() = [for (h = pcb_holes_abs) k2c(h)];

// vent zone around the sensor, KiCad coordinates (corners beyond the board = through the walls)
vent_k = is_bme ? [[185, 75], [250, 120]] : [[186, 0], [200, 53]];
va = k2c(vent_k[0]);
vb = k2c(vent_k[1]);
vent_zone = [min(va[0], vb[0]), min(va[1], vb[1]), max(va[0], vb[0]), max(va[1], vb[1])];

function lid_screws() = let(o = pillar_d / 2) [[o, o], [cav_w - o, o], [o, cav_h - o], [cav_w - o, cav_h - o]];

module rrect(w, h, r) { translate([r, r]) offset(r = r) square([w - 2 * r, h - 2 * r]); }
module ring_rect(d, r = 1) { translate([-d, -d]) rrect(cav_w + 2 * d, cav_h + 2 * d, max(r, 0.5)); }
// pillars: a circle hulled into its cavity corner
module pillars2d() { for (p = lid_screws()) hull() { translate(p) circle(d = pillar_d); translate([p[0] < cav_w / 2 ? 0 : p[0], p[1] < cav_h / 2 ? 0 : p[1]]) square(pillar_d / 2); } }
module free2d() { difference() { ring_rect(0); pillars2d(); } }
function plen(ps, i = 0) = i >= len(ps) - 1 ? 0 : norm(ps[i + 1] - ps[i]) + plen(ps, i + 1);
module vent_zone2d() { translate([vent_zone[0], vent_zone[1]]) square([vent_zone[2] - vent_zone[0], vent_zone[3] - vent_zone[1]]); }

// ---------- base ----------
module base() {
  difference() {
    union() {
      linear_extrude(base_h) ring_rect(wall, r_out);
      for (s = [-1, 1]) ear(s);
    }
    translate([0, 0, floor_t]) linear_extrude(base_h) difference() { ring_rect(0, 1); pillars2d(); }
    for (p = lid_screws()) translate([p[0], p[1], base_h - lid_screw_depth]) cylinder(d = lid_screw_d_pilot, h = lid_screw_depth + eps);
    intersection() {   // wall slots next to the sensor, clear of the pillars
      linear_extrude(base_h) vent_zone2d();
      union() {
        for (x = [pillar_d + 1 : vent_pitch : cav_w - pillar_d - 1 - vent_w], y = [-wall - eps, cav_h - eps])
          translate([x, y, floor_t + 1]) cube([vent_w, wall + 2 * eps, base_h - floor_t - 3]);
        for (y = [pillar_d + 1 : vent_pitch : cav_h - pillar_d - 1 - vent_w], x = [-wall - eps, cav_w - eps])
          translate([x, y, floor_t + 1]) cube([wall + 2 * eps, vent_w, base_h - floor_t - 3]);
      }
    }
    if (antenna_groove) translate([0, 0, base_h - 3 - antenna_groove_h]) linear_extrude(antenna_groove_h)
      difference() {   // follows the walls and skirts the pillars
        offset(r = antenna_groove_d) free2d();
        free2d();
        offset(delta = 2) vent_zone2d();
      }
  }
  for (h = holes()) translate([h[0], h[1], floor_t - eps]) difference() {
    cylinder(d = standoff_d, h = standoff_h + eps);
    cylinder(d = pcb_screw_d_pilot, h = standoff_h + 1);
  }
}

module ear(s) {   // wall tab on the left (s = -1) or right (s = 1) end
  x = s < 0 ? -wall - ear_l : cav_w;
  hx = s < 0 ? -wall - ear_l / 2 - 1.5 : cav_w + wall + ear_l / 2 + 1.5;
  difference() {
    translate([x, cav_h / 2 - ear_w / 2, 0]) cube([wall + ear_l, ear_w, floor_t + 1.6]);
    translate([hx, cav_h / 2, -eps]) {
      cylinder(d = wall_screw_d, h = 10);
      translate([0, 0, floor_t + 1.6 - 2.2]) cylinder(d1 = wall_screw_d, d2 = wall_screw_d + 4.4, h = 2.2 + 2 * eps);
    }
  }
}

// ---------- lid (inner face at Z = 0) ----------
module lid() {
  difference() {
    linear_extrude(lid_t) ring_rect(wall, r_out);
    for (p = lid_screws()) translate([p[0], p[1], -eps]) cylinder(d = lid_screw_d_clear, h = lid_t + 2 * eps);
    // slots above the sensor
    translate([0, 0, -eps]) linear_extrude(lid_t + 2 * eps) intersection() {
      offset(delta = -2) intersection() { vent_zone2d(); free2d(); }
      for (x = [0 : vent_pitch : cav_w]) translate([x, 0]) square([vent_w, cav_h]);
    }
  }
}

// ---------- fitted parts, render only (cavity frame) ----------
module board() {
  translate([pcb_x0, pcb_y0, z_pcb]) pcb_v3(jst_eh = false, bare = true);
  radio();
  if (is_bme) bme280(); else ds18b20();
}
module at_k(p, z) { c = k2c(p); translate([c[0], c[1], z]) children(); }
module radio() {
  pins = [for (i = [0 : 2]) [192.99936, 56.86044 + i * 2.54]];
  if (radio_flat) {
    // module edge 1.5 mm from the J1 pins, lying over U1 towards -X
    at_k([191.5, radio_y0], z_top + radio_lift) translate([-radio_size[0], -radio_size[1], 0]) {   // Y down in KiCad = towards Y = 0 here
      color("#1a6e3a") cube(radio_size);
      color("Silver") translate([6, 6, radio_size[2]]) cylinder(d = 8, h = radio_parts_h);   // SAW resonator
    }
    color("Gold") for (p = pins) {
      at_k(p, z_top - eps) translate([-0.32, -0.32, 0]) cube([0.64, 0.64, radio_lift + 0.8]);
      at_k(p, z_top + radio_lift + 0.5) translate([-1.82, -0.32, 0]) cube([1.82, 0.64, 0.64]);
    }
  } else at_k(pins[1], z_top + 2.5) color("#1a6e3a") translate([-0.8, -radio_size[0] / 2, 0]) cube([1.6, radio_size[0], radio_size[1]]);
  // antenna wire in the wall groove (17.3 cm from the module ANT pad)
  if (antenna_groove) color("Goldenrod") translate([0, 0, base_h - 3 - antenna_groove_h / 2]) {
    // along the wall away from the sensor, the left wall, then back along the sensor wall
    r = pillar_d / 2 + 0.4;
    o = pillar_d / 2;
    ys = is_bme ? [cav_h + 0.4, cav_h - o, o] : [-0.4, o, cav_h - o];   // start wall, first pillar, second pillar
    a = is_bme ? [90, -180, 180, -90] : [-90, 180, 180, 450];          // arc angles around the two left pillars
    arcs = concat([for (t = [0 : 8]) [o, ys[1]] + r * [cos(a[0] + t * (a[1] - a[0]) / 8), sin(a[0] + t * (a[1] - a[0]) / 8)]],
                  [for (t = [0 : 8]) [o, ys[2]] + r * [cos(a[2] + t * (a[3] - a[2]) / 8), sin(a[2] + t * (a[3] - a[2]) / 8)]]);
    head = concat([[cav_w - pillar_d - 4, ys[0]]], arcs);
    path = concat(head, [arcs[len(arcs) - 1] + [173 - plen(head), 0]]);
    for (i = [0 : len(path) - 2]) hull() for (q = [path[i], path[i + 1]]) translate(q) sphere(d = 0.8, $fn = 8);
  }
}
module bme280() {   // header in the J3 holes (2.54 pins in the 2.5 pitch: 0.1 mm per end), breakout over J14
  at_k([193.1, 77.11], z_top) {
    color("#222") translate([-7.62, -1.27, 0]) cube([15.24, 2.54, 2.5]);
    color("#5b2a86") translate([-7.5, -9.7, 2.5]) cube([15, 11, 1.6]);
    color("Silver") translate([-1.25, -8.5, 4.1]) cube([2.5, 2.5, 1]);
  }
}
module ds18b20() {   // TO-92 lying flat face down, past the top board edge
  at_k([191.49, 51.18], z_top) {
    color("#111") translate([0, 1.5, 0.8 + 1.4]) intersection() {
      rotate([-90, 0, 0]) cylinder(d = 4.8, h = 4.6);
      translate([-2.5, 0, -1.4]) cube([5, 4.6, 3.8]);
    }
    color("Silver") for (dx = [-2.54, 0, 2.54]) translate([dx - 0.25, -0.25, 0]) cube([0.5, 1.8, 2.4]);
  }
}

if (part == "base") base();
else if (part == "lid") translate([0, 0, lid_t]) rotate([180, 0, 0]) lid();   // outer face on the bed
else if (part == "open") {
  color("WhiteSmoke") base();
  board();
  color("Gainsboro") translate([cav_w + wall + ear_l + 25, cav_h, lid_t]) rotate([180, 0, 0]) lid();   // turned over on the right
  translate([cav_w / 2, -wall - 25, 0]) euro2();
}
else if (part == "exploded") {
  color("WhiteSmoke") base();
  translate([0, 0, explode]) board();
  color("Gainsboro") translate([0, 0, base_h + 2 * explode]) lid();
}
else {
  color("WhiteSmoke") base();
  board();
  color("Gainsboro", 0.5) translate([0, 0, base_h]) lid();
}
