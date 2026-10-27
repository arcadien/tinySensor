// Représentation simplifiée de la carte TinySensor v2.2 et de son accu 18650, pour les rendus.
// Positions relevées dans hardware/tinysensor/tinySensorv1.kicad_pcb (coordonnées KiCad absolues),
// volumes approximatifs (boîtes et cylindres), pas destinés à l'impression.
//
// Repère : coin bas-gauche du PCB, Y vers le haut (KiCad : Y vers le bas), dessous du PCB en Z = 0.

pcb_w = 93.98;
pcb_h = 63.754;
pcb_t = 1.6;
k_origin = [113.04016, 48.02632];   // coin haut-gauche du contour Edge.Cuts
k_holes = [[203.75016, 105.51032], [118.04904, 52.81676], [117.61216, 105.43032], [203, 52], [150.88616, 51.83632]];

show_battery = true;
// découpe le long des pointillés de sérigraphie (x = 147, y = 87 en KiCad) : on ne garde que
// la partie sans support 18650 ni support CR2032 ; l'accu arrive alors sur J13 « RAW IN »
k_crop = [[147, 48.02632], [207.02016, 87]];
crop_w = k_crop[1][0] - k_crop[0][0];
crop_h = k_crop[1][1] - k_crop[0][1];
// trous restants sur la carte découpée (MH5 et MH1 du haut), coordonnées KiCad
k_crop_holes = [[150.88616, 51.83632], [203, 52]];

$fn = 32;

// Place une empreinte KiCad (position absolue, angle) sur le dessus du PCB.
// Les enfants sont décrits en coordonnées locales de l'empreinte, Y vers le haut (= -Y KiCad).
module kfp(x, y, a = 0) {
  translate([x - k_origin[0], pcb_h - (y - k_origin[1]), pcb_t]) rotate([0, 0, a]) children();
}

module pins(pts, h = 8) { color("Gold") for (p = pts) translate([p[0] - 0.32, p[1] - 0.32, 0]) cube([0.64, 0.64, h]); }
module header(n, a = 0) {
  color("#222") translate([-1.27, -(n - 1) * 2.54 - 1.27, 0]) cube([2.54, n * 2.54, 2.5]);
  pins([for (i = [0 : n - 1]) [0, -i * 2.54]]);
}
module radial(d, h, p, c = "#1b3d8f") { color(c) translate([p / 2, 0, 0]) cylinder(d = d, h = h); }
module axial(l = 3.6, d = 1.6, p = 7.62, c = "Tan") { color(c) translate([p / 2 - l / 2, 0, d / 2 + 0.3]) rotate([0, 90, 0]) cylinder(d = d, h = l); }

module holder_18650() {
  // MPD BH-18650-PC2 : fab -2.8..74.8 x -9.85..9.85, bornes à 0 et 72
  color("#202020") {
    translate([-2.8, -9.85, 0]) cube([77.6, 19.7, 1.5]);
    for (y = [-9.85, 9.85 - 1.5]) translate([-2.8, y, 0]) cube([77.6, 1.5, 7]);
    for (x = [-2.8, 74.8 - 4]) translate([x, -9.85, 0]) cube([4, 19.7, 20]);
  }
  if (show_battery) {
    translate([3.4, 0, 1.5 + 9.3]) rotate([0, 90, 0]) {
      color("#2e6fd8") cylinder(d = 18.6, h = 64);
      color("Silver") translate([0, 0, 64]) cylinder(d = 7, h = 1.2);
    }
  }
}

// carte découpée, centrée sur l'origine (dessous du PCB en Z = 0)
module pcb_v22_cropped(jst_eh = true) {   // jst_eh = false : JST « I2C » et « I2C_SPI » non posés (champignon)
  translate([-(k_crop[0][0] - k_origin[0]) - crop_w / 2, -(pcb_h - (k_crop[1][1] - k_origin[1])) - crop_h / 2, 0])
    intersection() {
      pcb_v22(holder = false, jst_eh = jst_eh);
      translate([k_crop[0][0] - k_origin[0], pcb_h - (k_crop[1][1] - k_origin[1]), -1]) cube([crop_w, crop_h, 60]);
    }
}
// trous de la carte découpée dans le repère de pcb_v22_cropped()
function crop_holes() = [for (h = k_crop_holes) [h[0] - (k_crop[0][0] + k_crop[1][0]) / 2, (k_crop[0][1] + k_crop[1][1]) / 2 - h[1]]];

module pcb_v22(holder = true, jst_eh = true) {   // holder = false : support 18650 non peuplé
  color("#1f7a3a") difference() {
    cube([pcb_w, pcb_h, pcb_t]);
    for (h = k_holes) translate([h[0] - k_origin[0], pcb_h - (h[1] - k_origin[1]), -1]) cylinder(d = 2.7, h = pcb_t + 2);
  }
  // alimentation : support 18650 et convertisseur DC/DC
  if (holder) kfp(124.72416, 99.08032) holder_18650();
  kfp(147.56966, 58.78882) {
    pins([[1.96, 1.54], [4.5, 1.54], [7.04, 1.54]], 6);
    color("#1a4fa0") translate([0, 0, 5]) cube([19, 9, 1.2]);
    color("#333") translate([11, 2, 6.2]) cube([5, 5, 3]);
  }
  // ATtiny84A (DIP-14)
  kfp(168.92, 70.12, 90) color("#111") translate([0.6, -16.5, 0.5]) cube([6.4, 17.8, 3.3]);
  // bouton, condensateurs
  kfp(153.53016, 68.09232) {
    color("#444") translate([0.25, -5.25, 0]) cube([6, 6, 3.5]);
    color("#111") translate([3.25, -2.25, 0]) cylinder(d = 3.5, h = 4.3);
  }
  kfp(162.67176, 61.74232, 180) radial(5, 11, 2.5);
  kfp(194, 70, 180) radial(5, 11, 2.5);
  kfp(170.36796, 51.35372) radial(4, 7, 1.5);
  kfp(155.92044, 61.60008, 180) radial(4.5, 8, 2.5, "Orange");
  kfp(164.09416, 54.63032, -90) radial(4.5, 8, 2.5, "Orange");
  kfp(165.5, 64, -90) color("#c8a000") translate([-1, -1, 0]) cube([7, 2, 6]);
  // résistances, diodes, LED
  for (r = [[179.96444, 75.3934, -90], [182.36444, 75.3934, -90], [162.06216, 84.60232, 90], [185.78068, 57.99836, 90], [175.47336, 55.45836, 180]])
    kfp(r[0], r[1], r[2]) axial();
  for (d = [[182.58028, 58.07456, 90], [180.01996, 50.49012, -90]]) kfp(d[0], d[1], d[2]) axial(3.8, 1.9, 7.62, "#8a1c1c");
  kfp(157.99816, 81.55932, 90) color("Red") translate([1.27, 0, 0]) cylinder(d = 1.8, h = 3);
  // connecteurs
  if (jst_eh) for (y = [83.91032, 77.11032]) kfp(199.35016, y, 180) {
    color("WhiteSmoke") translate([-2.5, -2.3, 0]) cube([17.5, 4.0, 6]);
    pins([for (i = [0 : 5]) [i * 2.5, 0]], 3);
  }
  kfp(151.67356, 77.99324, -90) color("#1f6fd0") translate([-2.71, -4, 0]) cube([10.5, 8, 10]);
  kfp(164, 85) {
    color("#222") translate([3.18, 0.9, 0]) cube([7.62, 5.08, 2.5]);
    pins([for (x = [4.45428, 6.99428, 9.53428], y = [2.17488, 4.71488]) [x, y]]);
  }
  for (h = [[192.99936, 56.86044, 0, 3], [174.17796, 58.59272, -90, 3], [188.9506, 51.181, 90, 3],
            [199.75576, 66.79692, 0, 2], [199.71004, 56.61152, 0, 2], [171.20616, 73.68032, -90, 2]])
    kfp(h[0], h[1], h[2]) header(h[3]);
}

pcb_v22();
