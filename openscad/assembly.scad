// Overview: "ext" box + radiation shield clipped underneath,
// on the wall (mount = "wall") or on the table stand (mount = "stand", stand.scad).
// The parameters below must stay aligned with the models.
// Render: openscad --preview --camera=0,0,0,-20,-25,0,0 --viewall --autocenter --imgsize=1000,1200 -o rendu.png assembly.scad
//         openscad --preview -D 'mount="stand"' --camera=0,0,0,-20,-35,0,0 --viewall --autocenter --imgsize=1000,1200 -o rendu_pied.png assembly.scad

mount = "wall"; // [wall, stand]

use <tinysensor_box.scad>
use <sensor_shield.scad>
use <stand.scad>
use <euro2.scad>
// parameters copied from both files
cav_w = 95.98; reach = 22.4; base_h = 2.4 + 5 + 1.6 + 25 + 2;
gap = 10; plate_t = 1.6; n = 4; stack_h = (n + 1) * gap + plate_t;
wall_y = 40; clip_t = 4; clip_gap = 6;
cx = cav_w / 2; oy = -reach - (stack_h + clip_gap); W = wall_y + clip_t;
// wall

// box
color("DimGray") base(usage = "ext");
color("Gray") translate([0, 0, base_h]) lid();
// shield (shield use coordinates -> box coordinates)
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

// €2 coin for scale (euro2.scad)
// on the wall: flat against the wall, right of the shield; on the stand: lying on the table in front of the foot
if (mount == "stand") translate([cx + 45, -114, 115]) rotate([-90, 0, 0]) euro2();
else translate([cav_w + 35, -60, 0]) euro2();
