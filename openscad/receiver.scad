// Pupitre du récepteur tinyDecoders : Pro Micro, RXB6, OLED 0,91" 128x32.
// Unités : mm. Matière : PLA/PETG.
//
// Écran en façade inclinée vers l'utilisateur, prise micro-USB de la Pro Micro à l'arrière
// (alimentation 5 V + lignes RFLink), antenne hélicoïdale à l'intérieur, perçage SMA défonçable
// à l'arrière pour une embase et une antenne fouet.
// Pro Micro posée sur ses broches (8 mm, plastique compris) plantées dans deux rails percés : démontable.
// RXB6 retourné (composants en dessous, moignons des broches en l'air), en travers au-dessus de la
// Pro Micro, capot posé sur deux supports de part et d'autre de la Pro Micro, ergots + point de colle.
// Cotes marquées « À MESURER » : valeurs estimées.
//
// Pièces (imprimables sans support) :
//   shell : coque, dessus sur le plateau (face inclinée à 40° de la verticale)   x1
//   plate : fond vissé (4 vis), rails de la Pro Micro, supports du RXB6           x1
// Export :
//   openscad -D 'part="shell"' -o receiver_shell.stl receiver.scad
//   openscad -D 'part="plate"' -o receiver_plate.stl receiver.scad
//
// Repère : X largeur (gauche -> droite), Y profondeur (façade Y = 0 -> arrière), Z haut, dessus du fond en Z = 0.

/* [Rendu] */
part = "shell"; // [assembly, open, cutaway, shell, plate]

/* [Coque] */
box_w = 48;        // largeur : l'écran (38 mm) + parois et jeu
box_d = 52;        // profondeur : Pro Micro contre la paroi arrière
box_h = 26;        // hauteur à l'arrière : Pro Micro sur ses broches, RXB6 au-dessus, antenne hélicoïdale couchée
front_h = 10;      // hauteur du bas de la façade
face_tilt = 40;    // façade : inclinaison depuis la verticale (40° : se lit assis, imprimable à l'envers)
wall = 2;
corner_r = 2;      // arrondi du profil

/* [Fond] */
plate_t = 2;
plate_clr = 0.2;   // jeu du fond encastré dans la coque
boss_d = 6;        // 4 colonnes de vis dans les coins
boss_h = 8;
screw_pilot = 2.5; // vis M3 à métaux taraudant la colonne (repercer à 2.8 si elle force)
screw_clear = 3.4;
screw_head = [5.4, 3.1]; // tête cylindrique (diamètre, hauteur) mesurée, noyée dans le fond
screw_l = 6;       // M3x6 : une M3x8 demanderait des colonnes plus hautes, qui toucheraient l'écran
pad_floor = 1.2;   // matière sous la tête
pocket_clr = 0.4;  // jeu autour de l'écran dans son logement

/* [Pro Micro] */
pm_l = 33.4;       // mesuré
pm_w = 18.1;
pm_t = 1.6;
pm_comp = 1.74;    // composants au-dessus de la carte, hors prise (mesuré : 3.34 avec la carte)
pm_pin = 8;        // broches sous la carte, plastique compris (mesuré)
pm_spacer = 2.5;   // plastique des broches sous la carte
pm_row = 15.24;    // écart des deux rangées (0,6")
pm_pitch = 2.54;
pm_pins = 12;      // broches par rangée
pm_pin_off = 1.5 + (12 - 1) * 2.54 / 2 - 33.4 / 2; // décalage des rangées vers l'arrière (+Y) / centre de la carte : 1re broche à 1.5 du bout avant (mesuré)
usb_port = [8, 3]; // prise micro-USB (largeur, hauteur mesurée), dessus de la carte
usb_over = 1.7;    // dépassement de la prise au-delà du bord de la carte (mesuré)
usb_plug = [12, 8]; // ouverture dans la paroi pour la fiche surmoulée

/* [Rails de la Pro Micro] */
rail_h = 4;        // le plastique des broches repose sur le rail
rail_w = 3;
rail_hole = 1.2;   // broches carrées 0.64
rail_sink = 1.5;   // trous prolongés dans le fond

/* [RXB6] */
rx_l = 43.1;       // mesuré ; retourné, composants en dessous, bout DATA à gauche (X bas), bout ANT à droite
rx_w = 11.7;
rx_t = 1.0;
rx_y = 29.5;       // bord avant de la carte
rx_gap = 1;        // jeu entre les composants de la Pro Micro et le capot
rx_sh = [21, 10, 3.7]; // capot (mesuré)
rx_sh_y0 = 0.8;    // bord avant de la carte -> capot (mesuré)
rx_sh_x0 = 11.7;   // bout DATA -> capot (mesuré)
rx_q = [9, 4.45, 4.9]; // quartz couché au ras du bord arrière, bout DATA, de 2 à 11 (mesuré)
rx_q_x0 = 2;       // bout DATA -> quartz
sup_bite = 1;      // portée minimale d'un support sous le capot
ergot = [1, 1.5];  // ergot au-delà du bout du capot (épaisseur, hauteur au-dessus du support)
ant = [27.5, 5];   // antenne hélicoïdale (longueur, diamètre) mesurée, couchée au-dessus des moignons

/* [OLED 0,91" 128x32] */
oled_l = 38;       // carte (mesuré)
oled_w = 12;
oled_t = 1.2;
oled_glass = 1.9;  // verre + film de protection au-dessus de la carte (épaisseur totale mesurée 3.1)
oled_aa = [22.38, 5.58]; // zone active (plan du fabricant)
oled_aa_off = 0.56; // décalage de la zone active vers la droite (côté sans broches) : verre mesuré à 6.27 du bord côté broches, zone active à 2.1 du bord du verre (plan)
oled_aa_off_y = 0.78; // décalage vers le haut de la façade (broches à gauche) : zone active à 2.43 du bord haut de la carte (plan)
win_m = 1;         // marge de la fenêtre autour de la zone active
oled_pad = 3;      // surépaisseur de la façade autour du logement de l'écran

/* [Embase SMA] */
sma_d = 6.5;       // perçage de l'embase (méplat à 5.9) — À VÉRIFIER
sma_flat = 5.9;
sma_skin = 0.4;    // pellicule laissée côté extérieur : percer pour passer au fouet
sma_pos = [38, 20]; // X, Z sur la paroi arrière (au-dessus du RXB6)

$fn = 48;
eps = 0.01;

// ---------- dérivées ----------
face_dy = (box_h - front_h) * tan(face_tilt);   // recul du haut de la façade
face_a = 90 - face_tilt;                          // angle de la façade depuis l'horizontale
face_len = (box_h - front_h) / cos(face_tilt);
profile = [[0, -plate_t], [box_d, -plate_t], [box_d, box_h], [face_dy, box_h], [0, front_h]];   // parois prolongées sous Z = 0 : le fond s'y encastre
cb_h = screw_head[1] + 0.2;                       // lamage de la tête depuis le dessous du fond
pad_h = cb_h + pad_floor - plate_t;               // plot du fond sous chaque colonne
pilot_top = -plate_t + cb_h + screw_l + 0.5;      // bout de la vis + 0.5
bosses = [for (x = [wall + boss_d / 2, box_w - wall - boss_d / 2], y = [wall + boss_d / 2 + 1.5, box_d - wall - boss_d / 2]) [x, y]];
rx_x = box_w / 2;                                 // RXB6 centré entre les parois
rx_x0 = rx_x - rx_l / 2;                          // bout DATA
sh_x0 = rx_x0 + rx_sh_x0;                         // début du capot
sh_x1 = sh_x0 + rx_sh[0];                         // fin du capot
pm_x = (sh_x0 + sh_x1) / 2;                       // Pro Micro centrée sous le capot : supports de part et d'autre
pm_y1 = box_d - wall - 0.3 - usb_over;            // bord arrière de la carte, prise contre la paroi
pm_y = pm_y1 - pm_l / 2;
pm_z = rail_h + pm_spacer;                        // dessous de la carte
usb_port_z = pm_z + pm_t + usb_port[1] / 2;
rx_z = pm_z + pm_t + pm_comp + rx_gap;            // dessous du capot du RXB6
rx_bz = rx_z + rx_sh[2];                          // face composants de la carte RXB6
ant_z = rx_bz + rx_t + 2.5 + ant[1] / 2;          // axe de l'antenne : 2.5 de moignons + fils sous elle
sup_l = [sh_x0 - 0.3 - ergot[0], pm_x - pm_w / 2 - 0.3];   // support gauche (X)
sup_r = [pm_x + pm_w / 2 + 0.3, sh_x1 + 0.3 + ergot[0]];   // support droit (X)

// repère de la façade : origine au bas de la face extérieure, Y le long de la pente, Z sortant
module face_xf() { translate([0, 0, front_h]) rotate([face_a, 0, 0]) children(); }

// ---------- coque ----------
module body(off = 0) {   // off : retrait (intérieur)
  translate([off, 0, 0]) rotate([90, 0, 90])
    linear_extrude(box_w - 2 * off) offset(r = max(corner_r - off, 0.5)) offset(delta = -corner_r - off) polygon(profile);
}

module shell() {
  difference() {
    union() {
      difference() {
        body();
        // intérieur ouvert en bas
        union() { body(wall); translate([wall, wall, -plate_t - 1]) cube([box_w - 2 * wall, box_d - 2 * wall, front_h / 2 + plate_t + 1]); }
      }
      // colonnes de vis
      for (p = bosses) translate([p[0], p[1], pad_h]) cylinder(d = boss_d, h = boss_h - pad_h);   // posée sur le plot du fond
      // cônes de raccord : chaque colonne monte vers son coin de parois à 45°, plus de surplomb à l'impression (coque à l'envers)
      for (p = bosses) {
        c = [p[0] < box_w / 2 ? wall : box_w - wall, p[1] < box_d / 2 ? wall : box_d - wall];   // coin intérieur
        intersection() {
          body();
          hull() {
            translate([p[0], p[1], boss_h - eps]) cylinder(d = boss_d, h = eps);
            translate([c[0] - (c[0] == wall ? wall : 0.5), c[1] - (c[1] == wall ? wall : 0.5), boss_h + norm([p[0] - c[0], p[1] - c[1]]) + boss_d / 2])
              cube([wall + 0.5, wall + 0.5, eps]);
          }
        }
      }
      // surépaisseur de façade autour du logement de l'écran
      intersection() {
        body();
        face_xf() translate([box_w / 2 - oled_l / 2 - 2, face_len / 2 - oled_w / 2 - 2, -wall - oled_pad]) cube([oled_l + 4, oled_w + 4, oled_pad + 1]);
      }
    }
    for (p = bosses) translate([p[0], p[1], pad_h - eps]) cylinder(d = screw_pilot, h = pilot_top - pad_h);
    // fenêtre de l'écran et logement de la carte (verre affleurant sous la face)
    face_xf() {
      translate([box_w / 2 + oled_aa_off - oled_aa[0] / 2 - win_m, face_len / 2 + oled_aa_off_y - oled_aa[1] / 2 - win_m, -wall - 1]) cube([oled_aa[0] + 2 * win_m, oled_aa[1] + 2 * win_m, wall + 2]);
      translate([box_w / 2 - oled_l / 2 - pocket_clr, face_len / 2 - oled_w / 2 - pocket_clr, -wall - oled_pad - 1]) cube([oled_l + 2 * pocket_clr, oled_w + 2 * pocket_clr, oled_pad + 1 - 0.8 + eps]);
    }
    // prise micro-USB
    translate([pm_x - usb_plug[0] / 2, box_d - wall - 1, usb_port_z - usb_plug[1] / 2]) cube([usb_plug[0], wall + 2, usb_plug[1]]);
    // embase SMA défonçable : perçage à méplat, pellicule côté extérieur
    translate([sma_pos[0], box_d - wall - 1, sma_pos[1]]) rotate([-90, 0, 0]) intersection() {
      cylinder(d = sma_d, h = wall + 1 - sma_skin);
      translate([-sma_d / 2, -sma_flat / 2, 0]) cube([sma_d, sma_flat, wall + 1]);
    }
  }
}

// ---------- fond ----------
module plate() {
  difference() {
    union() {
      // fond encastré entre les parois, affleurant le bas de la coque
      translate([wall + plate_clr, wall + plate_clr, -plate_t]) linear_extrude(plate_t) offset(r = 0.5) offset(delta = -0.5) square([box_w - 2 * (wall + plate_clr), box_d - 2 * (wall + plate_clr)]);
      // rails : les broches de la Pro Micro s'y plantent, leur plastique repose dessus
      for (sx = [-1, 1]) translate([pm_x + sx * pm_row / 2 - rail_w / 2, pm_y + pm_pin_off - (pm_pins - 1) * pm_pitch / 2 - rail_w / 2, 0])
        cube([rail_w, (pm_pins - 1) * pm_pitch + rail_w, rail_h]);
      // plots sous les colonnes : la tête de vis s'y noie
      intersection() {
        for (p = bosses) translate([p[0], p[1], 0]) cylinder(d = screw_head[0] + 0.4 + 2.4, h = pad_h);
        translate([wall + plate_clr, wall + plate_clr, 0]) cube([box_w - 2 * (wall + plate_clr), box_d - 2 * (wall + plate_clr), pad_h]);
      }
      // supports du RXB6 : le capot repose dessus, un ergot au-delà de chaque bout du capot (+ point de colle)
      // gauche : dans la bande sans quartz (avant de la carte)
      translate([sup_l[0], rx_y + 0.5, 0]) cube([sup_l[1] - sup_l[0], rx_w - rx_q[1] - 1, rx_z]);
      translate([sup_l[0], rx_y + 0.5, 0]) cube([ergot[0], rx_w - rx_q[1] - 1, rx_z + ergot[1]]);
      translate([sup_r[0], rx_y + 0.5, 0]) cube([sup_r[1] - sup_r[0], rx_w - 1, rx_z]);
      translate([sup_r[1] - ergot[0], rx_y + 0.5, 0]) cube([ergot[0], rx_w - 1, rx_z + ergot[1]]);
    }
    for (sx = [-1, 1], i = [0 : pm_pins - 1])
      translate([pm_x + sx * pm_row / 2, pm_y + pm_pin_off + (i - (pm_pins - 1) / 2) * pm_pitch, -rail_sink]) cylinder(d = rail_hole, h = rail_h + rail_sink + 1, $fn = 12);
    for (p = bosses) translate([p[0], p[1], -plate_t - 1]) {
      cylinder(d = screw_clear, h = plate_t + pad_h + 2);
      cylinder(d = screw_head[0] + 0.4, h = cb_h + 1);   // lamage : tête noyée
    }
  }
}

// ---------- modules (gabarits) ----------
module promicro() {
  translate([pm_x - pm_w / 2, pm_y1 - pm_l, pm_z]) {
    color("#1a5fa0") cube([pm_w, pm_l, pm_t]);
    color("#222") translate([4, 8, pm_t]) cube([10, 10, pm_comp]);
    color("Silver") translate([pm_w / 2 - usb_port[0] / 2, pm_l - 5 + usb_over, pm_t]) cube([usb_port[0], 5, usb_port[1]]);
  }
  for (sx = [-1, 1]) color("#222") translate([pm_x + sx * pm_row / 2 - 1.27, pm_y + pm_pin_off - pm_pins * pm_pitch / 2, pm_z - pm_spacer]) cube([2.54, pm_pins * pm_pitch, pm_spacer]);
}
module rxb6() {
  color("#2a7a3a") translate([rx_x0, rx_y, rx_bz]) cube([rx_l, rx_w, rx_t]);
  color("Silver") translate([sh_x0, rx_y + rx_sh_y0, rx_z]) cube([rx_sh[0], rx_sh[1], rx_sh[2]]);
  color("Silver") translate([rx_x0 + rx_q_x0, rx_y + rx_w - rx_q[1], rx_bz - rx_q[2]]) cube([rx_q[0], rx_q[1], rx_q[2]]);
  color("Gold") translate([rx_x + rx_l / 2 - ant[0], rx_y + rx_w / 2, ant_z]) rotate([0, 90, 0]) cylinder(d = ant[1], h = ant[0]);   // hélicoïdale couchée, côté ANT
}
module oled() {
  face_xf() translate([box_w / 2 - oled_l / 2, face_len / 2 - oled_w / 2, -wall - oled_t - oled_glass]) {
    color("#1a1a1a") cube([oled_l, oled_w, oled_t]);
    color("#203060") translate([oled_l / 2 + oled_aa_off - oled_aa[0] / 2 - 3, 0.5, oled_t]) cube([oled_aa[0] + 6, oled_w - 1, oled_glass]);
  }
}

module assembled() {
  color("#e8e2d0", 0.45) shell();
  color("#c8c0a8") plate();
  promicro(); rxb6(); oled();
}

assert(rx_l + 0.6 <= box_w - 2 * wall, "RXB6 trop long entre les parois");
assert(pm_pin - pm_spacer <= rail_h + rail_sink, "Trous des rails trop courts pour les broches");
assert(sup_l[1] - sh_x0 >= sup_bite && sh_x1 - sup_r[0] >= sup_bite, "Supports du RXB6 hors du capot : Pro Micro plus large que le capot");
assert(rx_bz - rx_q[2] > 0, "Quartz sur le fond");
assert(ant_z + ant[1] / 2 < box_h - wall, "Antenne contre le dessus");
assert(pilot_top <= boss_h, "Vis trop longue pour la colonne");
assert(usb_port_z + usb_plug[1] / 2 < box_h - wall, "Prise USB trop haute");
echo(str("Pupitre ", box_w, " x ", box_d, " x ", box_h + plate_t, " mm ; façade à ", face_a, "° de l'horizontale, ", round(face_len), " mm de long"));

if (part == "shell") translate([0, 0, box_h]) rotate([180, 0, 0]) shell();   // dessus sur le plateau
else if (part == "plate") translate([0, 0, plate_t]) plate();
else if (part == "open") {   // vue ouverte : coque soulevée au-dessus du fond et des cartes
  translate([0, 0, 40]) color("#e8e2d0") shell();
  color("#c8c0a8") plate();
  promicro(); rxb6();
  translate([0, 0, 40]) oled();
}
else if (part == "cutaway") {   // coque coupée à mi-largeur : disposition intérieure
  color("#e8e2d0") difference() { shell(); translate([-1, -1, -1]) cube([box_w / 2 + 1, box_d + 2, box_h + 2]); }
  color("#c8c0a8") plate();
  promicro(); rxb6(); oled();
}
else assembled();
