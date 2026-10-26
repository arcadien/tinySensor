// Vue d'ensemble : boîtier version "ext" + abri de sonde clipsé dessous,
// au mur (mount = "wall") ou sur le pied de table (mount = "stand", stand.scad).
// Les paramètres ci-dessous doivent rester alignés sur les modèles.
// Rendu : openscad --preview --camera=0,0,0,-20,-25,0,0 --viewall --autocenter --imgsize=1000,1200 -o rendu.png assembly.scad
//         openscad --preview -D 'mount="stand"' --camera=0,0,0,-20,-35,0,0 --viewall --autocenter --imgsize=1000,1200 -o rendu_pied.png assembly.scad

mount = "wall"; // [wall, stand]

use <tinysensor_box.scad>
use <sensor_shield.scad>
use <stand.scad>
use <euro2.scad>
// paramètres repris des deux fichiers
cav_w = 95.98; reach = 22.4; base_h = 2.4 + 5 + 1.6 + 25 + 2;
gap = 10; plate_t = 1.6; n = 4; stack_h = (n + 1) * gap + plate_t;
wall_y = 40; clip_t = 4; clip_gap = 6;
cx = cav_w / 2; oy = -reach - (stack_h + clip_gap); W = wall_y + clip_t;
// mur

// boîtier
color("DimGray") base(usage = "ext");
color("Gray") translate([0, 0, base_h]) lid();
// abri (coords d'usage de l'abri -> coords boîtier)
multmatrix([[1, 0, 0, cx], [0, 0, 1, oy], [0, -1, 0, W], [0, 0, 0, 1]]) {
  color("White") base_print();
  for (i = [1 : n]) color("WhiteSmoke") translate([0, 0, i * gap + plate_t]) mirror([0, 0, 1]) plate_print();
  color("Gainsboro") translate([0, 0, stack_h]) mirror([0, 0, 1]) cap_print();
}
color("SteelBlue") translate([cx, oy, 0]) clip_use();
if (mount == "stand") {
  color("Peru") mast_use();
  color("Sienna") foot_use();
}

// pièce de 2 € pour l'échelle (euro2.scad)
// au mur : à plat contre le mur, à droite de l'abri ; sur pied : posée sur la table devant la semelle
if (mount == "stand") translate([cx + 45, -114, 115]) rotate([-90, 0, 0]) euro2();
else translate([cav_w + 35, -60, 0]) euro2();
