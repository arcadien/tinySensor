// Pupitre du récepteur tinyDecoders : Pro Mini, RXB6, OLED 0,91" 128x32, module USB/série micro-USB.
// Unités : mm. Matière : PLA/PETG.
//
// Écran en façade inclinée vers l'utilisateur, prise micro-USB du module USB/série à l'arrière
// (alimentation 5 V + lignes RFLink), antenne hélicoïdale à l'intérieur, perçage SMA défonçable
// à l'arrière pour une embase et une antenne fouet.
// COTES DES MODULES À VÉRIFIER sur les pièces (usb_*, oled_*, rx_* : valeurs typiques, non mesurées).
//
// Pièces (imprimables sans support) :
//   shell : coque, dessus sur le plateau (face inclinée à 35° de la verticale)   x1
//   plate : fond vissé (3 vis), logement du module USB/série, rainures du RXB6   x1
// Export :
//   openscad -D 'part="shell"' -o receiver_shell.stl receiver.scad
//   openscad -D 'part="plate"' -o receiver_plate.stl receiver.scad
//
// Repère : X largeur (gauche -> droite), Y profondeur (façade Y = 0 -> arrière), Z haut, dessus du fond en Z = 0.

/* [Rendu] */
part = "assembly"; // [assembly, open, cutaway, shell, plate]

/* [Coque] */
box_w = 48;        // largeur : l'écran (38 mm) + parois et jeu
box_d = 52;        // profondeur : Pro Mini devant, module USB/série derrière
box_h = 24;        // hauteur à l'arrière : RXB6 debout (14) + antenne hélicoïdale couchée au-dessus
front_h = 10;      // hauteur du bas de la façade
face_tilt = 40;    // façade : inclinaison depuis la verticale (40° : se lit assis, imprimable à l'envers)
wall = 2;
corner_r = 2;      // arrondi du profil

/* [Fond] */
plate_t = 2;
boss_d = 6;        // 3 colonnes de vis (coin avant droit pris par le RXB6)
boss_h = 8;
screw_pilot = 2.5; // M3 auto-taraudeuse dans la colonne
screw_clear = 3.4;
screw_head = 6;    // fraisage sous le fond
pocket_h = 1.5;    // rebord des logements de cartes (cartes posées + adhésif double face)
pocket_clr = 0.4;

/* [Pro Mini] */
pm_l = 33;         // 33 x 18, fils soudés directement (pas de barrettes), collé à plat (adhésif double face)
pm_w = 18;
pm_h = 3;          // composants compris
pm_pos = [21, 19]; // centre sur le fond, devant le module USB/série

/* [Module USB/série, micro-USB] */
usb_l = 20;        // À MESURER
usb_w = 20;        // À MESURER
usb_t = 1.6;       // épaisseur de la carte
usb_x = 24;        // centre en X ; la prise est au bord arrière de la carte, contre la paroi
usb_port = [8, 3]; // prise micro-USB (largeur, hauteur), dessus de la carte
usb_plug = [12, 8]; // ouverture dans la paroi pour la fiche surmoulée

/* [RXB6] */
rx_l = 30;         // RXB6 : carte debout le long de la paroi droite, broches en bas — À MESURER
rx_h = 14;
rx_t = 1.6;
rx_y = 11;         // début de la carte (côté façade)
slot_w = 2.2;      // rainures de maintien de la carte debout
slot_h = 6;

/* [OLED 0,91" 128x32] */
oled_l = 38;       // carte — À MESURER
oled_w = 12;
oled_t = 1.2;
oled_glass = 1.5;  // verre au-dessus de la carte
oled_aa = [22.4, 5.6]; // zone active
oled_aa_off = 3;   // décalage de la zone active vers la droite par rapport au centre de la carte (côté sans broches)
win_m = 1;         // marge de la fenêtre autour de la zone active
oled_pad = 3;      // surépaisseur de la façade autour du logement de l'écran

/* [Embase SMA] */
sma_d = 6.5;       // perçage de l'embase (méplat à 5.9) — À VÉRIFIER
sma_flat = 5.9;
sma_skin = 0.4;    // pellicule laissée côté extérieur : percer pour passer au fouet
sma_pos = [36, 14]; // X, Z sur la paroi arrière (entre la prise USB et le RXB6)

$fn = 48;
eps = 0.01;

// ---------- dérivées ----------
face_dy = (box_h - front_h) * tan(face_tilt);   // recul du haut de la façade
face_a = 90 - face_tilt;                          // angle de la façade depuis l'horizontale
face_len = (box_h - front_h) / cos(face_tilt);
profile = [[0, 0], [box_d, 0], [box_d, box_h], [face_dy, box_h], [0, front_h]];
bosses = [[wall + boss_d / 2, wall + boss_d / 2 + 1.5], [wall + boss_d / 2, box_d - wall - boss_d / 2], [box_w - wall - boss_d / 2, box_d - wall - boss_d / 2]];
usb_y = box_d - wall - usb_l / 2 - 0.3;           // carte contre la paroi arrière
usb_port_z = pocket_h + usb_t + usb_port[1] / 2;
rx_x = box_w - wall - 3.5;                        // plan de la carte RXB6

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
        union() { body(wall); translate([wall, wall, -1]) cube([box_w - 2 * wall, box_d - 2 * wall, front_h / 2 + 1]); }
      }
      // colonnes de vis
      for (p = bosses) translate([p[0], p[1], 0]) cylinder(d = boss_d, h = boss_h);
      // surépaisseur de façade autour du logement de l'écran
      intersection() {
        body();
        face_xf() translate([box_w / 2 - oled_l / 2 - 2, face_len / 2 - oled_w / 2 - 2, -wall - oled_pad]) cube([oled_l + 4, oled_w + 4, oled_pad + 1]);
      }
    }
    for (p = bosses) translate([p[0], p[1], -eps]) cylinder(d = screw_pilot, h = boss_h - 1);
    // fenêtre de l'écran et logement de la carte (verre affleurant sous la face)
    face_xf() {
      translate([box_w / 2 + oled_aa_off - oled_aa[0] / 2 - win_m, face_len / 2 - oled_aa[1] / 2 - win_m, -wall - 1]) cube([oled_aa[0] + 2 * win_m, oled_aa[1] + 2 * win_m, wall + 2]);
      translate([box_w / 2 - oled_l / 2 - pocket_clr, face_len / 2 - oled_w / 2 - pocket_clr, -wall - oled_pad - 1]) cube([oled_l + 2 * pocket_clr, oled_w + 2 * pocket_clr, oled_pad + 1 - 0.8 + eps]);
    }
    // prise micro-USB
    translate([usb_x - usb_plug[0] / 2, box_d - wall - 1, usb_port_z - usb_plug[1] / 2]) cube([usb_plug[0], wall + 2, usb_plug[1]]);
    // embase SMA défonçable : perçage à méplat, pellicule côté extérieur
    translate([sma_pos[0], box_d - wall - 1, sma_pos[1]]) rotate([-90, 0, 0]) intersection() {
      cylinder(d = sma_d, h = wall + 1 - sma_skin);
      translate([-sma_d / 2, -sma_flat / 2, 0]) cube([sma_d, sma_flat, wall + 1]);
    }
  }
}

// ---------- fond ----------
module pocket(c, l, w) {   // rebord autour d'une carte posée, centre c, l en X, w en Y
  translate([c[0], c[1], 0]) difference() {
    translate([-l / 2 - pocket_clr - 1.2, -w / 2 - pocket_clr - 1.2, 0]) cube([l + 2 * pocket_clr + 2.4, w + 2 * pocket_clr + 2.4, pocket_h]);
    translate([-l / 2 - pocket_clr, -w / 2 - pocket_clr, -1]) cube([l + 2 * pocket_clr, w + 2 * pocket_clr, pocket_h + 2]);
  }
}

module plate() {
  difference() {
    union() {
      translate([0, 0, -plate_t]) linear_extrude(plate_t) offset(r = corner_r) offset(delta = -corner_r) square([box_w, box_d]);
      // logements : rognés à l'intérieur des parois (la carte USB/série est plaquée contre la paroi arrière)
      intersection() {
        pocket([usb_x, usb_y], usb_w, usb_l);
        translate([wall + 0.2, wall + 0.2, 0]) cube([box_w - 2 * wall - 0.4, box_d - 2 * wall - 0.4, pocket_h]);
      }
      // rainures de la carte RXB6 debout (deux blocs, carte entre les deux)
      for (y = [rx_y, rx_y + rx_l - 6]) translate([rx_x - rx_t / 2 - 2.5, y, 0])
        difference() { cube([rx_t + 5, 6, slot_h]); translate([2.5 - (slot_w - rx_t) / 2, -1, 1]) cube([slot_w, 8, slot_h]); }
    }
    for (p = bosses) translate([p[0], p[1], -plate_t - 1]) {
      cylinder(d = screw_clear, h = plate_t + 2);
      cylinder(d1 = screw_head, d2 = screw_clear, h = (screw_head - screw_clear) / 2);   // fraisage 90°
    }
  }
}

// ---------- modules (gabarits) ----------
module promini() { color("#1a5fa0") translate([pm_pos[0] - pm_l / 2, pm_pos[1] - pm_w / 2, 0]) { cube([pm_l, pm_w, 1]); color("#222") translate([8, 4, 1]) cube([10, 10, pm_h - 1]); } }
module usbserial() {
  translate([usb_x - usb_w / 2, usb_y - usb_l / 2, 0]) { color("#7a2a8a") cube([usb_w, usb_l, usb_t]);
    color("Silver") translate([usb_w / 2 - usb_port[0] / 2, usb_l - 5, usb_t]) cube([usb_port[0], 5.3, usb_port[1]]); }
}
module rxb6() {
  color("#2a7a3a") translate([rx_x - rx_t / 2, rx_y, 1]) cube([rx_t, rx_l, rx_h]);
  color("Gold") translate([rx_x - 3, rx_y + 2, 1 + rx_h + 3]) rotate([-90, 0, 0]) cylinder(d = 5, h = 30);   // hélicoïdale couchée au-dessus du RXB6
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
  promini(); usbserial(); rxb6(); oled();
}

assert(usb_port_z + usb_plug[1] / 2 < box_h - wall, "Prise USB trop haute");
echo(str("Pupitre ", box_w, " x ", box_d, " x ", box_h + plate_t, " mm ; façade à ", face_a, "° de l'horizontale, ", round(face_len), " mm de long"));

if (part == "shell") translate([0, 0, box_h]) rotate([180, 0, 0]) shell();   // dessus sur le plateau
else if (part == "plate") translate([0, 0, plate_t]) plate();
else if (part == "open") {   // vue ouverte : coque soulevée au-dessus du fond et des cartes
  translate([0, 0, 40]) color("#e8e2d0") shell();
  color("#c8c0a8") plate();
  promini(); usbserial(); rxb6();
  translate([0, 0, 40]) oled();
}
else if (part == "cutaway") {   // coque coupée à mi-largeur : disposition intérieure
  color("#e8e2d0") difference() { shell(); translate([-1, -1, -1]) cube([box_w / 2 + 1, box_d + 2, box_h + 2]); }
  color("#c8c0a8") plate();
  promini(); usbserial(); rxb6(); oled();
}
else assembled();
