// TinySensor « champignon » — boîtier décoratif (amanite au chapeau penché), en plus du boîtier mural.
// Unités : mm. Matière : PETG.
//
// De haut en bas :
//   cap   : chapeau penché, bord ondulé, pièce séparée vissée par en dessous ; lame d'air au-dessus du toit
//   roof  : toit du compartiment carte
//   carte v2.2 découpée le long des pointillés (sans les supports d'accu), à plat
//   deck  : plancher percé, porte la carte
//   core  : corps ; chambre du capteur (BME280) et du module de charge, prise USB vers le bas ;
//           jupe conique à lamelles autour (= dessous du chapeau), coupée selon le plan du bord du chapeau
//   stem  : pied creux avec l'accu 18650 vertical, anneau, volve ; vissé sous le corps
//   dot   : pois blancs rapportés, collés dans les logements du chapeau
//
// Export (toutes les pièces dans leur sens d'impression) :
//   openscad -D 'part="cap"'  -o mushroom_cap.3mf  mushroom.scad   # bord sur le plateau, supports internes
//   openscad -D 'part="roof"' -o mushroom_roof.3mf mushroom.scad
//   openscad -D 'part="deck"' -o mushroom_deck.3mf mushroom.scad
//   openscad -D 'part="core"' -o mushroom_core.3mf mushroom.scad
//   openscad -D 'part="stem"' -o mushroom_stem.3mf mushroom.scad
//   openscad -D 'part="dot"' -D 'dot_d=8' -o mushroom_dot8.3mf mushroom.scad   # x n, voir l'écho
// Rendus :
//   openscad --preview -D 'part="assembly"' --camera=0,0,112,72,0,25,640 --imgsize=1000,1200 -o rendu_champignon.png mushroom.scad
//   openscad --preview -D 'part="assembly"' --camera=0,0,105,108,0,205,560 --imgsize=1000,1000 -o rendu_champignon_dessous.png mushroom.scad
//   openscad --preview -D 'part="section"'  --camera=0,0,112,90,0,0,560 --imgsize=1000,1200 -o rendu_champignon_coupe.png mushroom.scad
//
// Repère : Z vers le haut, axe du pied en X = Y = 0, Z = 0 sous la volve (table).
// Le chapeau a son propre repère (cap_xf) : bord dans le plan Z = 0, incliné de cap_tilt autour de Y ;
// il penche vers +X.

use <pcb.scad>
use <euro2.scad>

/* [Rendu] */
part = "assembly"; // [assembly, section, exploded, cap, roof, deck, core, stem, dot]
dot_d = 8;         // diamètre du pois exporté (part = "dot")
cap_color = "#c0392b"; // rendu uniquement (tue-mouches ; panthère : "#8b5a2b")

/* [Carte v2.2 découpée — voir pcb.scad] */
crop_w = 60.02;
crop_h = 38.974;
standoff_h = 5;
pcb_t = 1.6;
comp_h = 14;       // composant le plus haut (bornier, condensateurs)

/* [Corps] */
core_r = 46;       // rayon extérieur ; diagonale de la carte découpée 71.6
wall = 2;
gill_t = 2;        // fond de la chambre
chamber_h = 28;    // chambre capteur + module de charge (vertical, 26 mm)
deck_t = 2;
roof_t = 2;
side_vents = 10;   // fenêtres de la chambre vers la jupe
skirt_t = 1.6;     // jupe à lamelles
skirt_zt = 40;     // hauteur où la jupe atteint skirt_r (profil courbe, tangent en haut)
skirt_r = 60;      // rayon maxi de la jupe
n_gills = 48;

/* [Chapeau] */
cap_R = 70;        // rayon moyen du bord
cap_H = 54;        // hauteur au-dessus du bord
cap_n = 2.2;       // exposant de la super-ellipse (2 = ellipse, plus grand = plus « plat »)
cap_t = 2.4;
cap_tilt = 16;     // inclinaison du chapeau
cap_zp = 38;       // hauteur du centre du bord au-dessus du dessous du corps
cap_lean = 10;     // déport du sommet (vers le côté penché)
cap_wave = [0.035, 0.02];  // ondulation du bord (harmoniques 3 et 5)
lug_a = [60, 180, 300];    // vis M3 du chapeau, par en dessous, à travers la jupe
lug_r = 54;
dot_sizes = [6, 8, 10];
dot_pitch = 17;
dot_pocket = 0.8;

/* [Pied] */
stem_h = 120;      // de la table au dessous du corps
holder_w = 21;     // support 1x18650 du commerce (à mesurer)
holder_d = 19;
holder_l = 77;
stem_r = 16.5;     // rayon du fût sous l'évasement (le logement fait Ø29.3)
flare_r = 24;      // évasement sous le corps
stem_screw_r = 20;
ring_z = 74;       // anneau

/* [Modules] */
chg_w = 17;        // module de charge TP4056 micro-USB (à mesurer)
chg_l = 26;
chg_t = 1.6;
chg_x = -34;       // plan de la carte du module
usb_cut = [8.5, 12]; // ouverture de la fiche micro-USB dans le fond (X radial, Y)
bme_w = 15;        // breakout BMx280 15 x 11, posé verticalement
bme_x = 32;

/* [Visserie] */
m3_pilot = 2.5;
m3_clear = 3.4;
m3_head = 6;

$fn = 64;
eps = 0.01;

// ---------- dérivées ----------
bore_r = sqrt(holder_w * holder_w + holder_d * holder_d) / 2 + 0.5;
r_in = core_r - wall;
ledge_z = gill_t + chamber_h;
pcb_z = ledge_z + deck_t + standoff_h;
core_h = pcb_z + pcb_t + comp_h + 1.4;     // haut de la paroi = dessous du toit
roof_boss_a = [90, 225, 315];
roof_boss_r = 40;
// trous de la carte découpée (MH5, MH1 du haut) et appuis à lèvre sur le bord opposé
pcb_holes = [[-26.1243, 15.6770], [25.9902, 15.5130]];
pcb_lips = [[-10, -crop_h / 2], [3, -crop_h / 2]];   // zone libre du bord bas (entre J13/R1 et le JST I2C_SPI)

module cap_xf() { translate([0, 0, cap_zp]) rotate([0, cap_tilt, 0]) children(); }

// ---------- chapeau ----------
function wav(phi) = 1 + cap_wave[0] * sin(3 * phi + 20) + cap_wave[1] * sin(5 * phi + 70);
function cap_pt(phi, t, off = 0) =
  let(r = (cap_R * wav(phi) - off) * pow(cos(t), 2 / cap_n), z = (cap_H - off) * pow(sin(t), 2 / cap_n))
  [r * cos(phi) + cap_lean * pow(z / cap_H, 2), r * sin(phi), z];

na = 90;  // pas angulaire 4°
nt = 30;  // pas en latitude 3°
function ring(j, off) = [for (i = [0 : na - 1]) cap_pt(i * 360 / na, j * 90 / nt, off)];
function surf(off) = concat([for (j = [0 : nt - 1]) each ring(j, off)], [cap_pt(0, 90, off)]);
function o(i, j) = j * na + (i % na);

module cap_solid(off = 0) {   // volume plein sous la surface, fermé par le plan du bord
  ap = nt * na;
  polyhedron(convexity = 6, points = surf(off), faces = concat(
    [for (j = [0 : nt - 2], i = [0 : na - 1]) [o(i, j), o(i, j + 1), o(i + 1, j + 1), o(i + 1, j)]],
    [for (i = [0 : na - 1]) [o(i, nt - 1), ap, o(i + 1, nt - 1)]],
    [[for (i = [0 : na - 1]) o(i, 0)]]));
}

module cap_shell() {
  difference() {
    cap_solid();
    translate([0, 0, -eps]) cap_solid(cap_t);
  }
}

// pois : rangées le long d'un méridien moyen, nombre par rangée selon la circonférence
prof = [for (t = [0 : 90]) [cap_R * pow(cos(t), 2 / cap_n), cap_H * pow(sin(t), 2 / cap_n)]];
function arc(i) = i >= 90 ? 0 : arc(i + 1) + norm(prof[i] - prof[i + 1]);
function t_at(s) = let(d = [for (i = [0 : 90]) abs(arc(i) - s)]) search(min(d), d)[0];
dot_rows = [for (s = [dot_pitch * 0.6 : dot_pitch : arc(0)]) let(t = t_at(s)) if (prof[t][1] > 9) t];
dots = [for (j = [0 : len(dot_rows) - 1]) let(t = dot_rows[j], r = prof[t][0],
          n = max(1, floor(2 * PI * r / dot_pitch)), jit = rands(-0.25, 0.25, n, j), sz = rands(0, 2.99, n, 100 + j))
          for (k = [0 : n - 1]) [(k + 0.5 * (j % 2) + jit[k]) * 360 / n, t, dot_sizes[floor(sz[k])]]];

function unit(v) = v / norm(v);
function cap_normal(phi, t) = unit(cross(cap_pt(phi + 0.5, t) - cap_pt(phi - 0.5, t), cap_pt(phi, t + 0.5) - cap_pt(phi, t - 0.5)));
module at_dot(d) {
  p = cap_pt(d[0], d[1]);
  n = cap_normal(d[0], d[1]);
  translate(p) rotate([0, acos(n[2]), atan2(n[1], n[0])]) children();
}

module cap() {
  difference() {
    cap_shell();
    for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) cylinder(d = d[2] + 0.3, h = dot_pocket + 1, $fn = 32);
  }
  // pattes intérieures au bord : reposent sur les tubes de la jupe, vis M3 par en dessous
  difference() {
    intersection() {
      for (a = lug_a) rotate([0, 0, a]) translate([lug_r - 5, -6, 0]) cube([cap_R, 12, 5]);
      cap_solid();
    }
    for (a = lug_a) rotate([0, 0, a]) translate([lug_r, 0, -eps]) cylinder(d = m3_pilot, h = 5 + eps);
  }
}

module dot(d) {
  cylinder(d = d, h = dot_pocket, $fn = 32);
  translate([0, 0, dot_pocket]) intersection() {
    scale([1, 1, 0.45]) sphere(d = d, $fn = 32);
    translate([-d, -d, 0]) cube([2 * d, 2 * d, d]);
  }
}

// ---------- corps ----------
// jupe : surface courbe (comme le dessous d'un chapeau) depuis le bas du corps jusqu'à skirt_r,
// pente à la base ~35° de la verticale ; coupée au plan du bord du chapeau
function skirt_rz(z, off = 0) = (z >= skirt_zt ? skirt_r : core_r + (skirt_r - core_r) * (1 - pow(1 - z / skirt_zt, 2))) - off;
module skirt_solid(off = 0) {
  rotate_extrude($fn = 128) polygon(concat([[0, off > 0 ? -1 : 0]], [for (z = [0 : 2 : skirt_zt]) [skirt_rz(z, off), z]], [[skirt_r - off, 201], [0, 201]]));
}
module skirt_outer_solid() { skirt_solid(); }
module below_rim() { cap_xf() translate([-300, -300, -400]) cube([600, 600, 400]); }

module lug_tube(solid = true) {   // dans le repère du chapeau, sous le plan du bord
  for (a = lug_a) rotate([0, 0, a]) translate([lug_r, 0, 0])
    if (solid) translate([0, 0, -80]) cylinder(d = 9, h = 80);
    else translate([0, 0, -81]) { cylinder(d = 6.5, h = 77); cylinder(d = m3_clear, h = 82); }
}

module core() {
  difference() {
    union() {
      cylinder(r = core_r, h = core_h);
      // jupe à lamelles et tubes de vissage du chapeau
      intersection() {
        union() {
          difference() {
            skirt_solid();
            skirt_solid(skirt_t);
          }
          cap_xf() lug_tube();
        }
        skirt_outer_solid();
        below_rim();
      }
    }
    // chambre et compartiment carte
    translate([0, 0, gill_t]) cylinder(r = r_in, h = core_h);
    // fond de la chambre : fentes radiales
    for (i = [0 : 23]) let(a = i * 15) if (a % 180 != 0)
      rotate([0, 0, a]) translate([26, -0.8, -eps]) cube([r_in - 28, 1.6, gill_t + 2 * eps]);
    // lamelles de la jupe (fentes), sauf au droit des tubes
    intersection() {
      for (i = [0 : n_gills - 1]) let(a = (i + 0.5) * 360 / n_gills)
        if (min([for (l = lug_a) abs(((a - l + 540) % 360) - 180)]) < 180 - 9)
          rotate([0, 0, a]) translate([core_r + 0.5, -0.6, 1.5]) cube([skirt_r - core_r + 2, 1.2, 80]);
      cap_xf() translate([-300, -300, -403]) cube([600, 600, 400]);   // s'arrêtent 3 mm sous le bord
    }
    // fenêtres de la chambre vers la jupe
    for (i = [0 : side_vents - 1]) rotate([0, 0, (i + 0.5) * 360 / side_vents])
      translate([r_in - 1, -2, gill_t + 4]) cube([wall + 2, 4, chamber_h - 8]);
    // passage des fils de l'accu
    translate([0, 0, -eps]) cylinder(r = 12, h = gill_t + 2 * eps);
    // prise micro-USB du module de charge, vers le bas
    translate([chg_x + chg_t + 1.5 - usb_cut[0] / 2, -usb_cut[1] / 2, -eps]) cube([usb_cut[0], usb_cut[1], gill_t + 2 * eps]);
    // tubes de vissage : alésage ouvert par le dessous
    cap_xf() lug_tube(solid = false);
    // vis du pied
    for (a = [30, 150, 270]) rotate([0, 0, a]) translate([stem_screw_r, 0, -eps]) cylinder(d = m3_pilot, h = 9);
  }
  // feuillure du plancher (chanfrein 45° dessous)
  difference() {
    translate([0, 0, ledge_z - 3]) cylinder(r = r_in + eps, h = 3);
    translate([0, 0, ledge_z - 3 - eps]) cylinder(r1 = r_in, r2 = r_in - 3, h = 3 + 2 * eps);
  }
  // bossages des vis du pied
  for (a = [30, 150, 270]) rotate([0, 0, a]) translate([stem_screw_r, 0, gill_t - eps]) difference() {
    cylinder(d = 7, h = 7);
    cylinder(d = m3_pilot, h = 8);
  }
  // bossages du toit (sous le rebord, cône dessous)
  for (a = roof_boss_a) rotate([0, 0, a]) translate([roof_boss_r, 0, 0]) difference() {
    hull() {
      translate([0, 0, core_h - 8]) cylinder(d = 8, h = 8);
      translate([r_in - roof_boss_r, -4, core_h - 8 - (r_in - roof_boss_r) - 4]) cube([0.5, 8, 12 + r_in - roof_boss_r]);
    }
    translate([0, 0, core_h - 8]) cylinder(d = m3_pilot, h = 8 + eps);
  }
  // glissières : module de charge (prise en bas) et BME280
  translate([chg_x, 0, gill_t - eps]) rails(chg_w, chg_t, 20);
  translate([bme_x, 0, gill_t - eps]) rails(bme_w, 1.6, 16);
}

// deux rails en U face à face, carte dans le plan YZ (épaisseur selon X)
module rails(w, t, h) {
  for (s = [-1, 1]) translate([-1.5, s > 0 ? w / 2 - 1 : -w / 2 - 2, 0]) difference() {
    cube([t + 3, 3, h]);
    translate([1.5 - 0.15, s > 0 ? -eps : 2 - eps, 0]) cube([t + 0.3, 1 + eps, h + eps]);
  }
}

// ---------- plancher percé ----------
module deck() {
  difference() {
    cylinder(r = r_in - 0.3, h = deck_t);
    for (x = [-45 : 8 : 45], y = [-45 : 8 : 45])
      let(p = [x + (round(y / 8) % 2) * 4, y])
        if (norm(p) < r_in - 5 && min([for (h = concat(pcb_holes, pcb_lips)) norm(p - h)]) > 6)
          translate([p[0], p[1], -eps]) cylinder(d = 4.5, h = deck_t + 2 * eps, $fn = 6);
    for (a = roof_boss_a) rotate([0, 0, a]) translate([roof_boss_r, 0, -eps]) cylinder(d = 9, h = deck_t + 2 * eps);
  }
  // 2 entretoises vissées (bord haut de la carte)
  for (h = pcb_holes) translate([h[0], h[1], deck_t - eps]) difference() {
    cylinder(d = 6, h = standoff_h + eps);
    cylinder(d = 2.1, h = standoff_h + 1);
  }
  // 2 appuis à lèvre (bord bas) : glisser le bord sous les lèvres, puis visser l'autre bord
  for (p = pcb_lips) translate([p[0] - 3, p[1] - 3, deck_t - eps]) {
    translate([0, 1, 0]) cube([6, 4, standoff_h]);
    cube([6, 2.8, standoff_h + pcb_t + 2]);
    translate([0, 0, standoff_h + pcb_t + 0.3]) cube([6, 4.5, 1.7]);
  }
}

// ---------- toit ----------
module roof() {
  difference() {
    union() {
      cylinder(r = core_r, h = roof_t);
      translate([0, 0, -2]) difference() { cylinder(r = r_in - 0.3, h = 2 + eps); translate([0, 0, -eps]) cylinder(r = r_in - 2.3, h = 3); }
    }
    for (a = roof_boss_a) rotate([0, 0, a]) translate([roof_boss_r, 0, -3]) {
      cylinder(d = m3_clear, h = 10);
      translate([0, 0, 3 + roof_t - 1.7]) cylinder(d1 = m3_clear, d2 = m3_head, h = 1.7 + eps);
    }
    for (a = roof_boss_a) rotate([0, 0, a]) translate([roof_boss_r - 5, -5, -2 - eps]) cube([10, 10, 2 + eps]);
  }
}

// ---------- pied ----------
// profil lisse, dévers ≤ 45° partout (imprimé debout, volve sur le plateau)
volva_h = 34;
flare_h = 12;
function stem_rz(z) =
  z <= volva_h / 2 ? 22 + 5 * sin(180 * z / volva_h)
  : z <= volva_h ? 27 - 8 * sin(90 * (z - volva_h / 2) / (volva_h / 2))
  : z <= stem_h - flare_h ? 19 - (19 - stem_r) * (z - volva_h) / (stem_h - flare_h - volva_h)
  : stem_r + (flare_r - stem_r) * pow((z - stem_h + flare_h) / flare_h, 1.3);
stem_prof = concat([[0, 0]], [for (z = [0 : 1 : stem_h]) [stem_rz(z), z]], [[0, stem_h]]);
module stem() {
  difference() {
    union() {
      rotate_extrude($fn = 96) polygon(stem_prof);
      // anneau : jupe conique (dessous à ~50°, imprimable)
      rotate_extrude($fn = 96) polygon([[stem_rz(ring_z) - 0.5, ring_z + 1.4], [stem_rz(ring_z) + 11, ring_z - 14],
                                        [stem_rz(ring_z) + 11, ring_z - 15.2], [stem_rz(ring_z) - 0.5, ring_z - 1.2]]);
    }
    translate([0, 0, 3]) cylinder(r = bore_r, h = stem_h);
    // vis M3 x 12 par en dessous, têtes logées dans l'évasement
    for (a = [30, 150, 270]) rotate([0, 0, a]) translate([stem_screw_r, 0, 0]) {
      translate([0, 0, stem_h - 6]) cylinder(d = m3_clear, h = 7);
      translate([0, 0, stem_h - flare_h]) cylinder(d = m3_head, h = flare_h - 6);
    }
  }
}

// ---------- maquettes ----------
module holder_cell() {
  color("#202020") difference() {
    translate([-holder_w / 2, -holder_d / 2, 0]) cube([holder_w, holder_d, holder_l]);
    translate([-holder_w / 2 + 1.5, -holder_d / 2 + 1.5, 4]) cube([holder_w - 3, holder_d, holder_l - 8]);
  }
  color("#2e6fd8") translate([0, 0, 6]) cylinder(d = 18.6, h = 65);
}
module charger() { color("#1a4fa0") translate([0, -chg_w / 2, 0]) cube([chg_t, chg_w, chg_l]); color("Silver") translate([chg_t, -4, 0]) cube([3, 8, 5.5]); }
module bme() { color("Purple") translate([0, -bme_w / 2, 0]) cube([1.6, bme_w, 11]); }

module assembled(ex = 0) {
  color("Ivory") stem();
  translate([0, 0, stem_h + ex * 0.4]) {
    color("Beige") core();
    translate([chg_x + 0.15, 0, gill_t]) charger();
    translate([bme_x + 0.15, 0, gill_t]) bme();
    color("WhiteSmoke") translate([0, 0, ledge_z + ex * 0.8]) deck();
    translate([0, 0, pcb_z + ex * 1.2]) pcb_v22_cropped();
    color("WhiteSmoke") translate([0, 0, core_h + ex * 1.8]) roof();
    translate([0, 0, ex * 2.4]) cap_xf() {
      color(cap_color) cap();
      for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) color("White") dot(d[2]);
    }
  }
  translate([0, 0, 3]) holder_cell();
}

echo(str("Pois : ", len(dots), " au total — ", [for (s = dot_sizes) str("Ø", s, " x", len([for (d = dots) if (d[2] == s) 1]))]));
echo(str("Hauteur ~", round(stem_h + cap_zp + cap_H * cos(cap_tilt) + cap_R * sin(cap_tilt) * 0.6), " mm ; logement accu Ø", 2 * bore_r));

if (part == "cap") cap();
else if (part == "roof") translate([0, 0, roof_t]) mirror([0, 0, 1]) roof();   // lèvre vers le haut
else if (part == "deck") deck();
else if (part == "core") core();
else if (part == "stem") stem();
else if (part == "dot") dot(dot_d);
else if (part == "exploded") assembled(40);
else if (part == "section") difference() { assembled(); translate([-200, -200, -1]) cube([400, 200, 400]); }
else {
  assembled();
  translate([45, -5, 0]) euro2();
}
