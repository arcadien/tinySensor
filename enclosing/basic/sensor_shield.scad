// Outdoor radiation shield (stacked-plate Stevenson type) for a BMx280 breakout
// Units: mm. Material: PETG (white preferably: less solar heating).
//
// Parts (all printable without support, top face on the bed):
//   cap  : solid cap + arm + wall plate                  x1
//   plate: louvred plate                                 x n_plates
//   base : base with sensor slide and cable pass         x1
//   clip : (option) adapter to hang the shield under the main box  x1
//          bolted behind the wall plate (2 M4x8 countersunk screws + sunk
//          nuts), it clips onto the bottom wall tab of the box, which carries
//          two side notches (tinysensor_box.scad, shield_notch = true).
// Export:
//   openscad -D 'part="cap"'   -o shield_cap.3mf   sensor_shield.scad
//   openscad -D 'part="plate"' -o shield_plate.3mf sensor_shield.scad
//   openscad -D 'part="base"'  -o shield_base.3mf  sensor_shield.scad
//
// Assembly: 3 M3 threaded rods (or long M3 screws) through the spacers,
// nuts under the base. Fix the cap to the wall BEFORE stacking the plates.

/* [Render] */
part = "assembly"; // [cap, plate, base, clip, assembly]

/* [Plates] */
n_plates = 4;      // louvred plates between cap and base
plate_d = 56;      // outer diameter
hole_d = 26;       // diameter of the central column (sensor slide ~19.4 wide)
plate_t = 1.6;
skirt_h = 5;       // skirt: stops slanting rain and radiation
skirt_t = 1.6;
gap = 10;          // vertical pitch between two plates (spacer height)

/* [Assembly] */
post_r = 20;       // radius of the spacer circle
post_d = 7;
rod_d = 3.4;       // M3 clearance

/* [Wall mounting] */
wall_gap = 12;     // skirt / wall distance
bracket_w = 40;
bracket_t = 4;
wall_screw_d = 4.5;

/* [Clip under the box — must match the bottom tab of tinysensor_box.scad] */
ear_t = 4;          // tab thickness (floor_t + 1.6)
ear_w = 20;
notch_y = 8;        // notch from the end of the tab (shield_notch_y)
clip_gap = 6;       // vertical gap between tab end and top of the cap
clip_engage = 16;   // tab length held in the clip
clip_t = 4;         // adapter thickness
prong_w = 2.4;      // prong width (flex)
lip = 3;            // overlap on the front face of the tab
bump_r = 0.8;       // bump engaging the notch (r = 1)

/* [Sensor — 15 x 11 mm breakout, 1.6 mm PCB] */
sensor_w = 15;
sensor_pcb_t = 1.6;
rail_h = 16;
cable_d = 6;

$fn = 96;
eps = 0.01;

stack_h = (n_plates + 1) * gap + plate_t;   // from the underside of the base to the top of the cap
wall_y = plate_d / 2 + wall_gap;            // wall plane (back of the wall plate)

module posts(h) {
  for (a = [90, 210, 330]) rotate([0, 0, a]) translate([post_r, 0, 0]) cylinder(d = post_d, h = h);
}
module rod_holes(h) {
  for (a = [90, 210, 330]) rotate([0, 0, a]) translate([post_r, 0, -eps]) cylinder(d = rod_d, h = h + 2 * eps);
}

// Plate in its print orientation: top (in use) on the bed, skirt and spacers towards +Z
module plate_print(solid = false) {
  difference() {
    union() {
      cylinder(d = plate_d, h = plate_t);
      difference() {
        cylinder(d = plate_d, h = skirt_h);
        translate([0, 0, -eps]) cylinder(d = plate_d - 2 * skirt_t, h = skirt_h + 2 * eps);
      }
      posts(gap);
    }
    if (!solid) translate([0, 0, -eps]) cylinder(d = hole_d, h = plate_t + 2 * eps);
    rod_holes(gap);
  }
}

module cap_print() {
  plate_print(solid = true);
  // arm towards the wall (on the bed) + vertical wall plate (towards +Z = downwards in use)
  difference() {
    union() {
      translate([-bracket_w / 2, 0, 0]) cube([bracket_w, wall_y, plate_t * 2]);
      translate([-bracket_w / 2, wall_y - bracket_t, 0]) cube([bracket_w, bracket_t, stack_h - plate_t]);
      // gussets
      for (x = [-bracket_w / 2, bracket_w / 2 - 3])
        hull() {
          translate([x, plate_d / 2 - 2, 0]) cube([3, eps, plate_t * 2]);
          translate([x, wall_y - bracket_t, 0]) cube([3, eps, 20]);
        }
    }
    for (z = [12, stack_h - plate_t - 12])
      translate([0, wall_y + eps, z]) rotate([90, 0, 0]) {
        cylinder(d = wall_screw_d, h = bracket_t + 2 * eps);
        cylinder(d1 = wall_screw_d, d2 = wall_screw_d + 4.4, h = 2.2); // countersink on the sensor side
      }
  }
}

// Base in its use orientation (= print orientation)
module base_print() {
  difference() {
    cylinder(d = plate_d, h = plate_t);
    rod_holes(plate_t * 2);
    // cable pass under the sensor
    translate([0, 0, -eps]) cylinder(d = cable_d, h = plate_t * 2);
    // drainage
    for (a = [30 : 60 : 330]) rotate([0, 0, a]) translate([hole_d / 2 - 4, 0, -eps]) cylinder(d = 3, h = plate_t * 2);
  }
  // slide: two U rails holding the edges of the breakout, standing upright
  for (s = [-1, 1]) translate([s * (sensor_w / 2 + 0.2) - (s > 0 ? 0 : 2), -2.5, 0])
    difference() {
      cube([2, 5, rail_h]);
      translate([s > 0 ? -eps : 1, 2.5 - (sensor_pcb_t + 0.3) / 2, plate_t]) cube([1 + eps, sensor_pcb_t + 0.3, rail_h]);
    }
}


// Clip adapter, modelled in use coordinates then laid down for printing:
// X horizontal, Y vertical (up), Z out of the wall (back on the wall, Z = 0).
module clip_use() {
  y0 = plate_t;                     // bottom of the wall plate
  y1 = stack_h;                     // top (top of the cap)
  ye = y1 + clip_gap;               // end of the box tab
  yt = ye + clip_engage;            // top of the prongs
  zi = ear_t + 0.3;                 // inner face of the lips
  zt = zi + lip;                    // total prong thickness (45° lip)
  difference() {
    translate([-bracket_w / 2, y0, 0]) cube([bracket_w, y1 - y0, clip_t]);
    for (y = [y1 - 12, y0 + 12]) translate([0, y, -eps]) {
      cylinder(d = 4.4, h = clip_t + 2 * eps);
      cylinder(d = 8.1 + 0.3, h = 3.4, $fn = 6);  // M4 nut sunk on the wall side
    }
  }
  for (sx = [-1, 1]) {
    xi = sx * (ear_w / 2 + 0.2);    // inner face of the prong
    xo = sx * (ear_w / 2 + 0.2 + prong_w);
    // prong
    translate([min(xi, xo), y1 - 10, 0]) cube([prong_w, yt - y1 + 10, zt]);
    // 45° bevelled lip: sloped underside, printable back on the bed;
    // it presses on the front edge of the tab and holds it against the wall
    hull() {
      translate([xi - (sx > 0 ? eps : 0), ye, zi]) cube([eps, clip_engage, zt - zi]);
      translate([xi - sx * lip - (sx > 0 ? eps : 0), ye, zt - eps]) cube([eps, clip_engage, eps]);
    }
    // snap bump
    translate([xi, ye + notch_y, 0]) cylinder(r = bump_r, h = zi);
  }
}
module clip_print() { clip_use(); }  // back on the bed: use Z = print Z

if (part == "cap") cap_print();
else if (part == "plate") plate_print();
else if (part == "base") base_print();
else if (part == "clip") clip_print();
else {
  // overview in use position (Z up)
  color("White") base_print();
  for (i = [1 : n_plates]) color("WhiteSmoke") translate([0, 0, i * gap + plate_t]) mirror([0, 0, 1]) plate_print();
  color("Gainsboro") translate([0, 0, (n_plates + 1) * gap + plate_t]) mirror([0, 0, 1]) cap_print();
  color("Purple") translate([-sensor_w / 2, -0.8, plate_t]) cube([sensor_w, 1.6, 11]);
}
