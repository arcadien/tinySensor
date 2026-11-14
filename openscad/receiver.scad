// tinyDecoders receiver desk enclosure: Pro Micro, RXB6, 0.91" OLED 128x32.
// Units: mm. Material: PLA/PETG.
//
// Screen in a face tilted towards the user, Pro Micro micro-USB at the rear
// (5 V supply + RFLink lines), helical antenna inside, knock-out SMA hole
// at the rear for a socket and a whip antenna.
// Pro Micro standing on its pins (8 mm, plastic included) planted in two drilled rails: removable.
// RXB6 flipped (components underneath, pin stubs up), across the box above the
// Pro Micro, shield resting on two supports on either side of the Pro Micro, ergots + a drop of glue.
// Dimensions marked "TO MEASURE": estimated values.
//
// Parts (printable without support):
//   shell: shell, top on the bed (face tilted 40° from vertical)                  x1
//   plate: screwed bottom plate (4 screws), Pro Micro rails, RXB6 supports         x1
// Export:
//   openscad -D 'part="shell"' -o receiver_shell.stl receiver.scad
//   openscad -D 'part="plate"' -o receiver_plate.stl receiver.scad
//
// Frame: X width (left -> right), Y depth (face Y = 0 -> rear), Z up, top of the plate at Z = 0.

/* [Render] */
part = "assembly"; // [assembly, open, cutaway, shell, plate]

/* [Shell] */
box_w = 48;        // width: the screen (38 mm) + walls and clearance
box_d = 52;        // depth: Pro Micro against the rear wall
box_h = 26;        // height at the rear: Pro Micro on its pins, RXB6 above, helical antenna lying down
front_h = 10;      // height of the bottom of the face
face_tilt = 40;    // face: tilt from vertical (40°: readable seated, printable upside down)
wall = 2;
corner_r = 2;      // profile rounding

/* [Plate] */
plate_t = 2;
plate_clr = 0.2;   // clearance of the plate recessed in the shell
boss_d = 6;        // 4 screw bosses in the corners
boss_h = 8;
screw_pilot = 2.5; // M3 machine screw tapping the boss (drill to 2.8 if it binds)
screw_clear = 3.4;
screw_head = [5.4, 3.1]; // cylinder head (diameter, height) measured, recessed in the plate
screw_l = 6;       // M3x6: an M3x8 would need taller bosses, which would reach the screen
pad_floor = 1.2;   // material under the head
pocket_clr = 0.4;  // clearance around the screen in its pocket

/* [Pro Micro] */
pm_l = 33.4;       // measured
pm_w = 18.1;
pm_t = 1.6;
pm_comp = 1.74;    // components above the board, socket excluded (measured: 3.34 with the board)
pm_pin = 8;        // pins under the board, plastic included (measured)
pm_spacer = 2.5;   // pin plastic under the board
pm_row = 15.24;    // spacing of the two rows (0.6")
pm_pitch = 2.54;
pm_pins = 12;      // pins per row
pm_pin_off = 1.5 + (12 - 1) * 2.54 / 2 - 33.4 / 2; // offset of the rows towards the rear (+Y) / board centre: 1st pin 1.5 from the front end (measured)
usb_port = [8, 3]; // micro-USB socket (width, measured height), on top of the board
usb_over = 1.7;    // socket overhang beyond the board edge (measured)
usb_skin = 0.5;    // wall left in front of the socket face: the plug metal goes through, the overmould stays outside
usb_hole = [9, 4]; // opening around the socket (usb_port + clearance)

/* [Pro Micro rails] */
rail_h = 4;        // the pin plastic rests on the rail
rail_w = 3;
rail_hole = 1.2;   // 0.64 square pins; prints closed up: drill to 0.8 (1st print, 2026-11-14)
rail_sink = 1.5;   // holes continued into the plate

/* [RXB6] */
rx_l = 43.1;       // measured; flipped, components underneath, DATA end on the left (low X), ANT end on the right
rx_w = 11.7;
rx_t = 1.0;
rx_y = 31.3;       // front edge of the board: same place over the Pro Micro pins as v1 (29.5 + the 1.8 the board moved back)
rx_gap = 1;        // clearance between the Pro Micro components and the shield
rx_sh = [21, 10, 3.7]; // shield (measured)
rx_sh_y0 = 0.8;    // front edge of the board -> shield (measured)
rx_sh_x0 = 11.7;   // DATA end -> shield (measured)
rx_q = [9, 4.45, 4.9]; // crystal lying flush with the rear edge, DATA end, from 2 to 11 (measured)
rx_q_x0 = 2;       // DATA end -> crystal
sup_bite = 1;      // minimum reach of a support under the shield
ergot = [1, 1.5];  // ergot beyond the shield end (thickness, height above the support)
ant = [27.5, 5];   // helical antenna (length, diameter) measured, lying above the stubs

/* [OLED 0.91" 128x32] */
oled_l = 38;       // board (measured)
oled_w = 12;
oled_t = 1.2;
oled_glass = 1.9;  // glass + protective film above the board (total thickness measured 3.1)
oled_aa = [22.38, 5.58]; // active area (maker drawing)
oled_aa_off = 0.56; // active area offset to the right (side without pins): glass measured 6.27 from the pin-side edge, active area 2.1 from the glass edge (drawing)
oled_aa_off_y = 0.78; // offset towards the top of the face (pins on the left): active area 2.43 from the top edge of the board (drawing)
oled_pin_band = 3.5; // band of the pin solder joints from the left edge of the board (pins 1.5 from the edge)
oled_pin_relief = 0.5; // extra recess above the solder joints
oled_recess = 1.2; // glass below the outer face (1st print: 2.8, the screen looked sunk)
win_m = 1;         // window margin around the active area
win_bevel = oled_recess; // 45° window bevel on the sides and top: the whole thickness above the glass
oled_pad = 3;      // face thickening around the screen pocket

/* [SMA socket] */
sma_d = 6.5;       // socket hole (flat at 5.9) — TO CHECK
sma_flat = 5.9;
sma_skin = 0.4;    // skin left on the outside: drill through for the whip
sma_pos = [38, 20]; // X, Z on the rear wall (above the RXB6)

$fn = 48;
eps = 0.01;

// ---------- derived ----------
face_dy = (box_h - front_h) * tan(face_tilt);   // setback of the top of the face
face_a = 90 - face_tilt;                          // face angle from horizontal
face_len = (box_h - front_h) / cos(face_tilt);
profile = [[0, -plate_t], [box_d, -plate_t], [box_d, box_h], [face_dy, box_h], [0, front_h]];   // walls extended below Z = 0: the plate recesses into them
cb_h = screw_head[1] + 0.2;                       // head counterbore from the underside of the plate
pad_h = cb_h + pad_floor - plate_t;               // plate pad under each boss
pilot_top = -plate_t + cb_h + screw_l + 0.5;      // screw tip + 0.5
pad_d = screw_head[0] + 0.4 + 2.4;                // plate pad: counterbore + 1.2 of wall
boss_in = wall + plate_clr + pad_d / 2;           // bosses moved inward: whole plate pads, not clipped by the walls
bosses = [for (x = [boss_in, box_w - boss_in], y = [max(boss_in, wall + boss_d / 2 + 1.5), box_d - boss_in]) [x, y]];
rx_x = box_w / 2;                                 // RXB6 centred between the walls
rx_x0 = rx_x - rx_l / 2;                          // DATA end
sh_x0 = rx_x0 + rx_sh_x0;                         // start of the shield
sh_x1 = sh_x0 + rx_sh[0];                         // end of the shield
pm_x = (sh_x0 + sh_x1) / 2;                       // Pro Micro centred under the shield: supports on either side
pm_y1 = box_d - usb_skin - usb_over;              // rear edge of the board: socket face usb_skin from the outer face
pm_y = pm_y1 - pm_l / 2;
pm_z = rail_h + pm_spacer;                        // underside of the board
usb_port_z = pm_z + pm_t + usb_port[1] / 2;
win_drop = win_bevel * tan(face_tilt);            // bottom window edge horizontal (world): prints as a flat bridge, opens outwards like the others
rx_z = pm_z + pm_t + pm_comp + rx_gap;            // underside of the RXB6 shield
rx_bz = rx_z + rx_sh[2];                          // component face of the RXB6 board
ant_z = rx_bz + rx_t + 2.5 + ant[1] / 2;          // antenna axis: 2.5 of stubs + wires under it
sup_l = [sh_x0 - 0.3 - ergot[0], pm_x - pm_w / 2 - 0.3];   // left support (X)
sup_r = [pm_x + pm_w / 2 + 0.3, sh_x1 + 0.3 + ergot[0]];   // right support (X)

// face frame: origin at the bottom of the outer face, Y along the slope, Z outwards
module face_xf() { translate([0, 0, front_h]) rotate([face_a, 0, 0]) children(); }

// ---------- shell ----------
module body(off = 0) {   // off: inset (inside); every edge rounded (convex profile: fast minkowski)
  r = max(corner_r - off, 0.5);
  a = off + r;           // core inset by a, grown back by r: net inset off
  translate([a, 0, 0]) minkowski() {
    rotate([90, 0, 90]) linear_extrude(box_w - 2 * a) offset(delta = -a) polygon(profile);
    sphere(r, $fn = 24);
  }
}

module shell() {
  difference() {
    union() {
      difference() {
        body();
        // interior open at the bottom
        union() { body(wall); translate([wall, wall, -plate_t - 1]) cube([box_w - 2 * wall, box_d - 2 * wall, front_h / 2 + plate_t + 1]); }
      }
      // screw bosses
      for (p = bosses) {   // resting on the plate pad, solid up to both walls of its corner (no free-standing boss)
        c = [p[0] < box_w / 2 ? wall - 0.5 : box_w - wall + 0.5, p[1] < box_d / 2 ? wall - 0.5 : box_d - wall + 0.5];
        hull() {
          translate([p[0], p[1], pad_h]) cylinder(d = boss_d, h = boss_h - pad_h);
          translate([min(c[0], p[0]), min(c[1], p[1]), pad_h]) cube([abs(p[0] - c[0]), abs(p[1] - c[1]), boss_h - pad_h]);
        }
      }
      // blending cones: each boss rises to its wall corner at 45°, no overhang when printing (shell upside down)
      for (p = bosses) {
        c = [p[0] < box_w / 2 ? wall : box_w - wall, p[1] < box_d / 2 ? wall : box_d - wall];   // inner corner
        intersection() {
          body();
          hull() {
            translate([p[0], p[1], boss_h - eps]) cylinder(d = boss_d, h = eps);
            translate([c[0] - (c[0] == wall ? wall : 0.5), c[1] - (c[1] == wall ? wall : 0.5), boss_h + norm([p[0] - c[0], p[1] - c[1]]) + boss_d / 2])
              cube([wall + 0.5, wall + 0.5, eps]);
          }
        }
      }
      // face thickening around the screen pocket
      intersection() {
        body();
        face_xf() translate([box_w / 2 - oled_l / 2 - 2, face_len / 2 - oled_w / 2 - 2, -wall - oled_pad]) cube([oled_l + 4, oled_w + 4, oled_pad + 1]);
      }
    }
    for (p = bosses) translate([p[0], p[1], pad_h - eps]) cylinder(d = screw_pilot, h = pilot_top - pad_h);
    // screen window and board pocket (glass flush under the face)
    face_xf() {
      translate([box_w / 2 + oled_aa_off - oled_aa[0] / 2 - win_m, face_len / 2 + oled_aa_off_y - oled_aa[1] / 2 - win_m, -wall - 1]) cube([oled_aa[0] + 2 * win_m, oled_aa[1] + 2 * win_m, wall + 2]);
      // 45° bevel on the sides and the top; bottom edge horizontal in the world (win_drop): a flat bridge when
      // printing upside down, where a 45° bevel would be a near-flat sagging ceiling
      hull() {
        translate([box_w / 2 + oled_aa_off - oled_aa[0] / 2 - win_m, face_len / 2 + oled_aa_off_y - oled_aa[1] / 2 - win_m, -oled_recess]) cube([oled_aa[0] + 2 * win_m, oled_aa[1] + 2 * win_m, eps]);
        translate([box_w / 2 + oled_aa_off - oled_aa[0] / 2 - win_m - win_bevel, face_len / 2 + oled_aa_off_y - oled_aa[1] / 2 - win_m - win_drop, 0]) cube([oled_aa[0] + 2 * win_m + 2 * win_bevel, oled_aa[1] + 2 * win_m + win_bevel + win_drop, 1]);
      }
      translate([box_w / 2 - oled_l / 2 - pocket_clr, face_len / 2 - oled_w / 2 - pocket_clr, -wall - oled_pad - 1]) cube([oled_l + 2 * pocket_clr, oled_w + 2 * pocket_clr, wall + oled_pad + 1 - oled_recess + eps]);
      // groove for the pin solder joints (on the left, almost as high as the glass): deeper than the pocket
      translate([box_w / 2 - oled_l / 2 - pocket_clr, face_len / 2 - oled_w / 2 - pocket_clr, -oled_recess - eps]) cube([oled_pin_band + pocket_clr, oled_w + 2 * pocket_clr, oled_pin_relief + eps]);
    }
    // micro-USB socket: opening fitted to the socket
    translate([pm_x - usb_hole[0] / 2, box_d - wall - 1, usb_port_z - usb_hole[1] / 2]) cube([usb_hole[0], wall + 2, usb_hole[1]]);
    // knock-out SMA socket: hole with a flat, skin on the outside
    translate([sma_pos[0], box_d - wall - 1, sma_pos[1]]) rotate([-90, 0, 0]) intersection() {
      cylinder(d = sma_d, h = wall + 1 - sma_skin);
      translate([-sma_d / 2, -sma_flat / 2, 0]) cube([sma_d, sma_flat, wall + 1]);
    }
  }
}

// ---------- plate ----------
module plate() {
  difference() {
    union() {
      // plate recessed between the walls, flush with the bottom of the shell
      translate([wall + plate_clr, wall + plate_clr, -plate_t]) linear_extrude(plate_t) offset(r = 0.5) offset(delta = -0.5) square([box_w - 2 * (wall + plate_clr), box_d - 2 * (wall + plate_clr)]);
      // rails: the Pro Micro pins plug in, their plastic rests on top
      for (sx = [-1, 1]) translate([pm_x + sx * pm_row / 2 - rail_w / 2, pm_y + pm_pin_off - (pm_pins - 1) * pm_pitch / 2 - rail_w / 2, 0])
        cube([rail_w, (pm_pins - 1) * pm_pitch + rail_w, rail_h]);
      // pads under the bosses: the screw head sinks into them
      intersection() {
        for (p = bosses) translate([p[0], p[1], 0]) cylinder(d = pad_d, h = pad_h);
        translate([wall + plate_clr, wall + plate_clr, 0]) cube([box_w - 2 * (wall + plate_clr), box_d - 2 * (wall + plate_clr), pad_h]);
      }
      // RXB6 supports: the shield rests on them, an ergot beyond each end of the shield (+ a drop of glue)
      // left: in the crystal-free band (front of the board)
      translate([sup_l[0], rx_y + 0.5, 0]) cube([sup_l[1] - sup_l[0], rx_w - rx_q[1] - 1, rx_z]);
      translate([sup_l[0], rx_y + 0.5, 0]) cube([ergot[0], rx_w - rx_q[1] - 1, rx_z + ergot[1]]);
      translate([sup_r[0], rx_y + 0.5, 0]) cube([sup_r[1] - sup_r[0], rx_w - 1, rx_z]);
      translate([sup_r[1] - ergot[0], rx_y + 0.5, 0]) cube([ergot[0], rx_w - 1, rx_z + ergot[1]]);
    }
    for (sx = [-1, 1], i = [0 : pm_pins - 1])
      translate([pm_x + sx * pm_row / 2, pm_y + pm_pin_off + (i - (pm_pins - 1) / 2) * pm_pitch, -rail_sink]) cylinder(d = rail_hole, h = rail_h + rail_sink + 1, $fn = 12);
    for (p = bosses) translate([p[0], p[1], -plate_t - 1]) {
      cylinder(d = screw_clear, h = plate_t + pad_h + 2);
      cylinder(d = screw_head[0] + 0.4, h = cb_h + 1);   // counterbore: recessed head
    }
  }
}

// ---------- modules (gauges) ----------
module promicro() {
  translate([pm_x - pm_w / 2, pm_y1 - pm_l, pm_z]) {
    color("#1a5fa0") cube([pm_w, pm_l, pm_t]);
    color("#222") translate([4, 8, pm_t]) cube([10, 10, pm_comp]);
    color("Silver") translate([pm_w / 2 - usb_port[0] / 2, pm_l - 5 + usb_over, pm_t]) cube([usb_port[0], 5, usb_port[1]]);
  }
  for (sx = [-1, 1]) color("#222") translate([pm_x + sx * pm_row / 2 - 1.27, pm_y + pm_pin_off - pm_pins * pm_pitch / 2, pm_z - pm_spacer]) cube([2.54, pm_pins * pm_pitch, pm_spacer]);
}
module rxb6() {
  color("#2a7a3a") translate([rx_x0, rx_y, rx_bz]) cube([rx_l, rx_w, rx_t]);
  color("Silver") translate([sh_x0, rx_y + rx_sh_y0, rx_z]) cube([rx_sh[0], rx_sh[1], rx_sh[2]]);
  color("Silver") translate([rx_x0 + rx_q_x0, rx_y + rx_w - rx_q[1], rx_bz - rx_q[2]]) cube([rx_q[0], rx_q[1], rx_q[2]]);
  color("Gold") translate([rx_x + rx_l / 2 - ant[0], rx_y + rx_w / 2, ant_z]) rotate([0, 90, 0]) cylinder(d = ant[1], h = ant[0]);   // helical lying down, ANT side
}
module oled() {
  face_xf() translate([box_w / 2 - oled_l / 2, face_len / 2 - oled_w / 2, -oled_recess - oled_t - oled_glass]) {
    color("#1a1a1a") cube([oled_l, oled_w, oled_t]);
    color("#203060") translate([oled_l / 2 + oled_aa_off - oled_aa[0] / 2 - 3, 0.5, oled_t]) cube([oled_aa[0] + 6, oled_w - 1, oled_glass]);
  }
}

module assembled() {
  color("#e8e2d0", 0.45) shell();
  color("#c8c0a8") plate();
  promicro(); rxb6(); oled();
}

assert(rx_l + 0.6 <= box_w - 2 * wall, "RXB6 too long between the walls");
assert(pm_pin - pm_spacer <= rail_h + rail_sink, "Rail holes too short for the pins");
assert(sup_l[1] - sh_x0 >= sup_bite && sh_x1 - sup_r[0] >= sup_bite, "RXB6 supports outside the shield: Pro Micro wider than the shield");
assert(rx_bz - rx_q[2] > 0, "Crystal on the plate");
assert(ant_z + ant[1] / 2 < box_h - wall, "Antenna against the top");
assert(pilot_top <= boss_h, "Screw too long for the boss");
assert(usb_port_z + usb_hole[1] / 2 < box_h - wall, "USB socket too high");
assert(pm_y1 < box_d - wall - 0.1, "Pro Micro board into the rear wall");
assert(oled_recess - oled_pin_relief >= 0.6, "Face skin too thin over the OLED pin joints");
echo(str("Enclosure ", box_w, " x ", box_d, " x ", box_h + plate_t, " mm; face at ", face_a, "° from horizontal, ", round(face_len), " mm long"));

if (part == "shell") translate([0, 0, box_h]) rotate([180, 0, 0]) shell();   // top on the bed
else if (part == "plate") translate([0, 0, plate_t]) plate();
else if (part == "open") {   // open view: shell lifted above the plate and the boards
  translate([0, 0, 40]) color("#e8e2d0") shell();
  color("#c8c0a8") plate();
  promicro(); rxb6();
  translate([0, 0, 40]) oled();
}
else if (part == "cutaway") {   // shell cut at mid-width: inner layout
  color("#e8e2d0") difference() { shell(); translate([-1, -1, -1]) cube([box_w / 2 + 1, box_d + 2, box_h + 2]); }
  color("#c8c0a8") plate();
  promicro(); rxb6(); oled();
}
else assembled();
