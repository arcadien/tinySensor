// TinySensor v2.2 — boîtier mural vissé avec joint
// Unités : mm. Matière visée : PETG.
//
// Export :
//   openscad -D 'part="base"' -o base.3mf tinysensor_box.scad
//   openscad -D 'part="lid"'  -o lid.3mf  tinysensor_box.scad
//
// Repère : origine = coin "haut-gauche" du PCB vu de dessus (comme KiCad),
// X vers la droite, Y vers le bas (KiCad) converti en Y vers le haut ici.
// En pose murale, le côté "bas" (Y=0, côté support 18650) est orienté vers le sol.

/* [Rendu] */
part = "assembly"; // [base, lid, assembly]
usage = "ext";     // [int, ext]

/* [PCB — tiré de hardware/tinysensor/tinySensorv1.kicad_pcb] */
pcb_w = 93.98;
pcb_h = 63.754;
pcb_t = 1.6;
// Trous de fixation M2.5 (4 coins), coordonnées KiCad relatives au coin du PCB
pcb_holes_kicad = [[89.96, 3.97], [5.01, 4.79], [4.57, 57.40], [90.71, 57.48]];

/* [Volumes] */
clr = 1.0;            // jeu PCB / paroi
bay = 14;             // baie câble côté bas (presse-étoupe, antenne)
standoff_h = 5;       // hauteur sous PCB (pattes THT)
comp_h = 25;          // composant le plus haut au-dessus du PCB (gros condensateurs)
top_clr = 2;          // jeu sous le couvercle

/* [Parois] */
wall = 2.4;           // 6 périmètres buse 0.4
floor_t = 2.4;
lid_t = 3;
r_out = 3;            // rayon extérieur des angles

/* [Joint et vis de couvercle] */
flange = 8;           // largeur du rebord extérieur
flange_t = 5;         // épaisseur du rebord
groove_w = 2.2;       // gorge pour cordon/joint 2 mm (TPU, mousse EPDM)
groove_d = 1.5;
rib_w = 1.2;          // nervure du couvercle qui écrase le joint
rib_h = 1.0;
lid_screw_d_pilot = 2.5;  // M3 auto-taraudeuse dans le PETG
lid_screw_d_clear = 3.4;

/* [Fixations] */
pcb_screw_d_pilot = 2.1;  // M2.5 auto-taraudeuse
standoff_d = 6;
gland_d = 12.5;       // PG7
wall_screw_d = 4.5;   // vis murale Ø4
ear_l = 12;           // dépassement de la patte au-delà du rebord
shield_notch = true;  // encoches pour clipser l'abri de sonde sous le boîtier
shield_notch_y = 8;   // position des encoches depuis le bout de la patte basse

$fn = 48;
eps = 0.01;
gland_z = 11.5;      // axe du presse-étoupe (sous le chanfrein du rebord)
gland_x = 22;        // décalé de la patte murale centrale

// ---------- dimensions dérivées ----------
cav_w = pcb_w + 2 * clr;
cav_h = pcb_h + 2 * clr + bay;
cav_d = standoff_h + pcb_t + comp_h + top_clr;
base_h = floor_t + cav_d;
pcb_x0 = clr;                // coin PCB dans le repère cavité
pcb_y0 = clr + bay;
groove_off = wall + 1.8;     // axe de la gorge depuis le bord cavité
screw_off = wall + flange - 2.8; // axe des vis depuis le bord cavité

function holes() = [for (h = pcb_holes_kicad) [pcb_x0 + h[0], pcb_y0 + pcb_h - h[1]]];

module rrect(w, h, r) {
  translate([r, r]) offset(r = r) square([w - 2 * r, h - 2 * r]);
}
// rectangle cavité décalé de d (d>0 vers l'extérieur)
module ring_rect(d, r = 1) {
  translate([-d, -d]) rrect(cav_w + 2 * d, cav_h + 2 * d, max(r, 0.5));
}
module groove_path(off, w) {
  difference() { ring_rect(off + w / 2, 1 + off); ring_rect(off - w / 2, 1); }
}
function lid_screws() = let(o = screw_off) [
  [-o, -o], [cav_w + o, -o], [-o, cav_h + o], [cav_w + o, cav_h + o],
  [cav_w / 2, -o], [cav_w / 2, cav_h + o]
];

// ---------- base ----------
module base() {
  difference() {
    union() {
      // corps
      linear_extrude(base_h) ring_rect(wall, r_out);
      // rebord supérieur
      translate([0, 0, base_h - flange_t]) linear_extrude(flange_t) ring_rect(wall + flange, r_out);
      // chanfrein à 45° sous le rebord : imprimable sans support
      hull() {
        translate([0, 0, base_h - flange_t - flange]) linear_extrude(eps) ring_rect(wall, r_out);
        translate([0, 0, base_h - flange_t - eps]) linear_extrude(eps) ring_rect(wall + flange, r_out);
      }
      // pattes murales (haut et bas)
      for (s = [-1, 1]) ear(s);
    }
    // cavité
    translate([0, 0, floor_t]) linear_extrude(cav_d + eps) ring_rect(0, 1);
    // gorge du joint
    translate([0, 0, base_h - groove_d]) linear_extrude(groove_d + eps) groove_path(groove_off, groove_w);
    // trous de vis du couvercle (borgnes)
    for (p = lid_screws()) translate([p[0], p[1], base_h - flange_t + 0.8]) cylinder(d = lid_screw_d_pilot, h = flange_t);
    // presse-étoupe dans la paroi basse
    if (usage == "ext")
      translate([gland_x, -wall - flange - eps, gland_z]) rotate([-90, 0, 0]) cylinder(d = gland_d, h = wall + flange + 2 * eps);
    // aérations intérieures (fentes en paroi basse et haute)
    if (usage == "int") vents();
  }
  // entretoises PCB
  for (h = holes()) translate([h[0], h[1], floor_t - eps]) difference() {
    cylinder(d = standoff_d, h = standoff_h + eps);
    cylinder(d = pcb_screw_d_pilot, h = standoff_h + 1);
  }
}

module ear(s) {
  // la patte dépasse du rebord pour laisser passer le tournevis
  reach = wall + flange + ear_l;
  y = s < 0 ? -reach : cav_h;
  hy = s < 0 ? -(wall + flange + ear_l / 2) : cav_h + wall + flange + ear_l / 2;
  difference() {
    translate([cav_w / 2 - 10, y, 0]) cube([20, reach + eps, floor_t + 1.6]);
    translate([cav_w / 2, hy, -eps]) {
      cylinder(d = wall_screw_d, h = 10);
      translate([0, 0, floor_t + 1.6 - 2.2]) cylinder(d1 = wall_screw_d, d2 = wall_screw_d + 4.4, h = 2.2 + eps);
    }
    // encoches latérales de la patte basse : accroche du clip de l'abri de sonde (sensor_shield.scad, part="clip")
    if (s < 0 && shield_notch)
      for (sx = [-1, 1]) translate([cav_w / 2 + sx * 10, -reach + shield_notch_y, -eps]) cylinder(r = 1, h = 10);
  }
}

module vents() {
  n = 7; sw = 2; sl = 14; pitch = (cav_w - 20) / (n - 1);
  for (yy = [-wall - eps, cav_h - eps]) for (i = [0 : n - 1])
    translate([10 + i * pitch - sw / 2, yy, floor_t + 3]) cube([sw, wall + 2 * eps, sl]);
}

// ---------- couvercle ----------
module lid() {
  difference() {
    linear_extrude(lid_t) ring_rect(wall + flange, r_out);
    for (p = lid_screws()) translate([p[0], p[1], -eps]) cylinder(d = lid_screw_d_clear, h = lid_t + 2 * eps);
  }
  // nervure qui écrase le joint (face intérieure = z<0, retournée à l'impression)
  translate([0, 0, -rib_h]) linear_extrude(rib_h + eps) groove_path(groove_off, rib_w);
}

if (part == "base") base();
else if (part == "lid") translate([0, 0, lid_t]) mirror([0, 0, 1]) lid();  // face plate sur le plateau, nervure vers le haut
else {
  color("SteelBlue", 0.9) base();
  color("Orange", 0.6) translate([0, 0, base_h + 8]) lid();
  color("Green", 0.5) translate([pcb_x0, pcb_y0, floor_t + standoff_h]) cube([pcb_w, pcb_h, pcb_t]);
}
