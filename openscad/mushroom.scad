// TinySensor « champignon » — boîtier décoratif (amanite), en plus du boîtier mural.
// Unités : mm. Matière : PETG.
//
// Le chapeau est un tout (dôme + lamelles) posé sur le pied avec une légère inclinaison ;
// toute l'électronique est dans le chapeau, le pied ne porte que l'accu.
//
//   cap     : dôme, bord ondulé ; vissé par en dessous (3 vis M3, écrous logés dans le disque)
//   gills   : disque à lamelles = dessous du chapeau ; fentes de ventilation, prise USB, fourreau sur le pied
//   chassis : cadre imprimé debout, vissé sur le disque : BME280 couché (chambre du bas), plancher,
//             carte v2.2 découpée, glissière du module de charge (prise vers le bas, entre deux lamelles)
//   deck    : plancher percé entre le capteur et la carte
//   stem    : pied creux (support 18650 vertical), lobé, base évasée, corolle ondulée, tenon incliné en haut
//   ground  : disque de 1 mm sous le pied, avec des touffes d'herbe (stabilité + décor)
//   dot     : pois blancs rapportés, collés dans les logements du dôme
//
// Export (toutes les pièces dans leur sens d'impression) :
//   openscad -D 'part="cap"'     -o mushroom_cap.3mf     mushroom.scad   # bord sur le plateau, supports internes
//   openscad -D 'part="gills"'   -o mushroom_gills.3mf   mushroom.scad   # lamelles vers le haut
//   openscad -D 'part="chassis"' -o mushroom_chassis.3mf mushroom.scad
//   openscad -D 'part="deck"'    -o mushroom_deck.3mf    mushroom.scad
//   openscad -D 'part="stem"'    -o mushroom_stem.3mf    mushroom.scad
//   openscad -D 'part="ground"'  -o mushroom_ground.3mf  mushroom.scad   # sol + herbe, en vert ; pied collé dessus
//   openscad -D 'part="dot"' -D 'dot_d=8' -o mushroom_dot8.3mf mushroom.scad   # x n, voir l'écho
// Rendus :
//   openscad --preview -D 'part="assembly"' --camera=0,0,125,75,0,25,700 --imgsize=1000,1200 -o rendu_champignon.png mushroom.scad
//   openscad --preview -D 'part="assembly"' --camera=0,0,100,112,0,200,520 --imgsize=1000,1000 -o rendu_champignon_dessous.png mushroom.scad
//   openscad --preview -D 'part="assembly"' --camera=0,0,30,65,0,40,260 --imgsize=1000,800 -o rendu_champignon_pied.png mushroom.scad
//   openscad --preview -D 'part="section"'  --camera=0,0,140,90,0,0,480 --imgsize=1000,1000 -o rendu_champignon_coupe.png mushroom.scad
//
// Repères : monde = Z vers le haut, axe du pied en X = Y = 0, Z = 0 sur la table.
// Chapeau (cap_xf) : Z = 0 sous le disque à lamelles, axe incliné de cap_tilt vers +X.
// Dôme : Z = 0 à son bord, cap_skirt sous le disque.

use <pcb.scad>
use <euro2.scad>

/* [Rendu] */
part = "assembly"; // [assembly, section, cap, gills, chassis, deck, stem, ground, dot]
dot_d = 8;         // diamètre du pois exporté (part = "dot")
cap_color = "#c0392b"; // rendu uniquement (tue-mouches ; panthère : "#8b5a2b")

/* [Carte v2.2 découpée — voir pcb.scad] */
crop_w = 60.02;
crop_h = 38.974;
pcb_t = 1.6;
pcb_holes = [[-26.1243, 15.6770], [25.9902, 15.5130]];   // MH5, MH1 du haut

/* [Chapeau] */
cap_tilt = 10;     // inclinaison du chapeau sur le pied
cap_R = 72;        // rayon moyen du bord du dôme
cap_H = 52;        // hauteur du dôme au-dessus de son bord
cap_n = 2.2;       // exposant de la super-ellipse (2 = ellipse)
cap_t = 2.4;
cap_lean = 0;      // déport du sommet (vers +X, côté penché)
tip_h = 50;        // pointe : hauteur ajoutée au sommet (0 = dôme simple)
tip_w = [70, 32, 40];  // demi-largeurs de la pointe : vers -X (flanc doux), vers +X (flanc raide), en Y
tip_c = [0, 0];    // centre de la pointe (repère du dôme)
cap_skirt = 4;     // le bord du dôme descend sous le disque (pattes de vissage au ras du bord)
tab_base = [90, 210, 330];  // vis du dôme : chacune glisse (±40°) vers le lobe le plus proche
dot_sizes = [6, 8, 10];
dot_pocket = 0.8;

/* [Contour du chapeau : générateur de lobes] */
lobe_seed = 6;     // graine : changer pour une autre forme
lobe_n = 4.5;      // nombre de lobes dominant
lobe_amp = 0.22;   // écart maxi au rayon moyen, au bord (0.22 = ±22 %)
lobe_top = 0.3;    // part de la déformation qui reste au sommet (1 = bosselé jusqu'en haut)
lobe_rot = 165;    // orientation du contour en degrés ; -1 = automatique (meilleur jeu autour de l'électronique)

/* [Lamelles] */
disc_t = 3;
collar_h = 20;     // fourreau sous le disque, reçoit le tenon du pied
collar_r = 22.5;   // rayon extérieur du fourreau (= haut du pied)
tenon_r = 19.5;
n_gills = 48;
gill_t = 1.2;
gill_depth = [17, 3];  // profondeur des lamelles près du pied / au bord

/* [Châssis] */
chamber_h = 8;     // BME280 couché
deck_t = 1.6;
pcb_gap = 5;       // plancher -> dessous de la carte (broches traversantes)
ch_wall = 2;
bme_w = 15;        // breakout BMx280 15 x 11
bme_d = 11;
chg_w = 17;        // module de charge TP4056 micro-USB (à mesurer)
chg_l = 26;
chg_t = 1.6;
chg_x = 40;        // plan de la carte du module
usb_cut = [8.5, 12];

/* [Pied] */
stem_h = 125;      // centre du haut du pied (sous le fourreau)
holder_w = 21;     // support 1x18650 du commerce (à mesurer)
holder_d = 19;
holder_l = 77;
stem_r = 16.5;     // rayon du fût sous l'évasement du haut (19 en bas)
stem_amp = 0.15;   // lobes du fût (n'ajoutent que de la matière : la paroi autour de l'accu ne s'amincit pas)
stem_twist = 1.2;  // rotation des lobes le long du fût (°/mm)
foot_flare = [5, 20];     // évasement de la base (mm) : petit côté, grand côté
foot_flare_h = [16, 40];  // hauteur de l'évasement : petit côté, grand côté
foot_dir = 200;    // direction du grand évasement (°)
ring_z = 78;       // anneau (corolle)
ring_amp = 0.35;   // ondulation de la corolle (débord et retombée)

/* [Sol] */
ground_t = 1;      // disque sous le pied (pièce séparée, à coller)
ground_R = 58;     // rayon moyen
ground_amp = 0.15;
grass_seed = 11;
grass_tufts = 4;   // touffes de grands brins autour du pied
grass_single = 30; // brins isolés
fringe_h = [3, 9]; // frange d'herbe en zigzag autour du pied : hauteur des dents

/* [Visserie] */
m3_pilot = 2.5;
m3_clear = 3.4;
m3_nut = 6.4;      // M3 sur plats + jeu
m25_pilot = 2.1;

$fn = 64;
eps = 0.01;

// ---------- dérivées ----------
bore_r = sqrt(holder_w * holder_w + holder_d * holder_d) / 2 + 0.5;
ix = crop_w / 2 + 0.4;              // demi-dimensions intérieures du châssis
iy = crop_h / 2 + 0.4;
ch_z = disc_t;                      // bas du châssis
deck_z = ch_z + chamber_h;          // dessus des appuis du plancher
pcb_z = deck_z + deck_t + pcb_gap;  // dessous de la carte
ch_h = pcb_z - ch_z;

module cap_xf() { translate([0, 0, stem_h]) rotate([0, cap_tilt, 0]) translate([0, 0, collar_h]) children(); }

// ---------- dôme ----------
// contour : harmoniques 2 à 7 à phases aléatoires, pondérées autour de lobe_n, normalisées à lobe_amp
function vsum(v, i = 0) = i < len(v) ? v[i] + vsum(v, i + 1) : 0;
lobe_terms = [for (k = [2 : 7]) let(q = rands(0, 1, 2, lobe_seed * 17 + k)) [k, (0.5 + 0.5 * q[0]) * exp(-pow((k - lobe_n) / 1.5, 2)), 360 * q[1]]];
function lobe_raw(phi) = vsum([for (h = lobe_terms) h[1] * sin(h[0] * phi + h[2])]);
lobe_peak = max([for (i = [0 : 179]) abs(lobe_raw(2 * i))]);
function lobe_at(phi, rot) = lobe_amp * lobe_raw(phi - rot) / lobe_peak;
// déformation pleine au bord (s = 0), réduite à lobe_top au sommet (s = 1)
function wav_r(phi, s, rot) = 1 + lobe_at(phi, rot) * (lobe_top + (1 - lobe_top) * (1 - s));
function inner_rr(phi, zz, rot) = let(hi = cap_H - cap_t, s = min(1, zz / hi))
  (cap_R * wav_r(phi, s, rot) - cap_t) * pow(1 - pow(s, cap_n), 1 / cap_n);
// jeu : points de l'électronique (repère du chapeau) qui doivent rester à >= 3 mm sous le dôme
clear_pts = concat(
  [for (sx = [-1, 1], sy = [-1, 1]) [sx * (crop_w / 2 + 2.4), sy * (crop_h / 2 + 2.4), disc_t + 17]],   // châssis
  [for (sx = [-1, 1], sy = [-1, 1]) [sx * 30.5, sy * 20, disc_t + 31]],                                // composants
  [for (x = [38, 45], sy = [-1, 1]) [x, sy * 10.5, disc_t + 26]]);                                       // chargeur
// v <= 1 : le point est sous le dôme réduit de clear_m dans toutes les directions
clear_m = 3;
function v_in(p, rot) = let(zz = p[2] + cap_skirt, x = p[0] - cap_lean * pow(zz / cap_H, 2), phi = atan2(p[1], x),
  hi = cap_H - cap_t - clear_m, s = min(1, zz / (cap_H - cap_t)))
  pow(norm([x, p[1]]) / (cap_R * wav_r(phi, s, rot) - cap_t - clear_m), cap_n) + pow(zz / hi, cap_n);
function worst(rot) = max([for (p = clear_pts) v_in(p, rot)]);
rots = [for (a = [0 : 5 : 355]) worst(a)];
best_rot = search(min(rots), rots)[0] * 5;
rot = lobe_rot >= 0 ? lobe_rot : best_rot;
function wav(phi, s = 0) = wav_r(phi, s, rot);
function cap_pt(phi, t, off = 0) =
  let(s = pow(sin(t), 2 / cap_n), r = (cap_R * wav(phi, s) - off) * pow(cos(t), 2 / cap_n), z = (cap_H - off) * s)
  let(x = r * cos(phi) + cap_lean * pow(z / cap_H, 2), y = r * sin(phi)) [x, y, z + tip_bump(x, y, s, off)];
// pointe : bosse (1 - d²)³, pentes progressives (largeurs différentes de chaque côté), nulle au bord (s = 0) pour garder le bord plat ;
// côté intérieur (off > 0), pointe un peu plus étroite et plus basse pour garder l'épaisseur sur le flanc raide
function tip_bump(x, y, s, off) = let(k = off > 0 ? 3 : 0, dx = x - tip_c[0],
  d = norm([dx / ((dx > 0 ? tip_w[1] : tip_w[0]) - k), (y - tip_c[1]) / (tip_w[2] - k)]))
  tip_h <= 0 || d >= 1 ? 0 : (tip_h - k) * pow(1 - d * d, 3) * (1 - pow(1 - s, 3));
// rayon intérieur du dôme à la hauteur zz (repère du dôme), et décalage du sommet à cette hauteur
function inner_r(phi, zz) = inner_rr(phi, zz, rot);
function lean_at(zz) = cap_lean * pow(zz / cap_H, 2);

na = 90;
nt = 120;   // pas de 0.75° : flanc raide de la pointe sans escaliers
function ring(j, off) = [for (i = [0 : na - 1]) cap_pt(i * 360 / na, j * 90 / nt, off)];
function surf(off) = concat([for (j = [0 : nt - 1]) each ring(j, off)], [cap_pt(0, 90, off)]);
function o(i, j) = j * na + (i % na);
module dome_solid(off = 0) {   // volume plein sous la surface, fermé par le plan du bord
  polyhedron(convexity = 6, points = surf(off), faces = concat(
    [for (j = [0 : nt - 2], i = [0 : na - 1]) [o(i, j), o(i, j + 1), o(i + 1, j + 1), o(i + 1, j)]],
    [for (i = [0 : na - 1]) [o(i, nt - 1), nt * na, o(i + 1, nt - 1)]],
    [[for (i = [0 : na - 1]) o(i, 0)]]));
}

// pois : tirage « disque de Poisson » sur la surface réelle (pointe comprise) : candidats sur une grille
// (phi, t) dense et bruitée, parcourus dans un ordre pseudo-aléatoire, gardés s'ils sont assez loin des autres
dot_gap = 6;       // espace mini entre deux pois
dot_cand = [for (t = [4 : 2 : 88], phi = [0 : 3 : 357]) let(q = rands(0, 1, 3, t * 1000 + phi + lobe_seed))
  let(tt = t + q[0] * 2, pp = phi + q[1] * 3, p = cap_pt(pp, tt)) if (p[2] - tip_bump(p[0], p[1], 0, 0) > 9 || p[2] > 20)
  [pp, tt, dot_sizes[floor(q[2] * 2.99)], p]];
function dot_far(c, acc, i = 0) = i >= len(acc) ? true
  : norm(c[3] - acc[i][3]) < (c[2] + acc[i][2]) / 2 + dot_gap ? false : dot_far(c, acc, i + 1);
function dot_pick(i, acc) = i >= len(dot_cand) ? acc
  : let(c = dot_cand[(i * 7919) % len(dot_cand)]) dot_pick(i + 1, dot_far(c, acc) ? concat(acc, [c]) : acc);
dots = dot_pick(0, []);
function unit(v) = v / norm(v);
function cap_normal(phi, t) = unit(cross(cap_pt(phi + 0.5, t) - cap_pt(phi - 0.5, t), cap_pt(phi, t + 0.5) - cap_pt(phi, t - 0.5)));
module at_dot(d) {
  n = cap_normal(d[0], d[1]);
  translate(cap_pt(d[0], d[1])) rotate([0, acos(n[2]), atan2(n[1], n[0])]) children();
}

// vis du dôme : à mi-largeur des pattes
function tab_r(a) = inner_r(a, 2) - 5;
function argmax_a(a0) = let(c = [for (d = [-40 : 2 : 40]) lobe_at(a0 + d, rot)]) a0 - 40 + 2 * search(max(c), c)[0];
tab_a = [for (a = tab_base) argmax_a(a)];
module cap() {
  difference() {
    union() {
      difference() {
        dome_solid();
        translate([0, 0, -eps]) dome_solid(cap_t);
      }
      // pattes au ras du bord (sur le plateau), le disque à lamelles repose dessus
      intersection() {
        for (a = tab_a) rotate([0, 0, a]) translate([inner_r(a, 2) - 10, -6, 0]) cube([14, 12, cap_skirt]);
        dome_solid();
      }
    }
    for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) cylinder(d = d[2] + 0.3, h = dot_pocket + 1, $fn = 32);
    for (a = tab_a) rotate([0, 0, a]) translate([tab_r(a), 0, -eps]) cylinder(d = m3_clear, h = cap_skirt + 2 * eps);
  }
}

module dot(d) {
  cylinder(d = d, h = dot_pocket, $fn = 32);
  translate([0, 0, dot_pocket]) intersection() {
    scale([1, 1, 0.45]) sphere(d = d, $fn = 32);
    translate([-d, -d, 0]) cube([2 * d, 2 * d, d]);
  }
}

// ---------- disque à lamelles (repère du chapeau) ----------
function disc_pts() = [for (i = [0 : na - 1]) let(a = i * 360 / na, r = inner_r(a, cap_skirt + disc_t) - 0.4)
                         [r * cos(a) + lean_at(cap_skirt + disc_t), r * sin(a)]];
disc_rmax = cap_R - cap_t;   // normalisation de la profondeur des lamelles (au-delà : profondeur mini)
function gill_d(r) = gill_depth[1] + (gill_depth[0] - gill_depth[1]) * pow(max(0, (disc_rmax - r) / (disc_rmax - collar_r)), 1.3);
function adiff(a, b) = abs(((a - b + 540) % 360) - 180);
// angles où une lamelle gênerait : vis du dôme, prise USB, vis du châssis, vis du fourreau
ch_screws = [for (sx = [-1, 1], sy = [-1, 1]) [sx * (ix - 2.5), sy * (iy - 2.5)]];
gill_skip = concat([for (a = tab_a) [a, 7]], [[0, 12], [180, 7]], [for (p = ch_screws) [atan2(p[1], p[0]), 5]]);
function gill_ok(a) = min([for (s = gill_skip) adiff(a, s[0]) - s[1]]) > 0;

module gills() {
  difference() {
    union() {
      linear_extrude(disc_t) polygon(disc_pts());
      // fourreau
      translate([0, 0, -collar_h]) cylinder(r = collar_r, h = collar_h + eps);
      // lamelles : lames radiales, plus profondes près du pied
      intersection() {
        for (i = [0 : n_gills - 1]) let(a = (i + 0.5) * 360 / n_gills) if (gill_ok(a))
          rotate([0, 0, a]) rotate([90, 0, 0]) linear_extrude(gill_t, center = true)
            polygon(concat([[collar_r - 1, 0]], [for (r = [collar_r - 1 : 2 : cap_R * 1.5]) [r, -gill_d(r)]], [[cap_R * 1.5, 0]]));
        translate([0, 0, -50]) linear_extrude(50 + eps) offset(delta = -1) polygon(disc_pts());
      }
    }
    // fourreau
    translate([0, 0, -collar_h - eps]) cylinder(r = tenon_r + 0.25, h = collar_h);
    translate([0, 0, -1]) cylinder(r = 14, h = disc_t + 2);   // fils de l'accu
    // vis radiale du fourreau (côté -X)
    translate([-collar_r - 1, 0, -collar_h + 8]) rotate([0, 90, 0]) cylinder(d = m3_clear, h = collar_r - tenon_r + 2);
    // fentes de ventilation sous la chambre du capteur, entre les lamelles
    for (i = [0 : n_gills - 1]) let(a = i * 360 / n_gills) if (adiff(a, 0) > 14)
      rotate([0, 0, a]) translate([collar_r + 1, -0.8, -1]) cube([min(ix / max(abs(cos(a)), 0.01), iy / max(abs(sin(a)), 0.01), 31) - collar_r - 1, 1.6, disc_t + 2]);
    // prise micro-USB
    translate([chg_x + chg_t + 1.5 - usb_cut[0] / 2, -usb_cut[1] / 2, -1]) cube([usb_cut[0], usb_cut[1], disc_t + 2]);
    // vis du dôme : passage + écrou logé côté intérieur
    for (a = tab_a) rotate([0, 0, a]) translate([tab_r(a), 0, 0]) {
      translate([0, 0, -1]) cylinder(d = m3_clear, h = disc_t + 2);
      translate([0, 0, disc_t - 2.4]) cylinder(d = m3_nut / cos(30), h = 3, $fn = 6);
    }
    // vis du châssis, têtes sous le disque
    for (p = ch_screws) translate([p[0], p[1], -1]) cylinder(d = m3_clear, h = disc_t + 2);
  }
}

// ---------- châssis (repère du chapeau, posé sur le disque) ----------
module chassis() {
  translate([0, 0, ch_z]) difference() {
    union() {
      difference() {
        translate([-ix - ch_wall, -iy - ch_wall, 0]) cube([2 * (ix + ch_wall), 2 * (iy + ch_wall), ch_h]);
        translate([-ix, -iy, -1]) cube([2 * ix, 2 * iy, ch_h + 2]);
        // fenêtres de la chambre
        for (x = [-20, -6, 8, 22]) translate([x - 4, -iy - ch_wall - 1, 2]) cube([8, 2 * (iy + ch_wall) + 2, chamber_h - 3]);
        for (y = [-8, 8]) translate([-ix - ch_wall - 1, y - 4, 2]) cube([ch_wall + 2, 8, chamber_h - 3]);
      }
      // appuis du plancher et de la carte le long des grands côtés (chanfrein 45° dessous)
      for (s = [-1, 1], z = [chamber_h, ch_h]) mirror([0, s < 0 ? 1 : 0, 0]) translate([-ix, iy, z])
        rotate([0, 90, 0]) linear_extrude(2 * ix) polygon([[0, 0], [1.5, 0], [0, -1.5]]);
      // coins : bossages des vis du châssis (bas) ; coins hauts pleins jusqu'à la carte (vis M2.5 de la carte)
      for (p = ch_screws) translate([p[0] > 0 ? ix - 7 : -ix, p[1] > 0 ? iy - 7 : -iy, 0])
        cube([7, 7, p[1] > 0 ? ch_h : chamber_h - 1]);
      // glissière du BME280 couché (capteur vers le bas), contre la paroi -Y : les bords entrent de 0.8 dans les rainures
      for (s = [-1, 1]) translate([s * (bme_w / 2 - 0.8) + (s > 0 ? 0 : -2), -iy - eps, 0]) difference() {
        cube([2, bme_d + 1, chamber_h - 1]);
        translate([s > 0 ? -eps : 1, -eps, 2]) cube([1 + eps, bme_d + 2, 1.9]);
      }
      // glissière du module de charge, tenue par deux bras sur la paroi +X
      translate([chg_x, 0, 0]) rails(chg_w, chg_t, 20);
      for (s = [-1, 1]) translate([ix + ch_wall - eps, s > 0 ? chg_w / 2 - 1 : -chg_w / 2 - 2, 0]) cube([chg_x - 1.5 - ix - ch_wall + 2 * eps, 3, 20]);
    }
    // vis du châssis (par en dessous) et de la carte (par le dessus)
    for (p = ch_screws) translate([p[0], p[1], -eps]) cylinder(d = m3_pilot, h = 7);
    for (h = pcb_holes) translate([h[0], h[1], ch_h - 6]) cylinder(d = m25_pilot, h = 6 + eps);
  }
}

// deux rails en U face à face, carte dans le plan YZ (épaisseur selon X)
module rails(w, t, h) {
  for (s = [-1, 1]) translate([-1.5, s > 0 ? w / 2 - 1 : -w / 2 - 2, 0]) difference() {
    cube([t + 3, 3, h]);
    translate([1.5 - 0.15, s > 0 ? -eps : 2 - eps, -eps]) cube([t + 0.3, 1 + eps, h + 2 * eps]);
  }
}

// ---------- plancher percé ----------
module deck() {
  difference() {
    translate([-ix + 0.3, -iy + 0.3, 0]) cube([2 * ix - 0.6, 2 * iy - 0.6, deck_t]);
    for (p = ch_screws) translate([p[0] - 4.6, p[1] - 4.6, -1]) cube([9.2, 9.2, deck_t + 2]);
    for (x = [-24 : 6 : 24], y = [-15 : 6 : 15]) translate([x + (round(y / 6) % 2) * 3, y, -1]) cylinder(d = 4, h = deck_t + 2, $fn = 6);
  }
}

// ---------- pied ----------
// générateur générique (même principe que le contour du chapeau) : harmoniques 2..7 à phases aléatoires
function hterms(seed, n) = [for (k = [2 : 7]) let(q = rands(0, 1, 2, seed * 17 + k)) [k, (0.5 + 0.5 * q[0]) * exp(-pow((k - n) / 1.5, 2)), 360 * q[1]]];
function hval(T, phi) = vsum([for (h = T) h[1] * sin(h[0] * phi + h[2])]);
function hpeak(T) = max([for (i = [0 : 179]) abs(hval(T, 2 * i))]);
function hnorm(T, P, phi) = hval(T, phi) / P;   // dans [-1, 1]
stem_T = hterms(lobe_seed + 101, 3);  stem_P = hpeak(stem_T);
ring_T = hterms(lobe_seed + 202, 4);  ring_P = hpeak(ring_T);
ring_T2 = hterms(lobe_seed + 303, 5); ring_P2 = hpeak(ring_T2);
ground_T = hterms(grass_seed, 4);     ground_P = hpeak(ground_T);

flare_h = 14;      // évasement du haut, sous le fourreau
function base_r(z) = z <= stem_h - flare_h ? 19 - (19 - stem_r) * z / (stem_h - flare_h)
  : z <= stem_h ? stem_r + (collar_r - stem_r) * pow((z - stem_h + flare_h) / flare_h, 1.3) : collar_r;
// rayon extérieur du pied : fût conique + lobes tournants (nuls sous le fourreau) + évasement dissymétrique de la base
// (concave, pente à 45° au plus au pied, rayon décroissant en montant : imprimable)
function foot_w(phi) = pow((1 + cos(phi - foot_dir)) / 2, 2);   // 1 côté grand évasement, 0 à l'opposé
function stem_rr(phi, z) = let(w = foot_w(phi), F = foot_flare[0] + (foot_flare[1] - foot_flare[0]) * w,
  Hf = foot_flare_h[0] + (foot_flare_h[1] - foot_flare_h[0]) * w, fl = z < Hf ? F * pow(1 - z / Hf, 2) : 0,
  fade = max(0, min(1, (stem_h - flare_h - z) / 15)))
  base_r(z) * (1 + stem_amp * (0.5 + 0.5 * hnorm(stem_T, stem_P, phi - stem_twist * z)) * fade) + fl;

ns = 72;
stem_zs = [for (z = [0 : 1.5 : stem_h + 6]) z];
module stem_body() {
  J = len(stem_zs);
  function v(i, j) = j * ns + (i % ns);
  polyhedron(convexity = 4,
    points = [for (z = stem_zs, i = [0 : ns - 1]) let(a = i * 360 / ns, r = stem_rr(a, z)) [r * cos(a), r * sin(a), z]],
    faces = concat([for (j = [0 : J - 2], i = [0 : ns - 1]) [v(i, j), v(i, j + 1), v(i + 1, j + 1), v(i + 1, j)]],
                   [[for (i = [0 : ns - 1]) v(i, 0)]], [[for (i = [ns - 1 : -1 : 0]) v(i, J - 1)]]));
}

// corolle : jupe balayée autour du fût, débord et retombée ondulés, dessous à ~45° au moins
module ring_skirt() {
  function prof(a) = let(ri = stem_rr(a, ring_z) - 1, ext = 10 * (1 + ring_amp * hnorm(ring_T, ring_P, a)),
    drop = 16 * (1 + 0.6 * ring_amp * hnorm(ring_T2, ring_P2, a)))
    [[ri, ring_z + 1.4], [ri + ext, ring_z - drop + 1.2], [ri + ext, ring_z - drop], [ri, ring_z - 1.2]];
  function v(i, k) = (i % ns) * 4 + (k % 4);
  polyhedron(convexity = 4,
    points = [for (i = [0 : ns - 1], p = prof(i * 360 / ns)) let(a = i * 360 / ns) [p[0] * cos(a), p[0] * sin(a), p[1]]],
    faces = [for (i = [0 : ns - 1], k = [0 : 3]) [v(i, k + 1), v(i, k), v(i + 1, k), v(i + 1, k + 1)]]);
}

module stem() {
  difference() {
    union() {
      // fût, coupé sous le fourreau (plan perpendiculaire à l'axe du chapeau)
      intersection() {
        stem_body();
        cap_xf() translate([-100, -100, -collar_h - 200]) cube([200, 200, 200]);
      }
      ring_skirt();
      // tenon incliné, emboîté dans le fourreau
      cap_xf() translate([0, 0, -collar_h - 1]) cylinder(r = tenon_r, h = 17);
    }
    // logement de l'accu, vertical jusqu'en haut (le support passe par le tenon)
    translate([0, 0, 3]) cylinder(r = bore_r, h = stem_h + 40);
    // vis radiale du fourreau
    cap_xf() translate([-tenon_r - 1, 0, -collar_h + 8]) rotate([0, 90, 0]) cylinder(d = m3_pilot, h = 6);
  }
}

// ---------- sol : disque de 1 mm et végétation ----------
function ground_r(a) = ground_R * (1 + ground_amp * hnorm(ground_T, ground_P, a));
// brin : lame effilée (0.9 mm d'épaisseur, pointe 0.6 mm), courbée dans son plan, pente <= ~40° (imprimable)
module blade(h, w, lean) {
  for (k = [0 : 3]) hull() for (q = [k, k + 1])
    translate([lean * pow(q / 4, 2), 0, h * q / 4]) cube([max(0.6, w * pow(1 - q / 4, 1.3)), 0.9, 0.01], center = true);
}
grass = let(R = rands(0, 1, 6 * (grass_tufts * 9 + grass_single), grass_seed))
  concat(
    // touffes : 6 à 9 grands brins en éventail, collés au pied
    [for (t = [0 : grass_tufts - 1], b = [0 : 8]) let(o = 6 * (t * 9 + b), t0 = 6 * t * 9,
       ta = t * 360 / grass_tufts + 40 * R[t0], tr = stem_rr(ta, 0) + 2 + 4 * R[t0 + 1],
       fan = -70 + 140 * b / 8 + 15 * (R[o + 2] - 0.5), h = 10 + 9 * R[o + 3])
       if (b < 6 + 3 * R[t0 + 2])
         [[tr * cos(ta) + 4 * (R[o + 4] - 0.5), tr * sin(ta) + 4 * (R[o + 5] - 0.5)], ta + fan, h, 3 + 1.5 * R[o + 1], h * 0.55 * (R[o] - 0.2)]],
    // brins isolés
    [for (i = [0 : grass_single - 1]) let(o = 6 * (grass_tufts * 9 + i), a = 360 * R[o],
       rr = stem_rr(a, 0) + 4 + (ground_r(a) - 8 - stem_rr(a, 0) - 4) * pow(R[o + 1], 1.5), h = 4 + 6 * R[o + 2])
       [[rr * cos(a), rr * sin(a)], a + 90 * (R[o + 3] - 0.5), h, 2.5 + R[o + 4], h * 0.5 * (R[o + 5] - 0.5)]]);
// frange : dents triangulaires le long d'une courbe ondulée autour du pied (paroi continue de 0.9 mm)
fringe_n = 64;
fringe = let(R = rands(0, 1, 3 * fringe_n, grass_seed + 7))
  [for (i = [0 : fringe_n - 1]) let(a = i * 360 / fringe_n, rr = stem_rr(a, 0) + 2.5 + 2 * R[3 * i])
    [rr * cos(a), rr * sin(a), fringe_h[0] + (fringe_h[1] - fringe_h[0]) * R[3 * i + 1], 2 * (R[3 * i + 2] - 0.5)]];
module fringe_wall() {
  for (i = [0 : fringe_n - 1]) let(p = fringe[i], q = fringe[(i + 1) % fringe_n], m = (p + q) / 2,
                                   t = unit([q[0] - p[0], q[1] - p[1], 0]))
    hull() {
      translate([p[0], p[1], 0]) cylinder(d = 0.9, h = 0.01, $fn = 8);
      translate([q[0], q[1], 0]) cylinder(d = 0.9, h = 0.01, $fn = 8);
      translate([m[0] + t[0] * p[3] * 1.5, m[1] + t[1] * p[3] * 1.5, p[2]]) cylinder(d = 0.6, h = 0.01, $fn = 8);
    }
}
module ground() {
  linear_extrude(ground_t) polygon([for (i = [0 : 89]) let(a = i * 4) ground_r(a) * [cos(a), sin(a)]]);
  for (g = grass) translate([g[0][0], g[0][1], ground_t - 0.01]) rotate([0, 0, g[1]]) blade(g[2], g[3], g[4]);
  translate([0, 0, ground_t - 0.01]) fringe_wall();
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
module bme() { color("Purple") translate([-bme_w / 2, -iy, 0]) cube([bme_w, bme_d, 1.6]); }

module assembled() {
  color("Ivory") stem();
  color("#4a8f3c") translate([0, 0, -ground_t]) ground();
  translate([0, 0, 3]) holder_cell();
  cap_xf() {
    color("Ivory") gills();
    color("Beige") chassis();
    translate([0, 0, ch_z + 2.1]) bme();
    translate([chg_x + 0.15, 0, ch_z]) charger();
    color("WhiteSmoke") translate([0, 0, deck_z]) deck();
    translate([0, 0, pcb_z]) pcb_v22_cropped();
    translate([0, 0, -cap_skirt]) {
      color(cap_color) cap();
      for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) color("White") dot(d[2]);
    }
  }
}

echo(str("Contour : graine ", lobe_seed, ", orientation ", rot, "°, indice de jeu ", round(worst(rot) * 100) / 100, worst(rot) <= 1 ? " (ok : >= 3 mm partout)" : " (ATTENTION : électronique à moins de 3 mm du dôme, augmenter cap_R / cap_H ou changer de graine)"));
echo(str("Pois : ", len(dots), " au total — ", [for (s = dot_sizes) str("Ø", s, " x", len([for (d = dots) if (d[2] == s) 1]))]));
echo(str("Hauteur ~", round(stem_h + (collar_h + cap_H + tip_h - cap_skirt) * cos(cap_tilt) + 10), " mm ; chapeau Ø~", round(2 * cap_R), " ; logement accu Ø", 2 * bore_r));

if (part == "cap") cap();
else if (part == "gills") translate([0, 0, disc_t]) mirror([0, 0, 1]) gills();   // lamelles vers le haut
else if (part == "chassis") translate([0, 0, -ch_z]) chassis();
else if (part == "deck") deck();
else if (part == "stem") stem();
else if (part == "ground") ground();
else if (part == "dot") dot(dot_d);
else if (part == "section") difference() { assembled(); translate([-200, -200, -1]) cube([400, 200, 400]); }
else {
  assembled();
  translate([75, -25, 0]) euro2();
}
