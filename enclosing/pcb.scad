// Simplified TinySensor boards, for the renders: v3.0 (current, pcb_v3 / pcb_v3_cropped) and v2.2
// (pcb_v22 / pcb_v22_cropped, boards still in use). Positions read from hardware/tinysensor/tinysensor.kicad_pcb
// (absolute KiCad coordinates; the v2.2 ones from its last v2.2 revision), approximate volumes (boxes and
// cylinders), not meant for printing.
//
// Frame: bottom-left corner of the PCB, Y up (KiCad: Y down), PCB underside at Z = 0.

pcb_t = 1.6;

/* [v3.0 board: 94.81 x 39.334, no 18650 holder, CR2032 holder J10 on the left] */
v3_origin = [113.04016, 48.02632];   // top-left corner of the Edge.Cuts outline
v3_w = 207.85016 - v3_origin[0];
v3_h = 87.36032 - v3_origin[1];
v3_holes = [[203, 52], [151.35016, 52.11032], [117.85016, 52.86032]];   // MH1, MH2, MH3 (M3, Ø3.2)
// cut along the silkscreen dotted line x = 147: the right part (without J10 nor MH3) is kept, full height
v3_crop = [[147, v3_origin[1]], [207.85016, 87.36032]];
v3_crop_w = v3_crop[1][0] - v3_crop[0][0];
v3_crop_h = v3_crop[1][1] - v3_crop[0][1];
v3_crop_holes = [[151.35016, 52.11032], [203, 52]];   // MH2, MH1

/* [v2.2 board: 93.98 x 63.754, with its 18650 holder] */
v22_w = 93.98;
v22_h = 63.754;
v22_origin = [113.04016, 48.02632];
v22_holes = [[203.75016, 105.51032], [118.04904, 52.81676], [117.61216, 105.43032], [203, 52], [150.88616, 51.83632]];

show_battery = true;
// cut along the silkscreen dotted lines (x = 147, y = 87 in KiCad): only the part without
// the 18650 and CR2032 holders is kept; the cell then arrives on J13 "RAW IN"
v22_crop = [[147, 48.02632], [207.02016, 87]];
v22_crop_w = v22_crop[1][0] - v22_crop[0][0];
v22_crop_h = v22_crop[1][1] - v22_crop[0][1];
// holes left on the cut board (MH5 and top MH1), KiCad coordinates
v22_crop_holes = [[150.88616, 51.83632], [203, 52]];

$fn = 32;

// Places a KiCad footprint (absolute position, angle) on top of the PCB.
// Children are given in the footprint's local coordinates, Y up (= -Y KiCad).
module kfp(x, y, a = 0, o = v22_origin, h = v22_h) {
  translate([x - o[0], h - (y - o[1]), pcb_t]) rotate([0, 0, a]) children();
}
module kfp3(x, y, a = 0) kfp(x, y, a, v3_origin, v3_h) children();

module pins(pts, h = 8) { color("Gold") for (p = pts) translate([p[0] - 0.32, p[1] - 0.32, 0]) cube([0.64, 0.64, h]); }
module header(n, a = 0) {
  color("#222") translate([-1.27, -(n - 1) * 2.54 - 1.27, 0]) cube([2.54, n * 2.54, 2.5]);
  pins([for (i = [0 : n - 1]) [0, -i * 2.54]]);
}
module radial(d, h, p, c = "#1b3d8f") { color(c) translate([p / 2, 0, 0]) cylinder(d = d, h = h); }
module axial(l = 3.6, d = 1.6, p = 7.62, c = "Tan") { color(c) translate([p / 2 - l / 2, 0, d / 2 + 0.3]) rotate([0, 90, 0]) cylinder(d = d, h = l); }

module holder_18650() {
  // MPD BH-18650-PC2: fab -2.8..74.8 x -9.85..9.85, terminals at 0 and 72
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

// cut board, centred on the origin (PCB underside at Z = 0)
module pcb_v22_cropped(jst_eh = true) {   // jst_eh = false: "I2C" and "I2C_SPI" JSTs not fitted (mushroom)
  translate([-(v22_crop[0][0] - v22_origin[0]) - v22_crop_w / 2, -(v22_h - (v22_crop[1][1] - v22_origin[1])) - v22_crop_h / 2, 0])
    intersection() {
      pcb_v22(holder = false, jst_eh = jst_eh);
      translate([v22_crop[0][0] - v22_origin[0], v22_h - (v22_crop[1][1] - v22_origin[1]), -1]) cube([v22_crop_w, v22_crop_h, 60]);
    }
}
// holes of the cut board in the pcb_v22_cropped() frame
function crop_holes() = [for (h = v22_crop_holes) [h[0] - (v22_crop[0][0] + v22_crop[1][0]) / 2, (v22_crop[0][1] + v22_crop[1][1]) / 2 - h[1]]];

module pcb_v22(holder = true, jst_eh = true) {   // holder = false: 18650 holder not fitted
  color("#1f7a3a") difference() {
    cube([v22_w, v22_h, pcb_t]);
    for (h = v22_holes) translate([h[0] - v22_origin[0], v22_h - (h[1] - v22_origin[1]), -1]) cylinder(d = 2.7, h = pcb_t + 2);
  }
  // power: 18650 holder and DC/DC converter
  if (holder) kfp(124.72416, 99.08032) holder_18650();
  kfp(147.56966, 58.78882) {
    pins([[1.96, 1.54], [4.5, 1.54], [7.04, 1.54]], 6);
    color("#1a4fa0") translate([0, 0, 5]) cube([19, 9, 1.2]);
    color("#333") translate([11, 2, 6.2]) cube([5, 5, 3]);
  }
  // ATtiny84A (DIP-14)
  kfp(168.92, 70.12, 90) color("#111") translate([0.6, -16.5, 0.5]) cube([6.4, 17.8, 3.3]);
  // button, capacitors
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
  // resistors, diodes, LED
  for (r = [[179.96444, 75.3934, -90], [182.36444, 75.3934, -90], [162.06216, 84.60232, 90], [185.78068, 57.99836, 90], [175.47336, 55.45836, 180]])
    kfp(r[0], r[1], r[2]) axial();
  for (d = [[182.58028, 58.07456, 90], [180.01996, 50.49012, -90]]) kfp(d[0], d[1], d[2]) axial(3.8, 1.9, 7.62, "#8a1c1c");
  kfp(157.99816, 81.55932, 90) color("Red") translate([1.27, 0, 0]) cylinder(d = 1.8, h = 3);
  // connectors
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

// ---------- v3.0 ----------
module disc(c = "Orange") color(c) translate([1.25, 0, 3.6]) rotate([90, 0, 0]) cylinder(d = 3, h = 1.6, center = true);
module axial_v(p = 1.9) {   // resistor standing, body over pad 1, lead looping back to pad 2
  color("Tan") translate([0, 0, 0.5]) cylinder(d = 1.8, h = 3.8);
  color("Silver") translate([0, -0.25, 0]) cube([p, 0.5, 5.3]);
}
module jst_eh6() {
  color("WhiteSmoke") translate([-2.5, -2.3, 0]) cube([17.5, 4.0, 6]);
  pins([for (i = [0 : 5]) [i * 2.5, 0]], 3);
}

module pcb_v3(jst_eh = true) {   // jst_eh = false: J3 "I2C" and J14 JSTs not fitted (mushroom)
  color("#1f5a32") difference() {
    cube([v3_w, v3_h, pcb_t]);
    for (h = v3_holes) translate([h[0] - v3_origin[0], v3_h - (h[1] - v3_origin[1]), -1]) cylinder(d = 3.2, h = pcb_t + 2);
  }
  // CR2032 holder (Keystone 1060), contacts at ±14.65 along its local X
  kfp3(130.25016, 67.26032, 90) {
    color("#111") translate([-12.5, -11, 0]) cube([25, 22, 5]);
    color("Silver") translate([0, 0, 1]) cylinder(d = 20, h = 3.2);
  }
  // DC/DC module (TPS63020 board) on the J2 header, as on v2.2: removable to reach the MH2 screw
  kfp3(149.31016, 57.86032) {
    pins([[0, 0], [2.54, 0], [5.08, 0]], 6);
    color("#1a4fa0") translate([-1.96, -1.54, 5]) cube([19, 9, 1.2]);
    color("#333") translate([9.04, 0.46, 6.2]) cube([5, 5, 3]);
  }
  // ATtiny84A (DIP-14), Q1
  kfp3(168.92, 70.12, 90) color("#111") translate([0.6, -16.5, 0.5]) cube([6.4, 17.8, 3.3]);
  kfp3(149.85016, 71.17282, -90) color("#111") translate([-1.5, -0.7, 0]) cube([3, 1.4, 1.1]);
  // button, capacitors
  kfp3(153.53016, 68.09232) {
    color("#444") translate([0.25, -5.25, 0]) cube([6, 6, 3.5]);
    color("#111") translate([3.25, -2.25, 0]) cylinder(d = 3.5, h = 4.3);
  }
  kfp3(161.839228, 62.854276, 180) radial(6.3, 11.5, 2.5);   // C1, tallest part on the board
  kfp3(170.36796, 51.35372) radial(4, 7, 1.5);                 // C3
  for (c = [[165.5, 64, -90], [161.949425, 55.368239, 90]])    // C2, C7 film 7 x 2
    kfp3(c[0], c[1], c[2]) color("#c0201c") translate([-1, -1, 0]) cube([7, 2, 7]);
  for (c = [[196.35016, 59.288285, 90], [165.492048, 55.468742, -90], [155.92044, 61.60008, 180]])   // C4, C6, C5
    kfp3(c[0], c[1], c[2]) disc();
  // resistors, diodes, LED
  for (r = [[179.96444, 75.3934, -90], [182.36444, 75.3934, -90], [185.78068, 57.99836, 90], [175.47336, 55.45836, 180]])
    kfp3(r[0], r[1], r[2]) axial();
  for (r = [[157.60016, 83.11032, 0], [192.685273, 67.06532, -90], [189.360062, 67.010358, -90],
            [158.85016, 50.61032, 180], [156.94516, 53.36032, 0], [162.60016, 84.11032, 90]])
    kfp3(r[0], r[1], r[2]) axial_v();
  for (d = [[182.58028, 58.07456, 90], [180.01996, 50.49012, -90]]) kfp3(d[0], d[1], d[2]) axial(3.8, 1.9, 7.62, "#8a1c1c");
  kfp3(158.07175, 77.931107) color("Red") translate([1.27, 0, 0]) {   // D1, 5 mm LED
    cylinder(d = 5.8, h = 1);
    cylinder(d = 5, h = 6.1);
    translate([0, 0, 6.1]) sphere(d = 5);
  }
  // connectors
  if (jst_eh) for (y = [83.91032, 77.11032]) kfp3(199.35016, y, 180) jst_eh6();
  kfp3(151.62516, 79.86032, -90) {   // J13 "RAW IN", JST-XH B2B-XH-A vertical
    color("WhiteSmoke") translate([-2.45, -3.4, 0]) cube([7.4, 5.75, 7]);
    pins([[0, 0], [2.5, 0]], 3);
  }
  kfp3(164, 85) {
    color("#222") translate([3.18, 0.9, 0]) cube([7.62, 5.08, 2.5]);
    pins([for (x = [4.45428, 6.99428, 9.53428], y = [2.17488, 4.71488]) [x, y]]);
  }
  for (h = [[192.99936, 56.86044, 0, 3], [174.17796, 58.59272, -90, 3], [188.9506, 51.181, 90, 3],
            [199.75576, 66.79692, 0, 2], [201.35016, 57.57032, 0, 2], [171.20616, 73.68032, -90, 2]])
    kfp3(h[0], h[1], h[2]) header(h[3]);
}

// v3.0 board cut at x = 147 (MH2 and MH1 kept), centred on the origin (PCB underside at Z = 0)
module pcb_v3_cropped(jst_eh = true) {
  translate([-(v3_crop[0][0] - v3_origin[0]) - v3_crop_w / 2, -v3_crop_h / 2, 0])
    intersection() {
      pcb_v3(jst_eh = jst_eh);
      translate([v3_crop[0][0] - v3_origin[0], -1, -1]) cube([v3_crop_w, v3_crop_h + 2, 60]);
    }
}
// holes of the cut v3.0 board in the pcb_v3_cropped() frame (X right, Y up)
function crop_holes_v3() = [for (h = v3_crop_holes) [h[0] - (v3_crop[0][0] + v3_crop[1][0]) / 2, (v3_crop[0][1] + v3_crop[1][1]) / 2 - h[1]]];

pcb_v3();
