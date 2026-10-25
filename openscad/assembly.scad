// Vue d'ensemble : boîtier version "ext" au mur + abri de sonde clipsé dessous.
// Les paramètres ci-dessous doivent rester alignés sur les deux modèles.
// Rendu : openscad --preview --camera=0,0,0,-20,-25,0,0 --viewall --autocenter --imgsize=1000,1200 -o rendu.png assembly.scad

use <tinysensor_box.scad>
use <sensor_shield.scad>
// paramètres repris des deux fichiers
cav_w = 95.98; reach = 22.4; base_h = 2.4 + 5 + 1.6 + 25 + 2;
gap = 14; plate_t = 1.6; n = 4; stack_h = (n + 1) * gap + plate_t;
wall_y = 52; clip_t = 4; clip_gap = 6;
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
