// 18650 cradle geometry shared by the mushroom stem cradle (mushroom/mushroom.scad, part="cradle")
// and the wall box trough (basic/tinysensor_box.scad). Load it with include<>: it holds parameters,
// no top-level geometry.
//
// Cradle frame: X = cell axis (X = 0 at the "−" end), Y centred on the axis, Z up, cell axis at Z = cl_z.
// One tab contact per end, slid into a groove, tab up; the wire soldered on it leaves through the nearby
// notch. "−" spring at X = 0, "+" bump at X = cr_len.

/* [Cell and tab contacts] */
cell_d = 18.6;     // bare 18650, flat top (protected / button top: up to ~70)
cell_l = 65;
bt_w = 13;         // tab contacts: 13 x 11 plate, 3 x 7 tab (drilled)
bt_h = 11;
bt_t = 0.4;        // sheet thickness (to measure)
bt_tab = [3, 7];   // tab up (open side), wire soldered on it
wire_notch = [4.5, 3];  // wire pass above each end: width, depth below the top of the cell
bt_dimple = 1.8;   // bump of the "+" contact
spring_c = 4;      // spring of the "−" contact compressed (10.7 free)
bt_slot = 0.8;     // plate groove
bt_sink = 1.5;     // groove deeper than the plate (1st print: tab 1 mm above the edge; +0.4 for cr_fit 0.8)
bt_win = 10.5;     // window in front of the plate (room for the spring / the bump)

cr_end = 1.6 + bt_slot + 1;         // end: outer wall + groove + inner wall
cell_x0 = 1.6 + bt_t + spring_c;    // bottom of the cell (spring compressed)
cr_len = cell_x0 + cell_l + bt_dimple + bt_t + 1.6;

// Volumes to subtract from a cradle body: cell bore with its insertion slot (above the axis, +1 mm),
// then per end the plate groove, the wire notch and the window in front of the plate; h = cut height.
module cradle_cell_cuts(cl_z, h) {
  translate([cr_end, 0, cl_z]) rotate([0, 90, 0]) cylinder(d = cell_d + 0.6, h = cr_len - 2 * cr_end);
  translate([cr_end, -cell_d / 2 - 0.3, cl_z + 1]) cube([cr_len - 2 * cr_end, cell_d + 0.6, h]);
  for (s = [0, 1]) {
    xs = s == 0 ? 1.6 : cr_len - 1.6 - bt_slot;   // groove
    // plate groove, open at the top (tab up)
    translate([xs, -(bt_w + 0.6) / 2, cl_z - bt_h / 2 - bt_sink]) cube([bt_slot, bt_w + 0.6, h]);
    // wire notch next to the tab, through the end walls, up to above the cell
    translate([s == 0 ? -1 : cr_len - cr_end - 1, -(bt_w + 0.6) / 2 - wire_notch[0] + 2, cl_z + cell_d / 2 - wire_notch[1]])
      cube([cr_end + 2, wire_notch[0], h]);
    // window in front of the plate
    translate([s == 0 ? xs + bt_slot - eps : cr_len - cr_end - eps, -bt_win / 2, cl_z - bt_h / 2]) cube([1 + 2 * eps, bt_win, h]);
  }
}
// polarity, engraved on the end faces
module cradle_polarity(cl_z) {
  for (m = [[0, "-"], [1, "+"]]) translate([m[0] * cr_len, 0, cl_z]) mirror([m[0], 0, 0]) rotate([90, 0, -90])
    translate([0, 0, -0.6]) linear_extrude(1) text(m[1], size = 8, halign = "center", valign = "center");
}
// mock-up: cell and the two contacts (plate + tab)
module cradle_cell_mock(cl_z) {
  color("#2e6fd8") translate([cell_x0, 0, cl_z]) rotate([0, 90, 0]) cylinder(d = cell_d, h = cell_l);
  color("Silver") for (x = [1.6, cr_len - 1.6 - bt_t]) translate([x, -bt_w / 2, cl_z - bt_h / 2 - bt_sink]) {
    cube([bt_t, bt_w, bt_h]);
    translate([0, (bt_w - bt_tab[0]) / 2, bt_h]) cube([bt_t, bt_tab[0], bt_tab[1]]);
  }
}
