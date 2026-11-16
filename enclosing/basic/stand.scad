// Table stand: box + clipped radiation shield, standing on a table.
// Units: mm. Material: PETG.
//
// The mast stands in for the wall: the box screws onto it by its two wall tabs
// (M4 countersunk + nuts sunk in the back of the mast), the shield stays clipped under
// the bottom tab as on the wall. The mast fits into the foot (1 M4 screw).
//
// Parts (printable without support):
//   mast: mast, flat, back on the bed                      x1
//   foot: foot with socket, underside on the bed            x1
// Export:
//   openscad -D 'part="mast"' -o stand_mast.3mf stand.scad
//   openscad -D 'part="foot"' -o stand_foot.3mf stand.scad
//
// Frame = the one of tinysensor_box.scad (X right, Y up, Z out of the wall, wall at Z = 0).

/* [Render] */
part = "assembly"; // [mast, foot, assembly]

/* [Copied from tinysensor_box.scad / sensor_shield.scad — keep aligned] */
cav_w = 95.98;
ear_top = 102.154;              // end of the top tab (cav_h + wall + flange + ear_l)
ear_holes_y = [96.154, -16.4];  // axes of the tabs' wall screw holes
shield_bottom = -80;            // underside of the clipped shield (oy of assembly.scad)
shield_front = 72;              // front of the shield (wall_y + clip_t + plate_d / 2)

/* [Mast] */
mast_w = 30;
mast_t = 6;
under_clr = 30;    // clearance under the shield: loop of the cable coming up through the base

/* [Foot] */
foot_w = 130;      // width (X)
foot_back = 40;    // overhang behind the mast
foot_front = 90;   // overhang in front of the wall (the shield reaches shield_front)
foot_t = 4;
foot_r = 10;
socket_h = 25;     // mast insertion depth
socket_wall = 3;
fit = 0.3;         // mast / socket clearance
gusset_t = 3;

/* [Screws] */
screw_d = 4.4;     // M4
nut_af = 8.4;      // M4 nut (7.9 across flats + clearance)
nut_h = 3.4;

$fn = 48;
eps = 0.01;

cx = cav_w / 2;
y_bot = shield_bottom - under_clr;   // mast bottom = top of the foot
y_floor = y_bot - foot_t;             // table
mast_l = ear_top - y_bot;
socket_screw_y = y_bot + socket_h / 2;

module mast_use() {
  difference() {
    translate([cx - mast_w / 2, y_bot, -mast_t]) cube([mast_w, mast_l, mast_t]);
    // box tab screws, nuts sunk in the back
    for (y = ear_holes_y) translate([cx, y, -mast_t - eps]) {
      cylinder(d = screw_d, h = mast_t + 2 * eps);
      cylinder(d = nut_af / cos(30), h = nut_h, $fn = 6);
    }
    // socket screw
    translate([cx, socket_screw_y, -mast_t - eps]) cylinder(d = screw_d, h = mast_t + 2 * eps);
  }
}

module foot_use() {
  sx = mast_w / 2 + fit + socket_wall;   // outer half-width of the socket
  z0 = -mast_t - fit - socket_wall;      // back of the socket
  z1 = fit + socket_wall;                // front face of the socket
  difference() {
    union() {
      // foot
      translate([0, y_floor, 0]) rotate([-90, 0, 0]) linear_extrude(foot_t)
        translate([cx - foot_w / 2, -foot_front]) offset(r = foot_r) offset(delta = -foot_r)
          square([foot_w, foot_front + mast_t + foot_back]);
      // socket
      translate([cx - sx, y_bot - eps, z0]) cube([2 * sx, socket_h + eps, z1 - z0]);
      // front / rear gussets (YZ plane) and side ones (XY plane)
      for (g = [[z1, z1 + 30], [z0, z0 - 25]])
        hull() {
          translate([cx - gusset_t / 2, y_bot - eps, g[0]]) cube([gusset_t, socket_h, eps]);
          translate([cx - gusset_t / 2, y_bot - eps, g[1]]) cube([gusset_t, eps, eps]);
        }
      for (s = [-1, 1])
        hull() {
          translate([cx + s * sx - (s > 0 ? eps : 0), y_bot - eps, -mast_t / 2 - gusset_t / 2]) cube([eps, socket_h, gusset_t]);
          translate([cx + s * (sx + 25) - (s > 0 ? eps : 0), y_bot - eps, -mast_t / 2 - gusset_t / 2]) cube([eps, eps, gusset_t]);
        }
    }
    // mast pocket
    translate([cx - mast_w / 2 - fit, y_bot, -mast_t - fit]) cube([mast_w + 2 * fit, socket_h + eps, mast_t + 2 * fit]);
    // through screw M4 x 20 + nut
    translate([cx, socket_screw_y, z0 - eps]) cylinder(d = screw_d, h = z1 - z0 + 2 * eps);
  }
}

if (part == "mast") translate([0, 0, mast_t]) mast_use();          // back on the bed
else if (part == "foot") translate([0, 0, -y_floor]) rotate([90, 0, 0]) foot_use(); // use Y -> Z
else {
  color("Peru") mast_use();
  color("Sienna") foot_use();
}
