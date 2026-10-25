// Abri de sonde extérieur (type Stevenson à assiettes) pour breakout BMx280
// Unités : mm. Matière : PETG (blanc de préférence : moins d'échauffement solaire).
//
// Pièces (toutes imprimables sans support, face supérieure sur le plateau) :
//   cap   : chapeau plein + bras + platine murale       x1
//   plate : assiette ajourée                            x n_plates
//   base  : fond avec glissière capteur et passe-câble  x1
//   clip  : (option) adaptateur pour accrocher l'abri sous le boîtier principal  x1
//           boulonné derrière la platine murale (2 vis M4x8 tête fraisée + écrous
//           noyés), il se clipse sur la patte murale basse du boîtier, qui porte
//           deux encoches latérales (tinysensor_box.scad, shield_notch = true).
// Export :
//   openscad -D 'part="cap"'   -o shield_cap.3mf   sensor_shield.scad
//   openscad -D 'part="plate"' -o shield_plate.3mf sensor_shield.scad
//   openscad -D 'part="base"'  -o shield_base.3mf  sensor_shield.scad
//
// Assemblage : 3 tiges filetées M3 (ou vis M3 longues) traversant les entretoises,
// écrous sous le fond. Fixer le chapeau au mur AVANT d'empiler les assiettes.

/* [Rendu] */
part = "assembly"; // [cap, plate, base, clip, assembly]

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

/* [Clip sous le boîtier — doit correspondre à la patte basse de tinysensor_box.scad] */
ear_t = 4;          // épaisseur de la patte (floor_t + 1.6)
ear_w = 20;
notch_y = 8;        // encoche depuis le bout de la patte (shield_notch_y)
clip_gap = 6;       // jeu vertical entre bout de patte et dessus du chapeau
clip_engage = 16;   // longueur de patte prise dans le clip
clip_t = 4;         // épaisseur de l'adaptateur
prong_w = 2.4;      // largeur des branches (flexion)
lip = 3;            // recouvrement de la face avant de la patte
bump_r = 0.8;       // bossage qui s'engage dans l'encoche (r = 1)

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


// Adaptateur de clip, modélisé en coordonnées d'usage puis couché pour l'impression :
// X horizontal, Y vertical (haut), Z sortant du mur (dos sur le mur, Z = 0).
module clip_use() {
  y0 = plate_t;                     // bas de la platine murale
  y1 = stack_h;                     // haut (dessus du chapeau)
  ye = y1 + clip_gap;               // bout de la patte du boîtier
  yt = ye + clip_engage;            // haut des branches
  zi = ear_t + 0.3;                 // face intérieure des lèvres
  zt = zi + lip;                    // épaisseur totale des branches (lèvre à 45°)
  difference() {
    translate([-bracket_w / 2, y0, 0]) cube([bracket_w, y1 - y0, clip_t]);
    for (y = [y1 - 12, y0 + 12]) translate([0, y, -eps]) {
      cylinder(d = 4.4, h = clip_t + 2 * eps);
      cylinder(d = 8.1 + 0.3, h = 3.4, $fn = 6);  // écrou M4 noyé côté mur
    }
  }
  for (sx = [-1, 1]) {
    xi = sx * (ear_w / 2 + 0.2);    // face intérieure de la branche
    xo = sx * (ear_w / 2 + 0.2 + prong_w);
    // branche
    translate([min(xi, xo), y1 - 10, 0]) cube([prong_w, yt - y1 + 10, zt]);
    // lèvre en biseau 45° : dessous incliné, imprimable dos au plateau ;
    // elle appuie sur l'arête avant de la patte et la plaque contre le mur
    hull() {
      translate([xi - (sx > 0 ? eps : 0), ye, zi]) cube([eps, clip_engage, zt - zi]);
      translate([xi - sx * lip - (sx > 0 ? eps : 0), ye, zt - eps]) cube([eps, clip_engage, eps]);
    }
    // bossage d'accroche
    translate([xi, ye + notch_y, 0]) cylinder(r = bump_r, h = zi);
  }
}
module clip_print() { clip_use(); }  // dos sur le plateau : Z d'usage = Z d'impression

if (part == "cap") cap_print();
else if (part == "plate") plate_print();
else if (part == "base") base_print();
else if (part == "clip") clip_print();
else {
  // vue d'ensemble en position d'usage (Z vers le haut)
  color("White") base_print();
  for (i = [1 : n_plates]) color("WhiteSmoke") translate([0, 0, i * gap + plate_t]) mirror([0, 0, 1]) plate_print();
  color("Gainsboro") translate([0, 0, (n_plates + 1) * gap + plate_t]) mirror([0, 0, 1]) cap_print();
  color("Purple") translate([-sensor_w / 2, -0.8, plate_t]) cube([sensor_w, 1.6, 11]);
}
