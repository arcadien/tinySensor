// Abri de sonde extérieur (type Stevenson à assiettes) pour breakout BMx280
// Unités : mm. Matière : PETG (blanc de préférence : moins d'échauffement solaire).
//
// Pièces (toutes imprimables sans support, face supérieure sur le plateau) :
//   cap   : chapeau plein + bras + platine murale       x1
//   plate : assiette ajourée                            x n_plates
//   base  : fond avec glissière capteur et passe-câble  x1
// Export :
//   openscad -D 'part="cap"'   -o shield_cap.3mf   sensor_shield.scad
//   openscad -D 'part="plate"' -o shield_plate.3mf sensor_shield.scad
//   openscad -D 'part="base"'  -o shield_base.3mf  sensor_shield.scad
//
// Assemblage : 3 tiges filetées M3 (ou vis M3 longues) traversant les entretoises,
// écrous sous le fond. Fixer le chapeau au mur AVANT d'empiler les assiettes.

/* [Rendu] */
part = "assembly"; // [cap, plate, base, assembly]

/* [Assiettes] */
n_plates = 4;      // assiettes ajourées entre chapeau et fond
plate_d = 80;      // diamètre extérieur
hole_d = 40;       // diamètre de la colonne centrale
plate_t = 1.6;
skirt_h = 8;       // jupe : arrête la pluie oblique et le rayonnement
skirt_t = 1.6;
gap = 14;          // pas vertical entre deux assiettes (hauteur des entretoises)

/* [Assemblage] */
post_r = 30;       // rayon du cercle des entretoises
post_d = 7;
rod_d = 3.4;       // passage M3

/* [Fixation murale] */
wall_gap = 12;     // distance jupe / mur
bracket_w = 40;
bracket_t = 4;
wall_screw_d = 4.5;

/* [Capteur — breakout 15 x 11 mm, PCB 1.6 mm] */
sensor_w = 15;
sensor_pcb_t = 1.6;
rail_h = 16;
cable_d = 6;

$fn = 96;
eps = 0.01;

stack_h = (n_plates + 1) * gap + plate_t;   // du dessous du fond au dessus du chapeau
wall_y = plate_d / 2 + wall_gap;            // plan du mur (dos de la platine)

module posts(h) {
  for (a = [90, 210, 330]) rotate([0, 0, a]) translate([post_r, 0, 0]) cylinder(d = post_d, h = h);
}
module rod_holes(h) {
  for (a = [90, 210, 330]) rotate([0, 0, a]) translate([post_r, 0, -eps]) cylinder(d = rod_d, h = h + 2 * eps);
}

// Assiette dans son sens d'impression : dessus (usage) sur le plateau, jupe et entretoises vers +Z
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
  // bras vers le mur (sur le plateau) + platine murale verticale (vers +Z = vers le bas en usage)
  difference() {
    union() {
      translate([-bracket_w / 2, 0, 0]) cube([bracket_w, wall_y, plate_t * 2]);
      translate([-bracket_w / 2, wall_y - bracket_t, 0]) cube([bracket_w, bracket_t, stack_h - plate_t]);
      // goussets
      for (x = [-bracket_w / 2, bracket_w / 2 - 3])
        hull() {
          translate([x, plate_d / 2 - 2, 0]) cube([3, eps, plate_t * 2]);
          translate([x, wall_y - bracket_t, 0]) cube([3, eps, 20]);
        }
    }
    for (z = [12, stack_h - plate_t - 12])
      translate([0, wall_y + eps, z]) rotate([90, 0, 0]) {
        cylinder(d = wall_screw_d, h = bracket_t + 2 * eps);
        cylinder(d1 = wall_screw_d, d2 = wall_screw_d + 4.4, h = 2.2); // fraisage côté capteur
      }
  }
}

// Fond dans son sens d'usage (= sens d'impression)
module base_print() {
  difference() {
    cylinder(d = plate_d, h = plate_t);
    rod_holes(plate_t * 2);
    // passe-câble sous le capteur
    translate([0, 0, -eps]) cylinder(d = cable_d, h = plate_t * 2);
    // drainage
    for (a = [30 : 60 : 330]) rotate([0, 0, a]) translate([hole_d / 2 - 4, 0, -eps]) cylinder(d = 3, h = plate_t * 2);
  }
  // glissière : deux rails en U qui tiennent les bords du breakout, posé verticalement
  for (s = [-1, 1]) translate([s * (sensor_w / 2 + 0.2) - (s > 0 ? 0 : 2), -2.5, 0])
    difference() {
      cube([2, 5, rail_h]);
      translate([s > 0 ? -eps : 1, 2.5 - (sensor_pcb_t + 0.3) / 2, plate_t]) cube([1 + eps, sensor_pcb_t + 0.3, rail_h]);
    }
}

if (part == "cap") cap_print();
else if (part == "plate") plate_print();
else if (part == "base") base_print();
else {
  // vue d'ensemble en position d'usage (Z vers le haut)
  color("White") base_print();
  for (i = [1 : n_plates]) color("WhiteSmoke") translate([0, 0, i * gap + plate_t]) mirror([0, 0, 1]) plate_print();
  color("Gainsboro") translate([0, 0, (n_plates + 1) * gap + plate_t]) mirror([0, 0, 1]) cap_print();
  color("Purple") translate([-sensor_w / 2, -0.8, plate_t]) cube([sensor_w, 1.6, 11]);
}
