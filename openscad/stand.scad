// Pied de table : boîtier + abri de sonde clipsé, debout sur une table.
// Unités : mm. Matière : PETG.
//
// Le mât tient lieu de mur : le boîtier s'y visse par ses deux pattes murales
// (M4 tête fraisée + écrous noyés dans le dos du mât), l'abri reste clipsé sous
// la patte basse comme au mur. Le mât s'emboîte dans la semelle (1 vis M4).
//
// Pièces (imprimables sans support) :
//   mast : mât, à plat, dos sur le plateau                 x1
//   foot : semelle avec douille, dessous sur le plateau     x1
// Export :
//   openscad -D 'part="mast"' -o stand_mast.3mf stand.scad
//   openscad -D 'part="foot"' -o stand_foot.3mf stand.scad
//
// Repère = celui de tinysensor_box.scad (X droite, Y haut, Z sortant du mur, mur en Z = 0).

/* [Rendu] */
part = "assembly"; // [mast, foot, assembly]

/* [Repris de tinysensor_box.scad / sensor_shield.scad — à garder alignés] */
cav_w = 95.98;
ear_top = 102.154;              // bout de la patte haute (cav_h + wall + flange + ear_l)
ear_holes_y = [96.154, -16.4];  // axes des trous de vis murales des pattes
shield_bottom = -80;            // dessous de l'abri clipsé (oy de assembly.scad)
shield_front = 72;              // avant de l'abri (wall_y + clip_t + plate_d / 2)

/* [Mât] */
mast_w = 30;
mast_t = 6;
under_clr = 30;    // dégagement sous l'abri : boucle du câble qui remonte par le fond

/* [Semelle] */
foot_w = 130;      // largeur (X)
foot_back = 40;    // débord derrière le mât
foot_front = 90;   // débord devant le mur (l'abri avance jusqu'à shield_front)
foot_t = 4;
foot_r = 10;
socket_h = 25;     // profondeur d'emboîtement du mât
socket_wall = 3;
fit = 0.3;         // jeu mât / douille
gusset_t = 3;

/* [Visserie] */
screw_d = 4.4;     // M4
nut_af = 8.4;      // écrou M4 (7.9 sur plats + jeu)
nut_h = 3.4;

$fn = 48;
eps = 0.01;

cx = cav_w / 2;
y_bot = shield_bottom - under_clr;   // pied du mât = dessus de la semelle
y_floor = y_bot - foot_t;             // table
mast_l = ear_top - y_bot;
socket_screw_y = y_bot + socket_h / 2;

module mast_use() {
  difference() {
    translate([cx - mast_w / 2, y_bot, -mast_t]) cube([mast_w, mast_l, mast_t]);
    // vis des pattes du boîtier, écrous noyés dans le dos
    for (y = ear_holes_y) translate([cx, y, -mast_t - eps]) {
      cylinder(d = screw_d, h = mast_t + 2 * eps);
      cylinder(d = nut_af / cos(30), h = nut_h, $fn = 6);
    }
    // vis de la douille
    translate([cx, socket_screw_y, -mast_t - eps]) cylinder(d = screw_d, h = mast_t + 2 * eps);
  }
}

module foot_use() {
  sx = mast_w / 2 + fit + socket_wall;   // demi-largeur extérieure de la douille
  z0 = -mast_t - fit - socket_wall;      // dos de la douille
  z1 = fit + socket_wall;                // face avant de la douille
  difference() {
    union() {
      // semelle
      translate([0, y_floor, 0]) rotate([-90, 0, 0]) linear_extrude(foot_t)
        translate([cx - foot_w / 2, -foot_front]) offset(r = foot_r) offset(delta = -foot_r)
          square([foot_w, foot_front + mast_t + foot_back]);
      // douille
      translate([cx - sx, y_bot - eps, z0]) cube([2 * sx, socket_h + eps, z1 - z0]);
      // goussets avant / arrière (plan YZ) et latéraux (plan XY)
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
    // logement du mât
    translate([cx - mast_w / 2 - fit, y_bot, -mast_t - fit]) cube([mast_w + 2 * fit, socket_h + eps, mast_t + 2 * fit]);
    // vis traversante M4 x 20 + écrou
    translate([cx, socket_screw_y, z0 - eps]) cylinder(d = screw_d, h = z1 - z0 + 2 * eps);
  }
}

if (part == "mast") translate([0, 0, mast_t]) mast_use();          // dos sur le plateau
else if (part == "foot") translate([0, 0, -y_floor]) rotate([90, 0, 0]) foot_use(); // Y d'usage -> Z
else {
  color("Peru") mast_use();
  color("Sienna") foot_use();
}
