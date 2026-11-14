// TinySensor "mushroom" — decorative enclosure (fly agaric), alongside the wall box.
// Units: mm. Material: PETG.
//
// The cap is one piece (dome + gills) sitting on the stem with a slight tilt;
// all the electronics are in the cap, the stem only carries the cell.
//
//   cap     : gourd-shaped dome (sketch), wavy edge; 3 inner corbels above the disc, M3 screws from below (self-tapping)
//   gills   : gill disc = underside of the cap; vent slots, USB socket, sleeve on the stem
//   chassis : frame printed upright, screwed onto the disc: ventilated lower chamber (BME280 plugged under the board),
//             deck, cut-down v2.2 board, charge module slide (socket facing down, between two gills)
//   deck    : perforated deck between the chamber and the board, I2C connector window (female header through it)
//   stem    : hollow stem (upright 18650 cradle), lobed, bulbous base, cup ring, tilted tenon at the top
//   ground  : 1 mm disc under the stem, with grass tufts (stability + decoration)
//   cradle  : removable cell cradle, slid into the stem; two tab contacts (cell not soldered)
//   plug    : plug of the ballast chamber (under the cradle), glued into the counterbore of the bottom
//   dot     : white dots added afterwards, glued into the dome recesses
//
// Export: python3 update_3mf.py updates the parts in the Orca project print/mushroom.3mf (plates kept).
// By hand (every part in its print orientation):
//   openscad -D 'part="cap"'     -o mushroom_cap.3mf     mushroom.scad   # edge on the bed, no support
//   openscad -D 'part="gills"'   -o mushroom_gills.3mf   mushroom.scad   # gills up
//   openscad -D 'part="chassis"' -o mushroom_chassis.3mf mushroom.scad
//   openscad -D 'part="deck"'    -o mushroom_deck.3mf    mushroom.scad
//   openscad -D 'part="stem"'    -o mushroom_stem.3mf    mushroom.scad
//   openscad -D 'part="ground"'  -o mushroom_ground.3mf  mushroom.scad   # ground + grass, in green; stem glued on it
//   openscad -D 'part="cradle"'  -o mushroom_cradle.3mf  mushroom.scad   # flat, flat face on the bed
//   openscad -D 'part="plug"'    -o mushroom_plug.3mf    mushroom.scad
//   openscad -D 'part="dot"' -D 'dot_d=8' -o mushroom_dot8.3mf mushroom.scad   # x n, see the echo
// Renders:
//   openscad --preview -D 'part="assembly"' --camera=10,0,145,75,0,25,950 --imgsize=1000,1300 -o rendu_champignon.png mushroom.scad
//   openscad --preview -D 'part="assembly"' --camera=0,0,90,112,0,200,720 --imgsize=1000,1000 -o rendu_champignon_dessous.png mushroom.scad
//   openscad --preview -D 'part="assembly"' --camera=0,0,30,65,0,40,260 --imgsize=1000,800 -o rendu_champignon_pied.png mushroom.scad
//   openscad --preview -D 'part="exploded"' --camera=0,0,330,70,0,25,1950 --imgsize=1000,1800 -o rendu_champignon_eclate.png mushroom.scad
//   openscad --preview -D 'part="cradle_view"' --camera=40,20,10,55,0,30,230 --imgsize=1200,800 -o rendu_champignon_berceau.png mushroom.scad
//   openscad --preview -D 'part="section"'  --camera=0,0,150,90,0,0,850 --imgsize=1000,1300 -o rendu_champignon_coupe.png mushroom.scad
//   openscad --preview -D 'part="sensor"'   --camera=-18,5,166,96,0,0,150 --imgsize=1200,800 -o rendu_champignon_capteur.png mushroom.scad
//   openscad --preview -D 'part="console"'  --camera=56,0,5,75,0,-20,95 --imgsize=1200,800 -o rendu_champignon_console.png mushroom.scad   # ~4 min
//
// Frames: world = Z up, stem axis at X = Y = 0, Z = 0 on the table.
// Cap (cap_xf): Z = 0 under the gill disc, axis tilted by cap_tilt towards +X.
// Dome: Z = 0 at its edge, cap_skirt below the disc. The top of the gourd is centred on J1.

use <pcb.scad>
use <euro2.scad>

/* [Render] */
part = "assembly"; // [assembly, exploded, cradle_view, section, sensor, console, cap, gills, chassis, deck, stem, ground, cradle, plug, dot]
dot_d = 8;         // diameter of the exported dot (part = "dot")
explode = 40;      // exploded view: gap between levels (part = "exploded")
cap_color = "#c0392b"; // render only (fly agaric; panther cap: "#8b5a2b")

/* [Cut-down v2.2 board — see pcb.scad] */
crop_w = 60.02;
crop_h = 38.974;
pcb_t = 1.6;
pcb_holes = [[26.1243, -15.6770], [-25.9902, -15.5130]];   // MH5, top MH1, board turned 180°
k_crop_c = [177.01008, 67.51316];  // centre of the cut (KiCad): local = [x - cx, cy - y]
i2c_k = [199.35016, 77.11032];     // "I2C" footprint (JST-EH 6, 2.50 pitch), pin 1; pins towards +X once the board is turned
radio_k = [192.99936, 56.86044];   // J1 "RADIO" (1x3, 433 MHz module standing): the dome top is right above it

/* [Cap] */
cap_tilt = 6;      // tilt of the cap on the stem
cap_R = 72;        // average max radius of the cap (bottom of the gourd)
cap_H = 190;       // dome height above its edge
// gourd meridian, after the sketch: [radius / cap_R, height / cap_H], from edge to top (increasing heights)
cap_shape = [[0.86, 0], [0.97, 0.08], [1, 0.16], [0.93, 0.29], [0.68, 0.41], [0.45, 0.5], [0.38, 0.58],
             [0.43, 0.71], [0.5, 0.84], [0.45, 0.94], [0.25, 0.99], [0, 1]];
cap_lean = 8;      // the top leans towards -X (against cap_tilt), pivoting where the cap narrows
cap_neck = [0.4, 0.62];  // neck: offset (computed to put the top over J1) and lean set in between these two heights (/ cap_H)
cap_t = 2.4;
vent_d = 5;        // vent: from the top of the ceiling to the flank, going down (rain stays out, warm air leaves)
vent_at = [90, 72]; // vent exit on the surface: (phi, t) of cap_pt, rear side
vent_clear = 14;   // no dot closer than this to the exit
win_h = 22;        // chassis windows, at the top of the chamber: air flows past the BME280
rim_w = 6;         // edge width on the bed: the bottom of the wall thickens inwards (adhesion, very tall cap)
cap_skirt = 4;     // the dome edge drops below the disc (hides its edge; the disc must get through)
disc_fit = 0.4;    // radial clearance between the disc and the narrowest part of the dome skirt
corbel_w = 12;     // dome corbels, above the disc: width (around the screw)
corbel_h = 10;     // height of the straight face, 45° chamfer above up to the wall
corbel_in = 5;     // corbel overhang beyond the screw axis, inwards
corbel_pilot = 8;  // pilot hole depth (M3x10 self-tapping: 3 of disc + 7 engaged)
tab_base = [90, 210, 330];  // dome screws (corbels): each slides (±40°) towards the nearest lobe
dot_sizes = [6, 8, 10];
dot_bag = [6, 6, 6, 8, 8, 8, 10, 10, 10, 10, 10];  // dot size draw: large ones favoured
dot_pocket = 0.8;
dot_gap = 12;      // minimum gap between two dots

/* [Cap outline: lobe generator] */
lobe_seed = 6;     // seed: change for another shape
lobe_n = 4.5;      // dominant lobe count
lobe_amp = 0.22;   // max deviation from the mean radius, at the edge (0.22 = ±22 %)
lobe_top = 0.3;    // share of the deformation left at the top (1 = bumpy all the way up)
lobe_rot = 165;    // outline orientation in degrees; -1 = automatic (best clearance around the electronics)
clear_m = 3;       // target minimum clearance between the electronics and the dome (clearance index)

/* [Gills] */
disc_t = 3;
collar_h = 20;     // sleeve under the disc, takes the stem tenon
collar_r = 22.5;   // outer radius of the sleeve (= top of the stem)
collar_screw_a = 0; // radial sleeve screw: under the USB socket (+X), where the cup dips
tenon_r = 19.5;
disc_bore = 18.5;   // central hole of the disc: the top of the cradle goes through, slanted by cap_tilt
n_gills = 48;
gill_t = 1.2;
gill_depth = [17, 3];  // gill depth near the stem / at the edge

/* [Chassis] */
chamber_h = 48;    // lower chamber: BME280 hanging under the board, top of the cradle and JST-XH pair
deck_t = 1.6;
pcb_gap = 5;       // deck -> underside of the board (through-hole pins)
ch_wall = 2;
bme_w = 15;        // BMx280 breakout 15 x 11, 6 pins VIN GND SCL SDA CSB SDO centred on the width
bme_d = 11;
bme_t = 1.6;
bme_pins = 6;
bme_edge = 1.3;    // pin row -> breakout edge (body towards +Y)
hdr_p = 2.54;      // 2.54 pitch headers
fh_n = 6;          // female header under the board: 1 SENSOR_VCC, 2 GND, 3 SCL, 4 SDA, 5 NC (CSB), 6 GND (SDO -> 0x76)
fh_h = 8.5;        // height of the female header under the board
mh_h = 2.5;        // plastic of the breakout's male header
mh_pin = 6;        // male pins above the plastic (drawn if fh_n < bme_pins)
fh_clr = 1;        // clearance of the deck window around the headers
chg_w = 17.15;     // protected TP4056 charge module, measured (Tars): 28.7 including socket and break-off tabs
chg_l = 28.7;
chg_t = 0.9;
chg_x = 40;        // plane of the module board
usb_cut = [8.5, 12];

/* [Stem] */
stem_h = 95;       // centre of the top of the stem (under the sleeve)
bore_r = 14.7;     // cell bore (Ø29.4, takes the cradle)
foot_t = 3;        // stem bottom
ballast_h = 52;    // ballast chamber under the cradle: height at the bore radius, conical ceiling at 45°
ballast_wall = 2.4; // wall between the chamber and the outside of the stem
fill_d = 15;       // filling hole in the bottom
plug_d = 21;       // counterbore of the glued plug
plug_t = 1.2;      // = counterbore depth (6 layers of 0.2)
stem_r = 19;       // shaft radius at the bottom, it widens steadily up to the sleeve (collar_r)
stem_amp = 0.15;   // shaft lobes (only add material: the wall around the cell never gets thinner)
stem_twist = 1.2;  // lobe rotation along the shaft (°/mm)
bulb_r = [22, 34];   // bulbous base (sketch): bulge, short side then long side
bulb_z = 22;         // belly height
bulb_up = [21, 27];  // upward spread of the bulb: short side, long side (shoulder)
bulb_dn = 34;        // downward spread: rounded base, slope < 45° for printing
foot_dir = 200;    // direction of the long side (°)
ring_z = 72;       // cup ring: attached to the neck, raised edge under the cap
ring_ext = 20;     // average cup overhang
ring_rise = 0.75;  // edge rise / overhang (< 1: 45° at most, printable without support)
ring_amp = 0.35;   // cup waviness (overhang and rise)
ring_rot = 131;    // phase of the cup waves (°): its widest dip under the USB socket (+X), to plug the cable

/* [Cell cradle] */
cell_d = 18.6;     // bare 18650, flat top (protected / button top: up to ~70)
cell_l = 65;
cr_fit = 0.8;      // cradle / bore clearance (radial; 0.4: 1st print barely went in)
cell_up = 2;       // cell offset towards the open top (thickness of the floor under the cell)
bt_w = 13;         // tab contacts: 13 x 11 plate, 3 x 7 tab (drilled)
bt_h = 11;
bt_t = 0.4;        // sheet thickness (to measure)
bt_tab = [3, 7];   // tab up (open side), wire soldered on it
cr_win = [3, 15, 4, 14];  // lightening: windows per flank (count, length, bottom, top above the flat); floor and rails kept
wire_notch = [4.5, 3];  // wire pass above each end: width, depth below the top of the cell
bt_dimple = 1.8;   // bump of the "+" contact
spring_c = 4;      // spring of the "−" contact compressed (10.7 free)
bt_slot = 0.8;     // plate groove
bt_sink = 1.5;     // groove deeper than the plate (1st print: tab 1 mm above the edge; +0.4 for cr_fit 0.8)
bt_win = 10.5;     // window in front of the plate (room for the spring / the bump)

/* [Ground] */
ground_t = 1;      // disc under the stem (separate part, to glue)
ground_R = 72;     // mean radius
ground_amp = 0.15;
grass_seed = 11;
grass_tufts = 4;   // tufts of tall blades around the stem
grass_single = 30; // single blades
fringe_h = [3, 9]; // zigzag grass fringe around the stem: tooth height
fringe_n = 64;     // number of fringe teeth

/* [Mesh resolution] */
na = 90;           // dome: angular step
nt = 120;          // dome: step along the meridian
ns = 72;           // stem, ring: angular step

/* [Screws] */
m3_pilot = 2.5;
m3_clear = 3.4;
m25_pilot = 2.1;

$fn = 64;
eps = 0.01;

// ---------- derived ----------
bore_z = foot_t + ballast_h + bore_r + 2;   // bottom of the cradle bore, above the cone
ballast_top = foot_t + ballast_h + bore_r;  // top of the ceiling
ix = crop_w / 2 + 0.4;              // inner half-dimensions of the chassis
iy = crop_h / 2 + 0.4;
ch_z = disc_t;                      // bottom of the chassis
deck_z = ch_z + chamber_h;          // top of the deck ledges
pcb_z = deck_z + deck_t + pcb_gap;  // underside of the board
ch_h = pcb_z - ch_z;
// board turned 180° in the chassis: J1 moves to the -X side, under the leaning top of the cap
i2c_p1 = [k_crop_c[0] - i2c_k[0], i2c_k[1] - k_crop_c[1]];   // pin 1 in the cap frame
radio_p = [k_crop_c[0] - radio_k[0], radio_k[1] - k_crop_c[1]];   // J1, same (= dome frame in XY)
cap_top = [(radio_p[0] + (1 - cap_neck[0]) * cap_H * sin(cap_lean)) / cos(cap_lean), radio_p[1]];   // offset putting the leaning top over J1
bme_z = pcb_z - fh_h - mh_h;        // top of the breakout

module cap_xf() { translate([0, 0, stem_h]) rotate([0, cap_tilt, 0]) translate([0, 0, collar_h]) children(); }

// ---------- dome ----------
// meridian: Catmull-Rom through cap_shape; mirrored ghost points = vertical tangent at the edge, horizontal at the top
cap_prof = [for (p = cap_shape) [p[0] * cap_R, p[1] * cap_H]];
prof_n = len(cap_prof) - 1;
function ctrl(i) = i < 0 ? [cap_prof[1][0], -cap_prof[1][1]]
  : i > prof_n ? [-cap_prof[prof_n - 1][0], cap_prof[prof_n - 1][1]] : cap_prof[i];
// [point, derivative] of the meridian at u, from 0 (edge) to 1 (top)
function prof(u) = let(x = u * prof_n, i = min(prof_n - 1, floor(x)), v = x - i,
  a = ctrl(i - 1), b = ctrl(i), c = ctrl(i + 1), d = ctrl(i + 2),
  k1 = c - a, k2 = 2 * a - 5 * b + 4 * c - d, k3 = 3 * b - a - 3 * c + d)
  [b + (k1 + (k2 + k3 * v) * v) * v / 2, (k1 + (2 * k2 + 3 * k3 * v) * v) / 2];
prof_zr = [for (i = [0 : 240]) let(p = prof(i / 240)[0]) [p[1], p[0]]];   // height -> radius table
assert(min([for (i = [1 : 240]) prof_zr[i][0] - prof_zr[i - 1][0]]) > 0, "cap_shape: heights not increasing");
// dome axis at height z: starts on the disc axis, ends at cap_top
function neck_k(z) = let(k = max(0, min(1, (z / cap_H - cap_neck[0]) / (cap_neck[1] - cap_neck[0])))) k * k * (3 - 2 * k);
function cap_shift(z) = cap_top * neck_k(z);
// lean: rotation about the base of the neck, zero below the neck (electronics, corbels and disc unaffected)
function lean_pt(p) = let(a = cap_lean * neck_k(p[2]), dz = p[2] - cap_neck[0] * cap_H)
  [p[0] * cos(a) - dz * sin(a), p[1], cap_neck[0] * cap_H + p[0] * sin(a) + dz * cos(a)];

// outline: harmonics 2 to 7 with random phases, weighted around lobe_n, normalised to lobe_amp
function vsum(v, i = 0) = i < len(v) ? v[i] + vsum(v, i + 1) : 0;
lobe_terms = [for (k = [2 : 7]) let(q = rands(0, 1, 2, lobe_seed * 17 + k)) [k, (0.5 + 0.5 * q[0]) * exp(-pow((k - lobe_n) / 1.5, 2)), 360 * q[1]]];
function lobe_raw(phi) = vsum([for (h = lobe_terms) h[1] * sin(h[0] * phi + h[2])]);
lobe_peak = max([for (i = [0 : 179]) abs(lobe_raw(2 * i))]);
function lobe_at(phi, rot) = lobe_amp * lobe_raw(phi - rot) / lobe_peak;
// full deformation at the edge (s = 0), reduced to lobe_top at the top (s = 1)
function wav_r(phi, s, rot) = 1 + lobe_at(phi, rot) * (lobe_top + (1 - lobe_top) * (1 - s));
// edge thickening: full under the disc, then 45° blend (no support)
function rim_extra(z) = let(e = rim_w - cap_t) max(0, min(e, e - (z - cap_skirt)));
function inner_rr(phi, zz, rot) = lookup(zz, prof_zr) * wav_r(phi, min(1, zz / cap_H), rot) - cap_t - rim_extra(zz);
// clearance: electronics points (cap frame) that must stay >= 3 mm under the dome
clear_pts = concat(
  [for (sx = [-1, 1], sy = [-1, 1]) [sx * (crop_w / 2 + 2.4), sy * (crop_h / 2 + 2.4), disc_t + chamber_h + 9]],   // chassis
  [for (sx = [-1, 1], sy = [-1, 1]) [sx * 30.5, sy * 20, disc_t + chamber_h + 23]],                                // components
  [for (x = [38, 45], sy = [-1, 1]) [x, sy * 10.5, disc_t + chg_l]]);                                       // charger
// v <= 1: the point is under the dome shrunk by clear_m
function v_in(p, rot) = let(zz = p[2] + cap_skirt, d = [p[0], p[1]] - cap_shift(zz))
  norm(d) / (inner_rr(atan2(d[1], d[0]), zz, rot) - clear_m);
function worst(rot) = max([for (p = clear_pts) v_in(p, rot)]);
rots = [for (a = [0 : 5 : 355]) worst(a)];
best_rot = search(min(rots), rots)[0] * 5;
rot = lobe_rot >= 0 ? lobe_rot : best_rot;
function wav(phi, s = 0) = wav_r(phi, s, rot);
// surface point: t from 0 (edge) to 90 (top) along the meridian, off = inset along the normal
function cap_pt(phi, t, off = 0) = let(m = prof(t / 90), n = [m[1][1], -m[1][0]] / norm(m[1]), s = m[0][1] / cap_H,
  e = off > 0 ? rim_extra(m[0][1]) : 0, r = max(0, m[0][0] * wav(phi, s) - off * n[0] - e), z = m[0][1] - off * n[1], c = cap_shift(z))
  lean_pt([r * cos(phi) + c[0], r * sin(phi) + c[1], z]);
// inner dome radius at height zz (dome frame)
function inner_r(phi, zz) = inner_rr(phi, zz, rot);
apex = cap_pt(0, 90);   // dome frame
vent_in = apex - [0, 0, cap_t + 3];   // under the tip of the conical ceiling
vent_out = cap_pt(vent_at[0], vent_at[1]);
assert(vent_out[2] < vent_in[2], "The vent must slope down towards the outside");

function ring(j, off) = [for (i = [0 : na - 1]) cap_pt(i * 360 / na, j * 90 / nt, off)];
function surf(off) = concat([for (j = [0 : nt - 1]) each ring(j, off)], [cap_pt(0, 90, off)]);
function o(i, j) = j * na + (i % na);
module dome_solid(off = 0) {   // solid volume under the surface, closed by the edge plane
  polyhedron(convexity = 6, points = surf(off), faces = concat(
    [for (j = [0 : nt - 2], i = [0 : na - 1]) [o(i, j), o(i, j + 1), o(i + 1, j + 1), o(i + 1, j)]],
    [for (i = [0 : na - 1]) [o(i, nt - 1), nt * na, o(i + 1, nt - 1)]],
    [[for (i = [0 : na - 1]) o(i, 0)]]));
}

// dots: "Poisson disc" draw on the real surface: candidates on a dense, noisy (phi, t)
// grid, visited in a pseudo-random order, kept if far enough from the others
dot_cand = [for (t = [4 : 2 : 88], phi = [0 : 3 : 357]) let(q = rands(0, 1, 3, t * 1000 + phi + lobe_seed))
  let(tt = t + q[0] * 2, pp = phi + q[1] * 3, p = cap_pt(pp, tt)) if (p[2] > 9)
  [pp, tt, dot_bag[floor(q[2] * len(dot_bag) * 0.999)], p]];
function dot_far(c, acc, i = 0) = i >= len(acc) ? true
  : norm(c[3] - acc[i][3]) < (c[2] + acc[i][2]) / 2 + dot_gap ? false : dot_far(c, acc, i + 1);
function dot_pick(i, acc) = i >= len(dot_cand) ? acc
  : let(c = dot_cand[(i * 7919) % len(dot_cand)]) dot_pick(i + 1, dot_far(c, acc) ? concat(acc, [c]) : acc);
dots = [for (d = dot_pick(0, [])) if (norm(d[3] - vent_out) > vent_clear) d];   // filtered after the draw: the size distribution does not change
function unit(v) = v / norm(v);
function cap_normal(phi, t) = unit(cross(cap_pt(phi + 0.5, t) - cap_pt(phi - 0.5, t), cap_pt(phi, t + 0.5) - cap_pt(phi, t - 0.5)));
module at_dot(d) {
  n = cap_normal(d[0], d[1]);
  translate(cap_pt(d[0], d[1])) rotate([0, acos(n[2]), atan2(n[1], n[0])]) children();
}

// dome screws: axis 5 mm from the wall at the edge (away from the electronics), in the corbels
function tab_r(a) = inner_r(a, 2) - 5;
corbel_z = cap_skirt + disc_t;   // corbel bearing face = top of the disc (dome frame)
function argmax_a(a0) = let(c = [for (d = [-40 : 2 : 40]) lobe_at(a0 + d, rot)]) a0 - 40 + 2 * search(max(c), c)[0];
tab_a = [for (a = tab_base) argmax_a(a)];
// corbels on the wall, just above the disc: the disc rises into the dome and is pulled up by the screws;
// straight face on the inside, 45° chamfer up to the wall above; the only face towards the bed:
// the bearing face (corbel_z, ~7 mm above the bed), bridged without support (checked in PETG)
module corbels() {
  intersection() {
    for (a = tab_a) let(x0 = tab_r(a) - corbel_in, x1 = 1.5 * cap_R)   // cut by the wall
      rotate([0, 0, a]) translate([0, corbel_w / 2, 0]) rotate([90, 0, 0]) linear_extrude(corbel_w)
        polygon([[x0, corbel_z], [x1, corbel_z], [x1, corbel_z + corbel_h + x1 - x0], [x0, corbel_z + corbel_h]]);
    dome_solid();
  }
}
module cap() {
  difference() {
    union() {
      difference() {
        dome_solid();
        // 45° conical ceiling at the top: the narrow neck would prevent removing supports
        intersection() {
          translate([0, 0, -eps]) dome_solid(cap_t);
          translate([apex[0], apex[1], 0]) cylinder(r1 = apex[2] - cap_t, r2 = 0, h = apex[2] - cap_t);
        }
      }
      corbels();
    }
    for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) cylinder(d = d[2] + 0.3, h = dot_pocket + 1, $fn = 32);
    for (a = tab_a) rotate([0, 0, a]) translate([tab_r(a), 0, corbel_z - eps]) cylinder(d = m3_pilot, h = corbel_pilot + eps);
    let(v = vent_out - vent_in) translate(vent_in) rotate([0, acos(v[2] / norm(v)), atan2(v[1], v[0])]) cylinder(d = vent_d, h = norm(v) + 5, $fn = 32);
  }
}

module dot(d) {
  cylinder(d = d, h = dot_pocket + eps, $fn = 32);   // overlap: a single solid with the lens
  translate([0, 0, dot_pocket]) intersection() {
    scale([1, 1, 0.45]) sphere(d = d, $fn = 32);
    translate([-d, -d, 0]) cube([2 * d, 2 * d, d]);
  }
}

// ---------- gill disc (cap frame) ----------
// disc radius: the narrowest part of the wall between the edge and the top of the disc (in the lobe hollows,
// the wall moves inwards going down), so that it rises into the closed dome up to the corbels
function inner_min(a) = min([for (z = [0 : 0.5 : cap_skirt + disc_t]) inner_r(a, z)]);
function disc_pts() = [for (i = [0 : na - 1]) let(a = i * 360 / na, r = inner_min(a) - disc_fit)
                         r * [cos(a), sin(a)]];
disc_rmax = cap_R - cap_t;   // gill depth normalisation (beyond: minimum depth)
function gill_d(r) = gill_depth[1] + (gill_depth[0] - gill_depth[1]) * pow(max(0, (disc_rmax - r) / (disc_rmax - collar_r)), 1.3);
function adiff(a, b) = abs(((a - b + 540) % 360) - 180);
// angles where a gill would be in the way: dome screws, USB socket, chassis screws, sleeve screw
ch_screws = [for (sx = [-1, 1], sy = [-1, 1]) [sx * (ix - 2.5), sy * (iy - 2.5)]];
gill_skip = concat([for (a = tab_a) [a, 7]], [[0, 12], [collar_screw_a, 7]], [for (p = ch_screws) [atan2(p[1], p[0]), 5]]);
assert(min([for (a = [-12 : 2 : 12]) inner_min(a) - disc_fit]) >= chg_x + chg_t + 1.5 + usb_cut[0] / 2 + 2, "Disc too small around the USB socket");
function gill_ok(a) = min([for (s = gill_skip) adiff(a, s[0]) - s[1]]) > 0;

module gills() {
  difference() {
    union() {
      linear_extrude(disc_t) polygon(disc_pts());
      // sleeve
      translate([0, 0, -collar_h]) cylinder(r = collar_r, h = collar_h + eps);
      // gills: radial blades, deeper near the stem
      intersection() {
        for (i = [0 : n_gills - 1]) let(a = (i + 0.5) * 360 / n_gills) if (gill_ok(a))
          rotate([0, 0, a]) rotate([90, 0, 0]) linear_extrude(gill_t, center = true)
            polygon(concat([[collar_r - 1, 0]], [for (r = [collar_r - 1 : 2 : cap_R * 1.5]) [r, -gill_d(r)]], [[cap_R * 1.5, 0]]));
        translate([0, 0, -50]) linear_extrude(50 + eps) offset(delta = -1) polygon(disc_pts());
      }
    }
    // sleeve
    translate([0, 0, -collar_h - eps]) cylinder(r = tenon_r + 0.25, h = collar_h);
    translate([0, 0, -1]) cylinder(r = disc_bore, h = disc_t + 2);
    // radial sleeve screw
    rotate([0, 0, collar_screw_a]) translate([tenon_r - 1, 0, -collar_h + 8]) rotate([0, 90, 0]) cylinder(d = m3_clear, h = collar_r - tenon_r + 2);
    // vent slots under the sensor chamber, between the gills; on the +X side too (flush with the edge
    // of the BME280, the rest of the breakout is above the sleeve)
    for (i = [0 : n_gills - 1]) let(a = i * 360 / n_gills)
      rotate([0, 0, a]) translate([collar_r + 1, -0.8, -1]) cube([min(ix / max(abs(cos(a)), 0.01), iy / max(abs(sin(a)), 0.01), 31) - collar_r - 1, 1.6, disc_t + 2]);
    // micro-USB socket
    translate([chg_x + chg_t + 1.5 - usb_cut[0] / 2, -usb_cut[1] / 2, -1]) cube([usb_cut[0], usb_cut[1], disc_t + 2]);
    // dome screws: clearance, heads under the disc between the gills
    for (a = tab_a) rotate([0, 0, a]) translate([tab_r(a), 0, -1]) cylinder(d = m3_clear, h = disc_t + 2);
    // chassis screws, heads under the disc
    for (p = ch_screws) translate([p[0], p[1], -1]) cylinder(d = m3_clear, h = disc_t + 2);
  }
}

// ---------- chassis (cap frame, resting on the disc) ----------
module chassis() {
  translate([0, 0, ch_z]) difference() {
    union() {
      difference() {
        translate([-ix - ch_wall, -iy - ch_wall, 0]) cube([2 * (ix + ch_wall), 2 * (iy + ch_wall), ch_h]);
        translate([-ix, -iy, -1]) cube([2 * ix, 2 * iy, ch_h + 2]);
        // chamber windows
        for (x = [-20, -6, 8, 22]) translate([x - 4, -iy - ch_wall - 1, chamber_h - 1 - win_h]) cube([8, 2 * (iy + ch_wall) + 2, win_h]);
        for (y = [-8, 8]) translate([-ix - ch_wall - 1, y - 4, chamber_h - 1 - win_h]) cube([ch_wall + 2, 8, win_h]);
      }
      // deck and board ledges along the long sides (45° chamfer underneath)
      for (s = [-1, 1], z = [chamber_h, ch_h]) mirror([0, s < 0 ? 1 : 0, 0]) translate([-ix, iy, z])
        rotate([0, 90, 0]) linear_extrude(2 * ix) polygon([[0, 0], [1.5, 0], [0, -1.5]]);
      // corners: chassis screw bosses (bottom); top corners solid up to the board (board M2.5 screws)
      for (p = ch_screws) translate([p[0] > 0 ? ix - 7 : -ix, p[1] > 0 ? iy - 7 : -iy, 0])
        cube([7, 7, p[1] < 0 ? ch_h : chamber_h - 1]);
      // charge module slide, held by two arms on the +X wall
      translate([chg_x, 0, 0]) rails(chg_w, chg_t, 20);
      for (s = [-1, 1]) translate([ix + ch_wall - eps, s > 0 ? chg_w / 2 - 1 : -chg_w / 2 - 2, 0]) cube([chg_x - 1.5 - ix - ch_wall + 2 * eps, 3, 20]);
    }
    // chassis screws (from below) and board screws (from above)
    for (p = ch_screws) translate([p[0], p[1], -eps]) cylinder(d = m3_pilot, h = 7);
    for (h = pcb_holes) translate([h[0], h[1], ch_h - 6]) cylinder(d = m25_pilot, h = 6 + eps);
  }
}

// two facing U rails, board in the YZ plane (thickness along X)
module rails(w, t, h) {
  for (s = [-1, 1]) translate([-1.5, s > 0 ? w / 2 - 1 : -w / 2 - 2, 0]) difference() {
    cube([t + 3, 3, h]);
    translate([1.5 - 0.15, s > 0 ? -eps : 2 - eps, -eps]) cube([t + 0.3, 1 + eps, h + 2 * eps]);
  }
}

// ---------- perforated deck ----------
module deck() {
  difference() {
    translate([-ix + 0.3, -iy + 0.3, 0]) cube([2 * ix - 0.6, 2 * iy - 0.6, deck_t]);
    for (p = ch_screws) translate([p[0] - 4.6, p[1] - 4.6, -1]) cube([9.2, 9.2, deck_t + 2]);
    for (x = [-24 : 6 : 24], y = [-15 : 6 : 15]) translate([x + (round(y / 6) % 2) * 3, y, -1]) cylinder(d = 4, h = deck_t + 2, $fn = 6);
    // I2C female header window
    translate([i2c_p1[0], i2c_p1[1], -1]) rotate(180) translate([-(bme_pins - 0.5) * hdr_p - fh_clr, -hdr_p / 2 - fh_clr, 0])
      cube([bme_pins * hdr_p + 2 * fh_clr, hdr_p + 2 * fh_clr, deck_t + 2]);
  }
}

// ---------- stem ----------
// generic generator (same principle as the cap outline): harmonics 2..7 with random phases
function hterms(seed, n) = [for (k = [2 : 7]) let(q = rands(0, 1, 2, seed * 17 + k)) [k, (0.5 + 0.5 * q[0]) * exp(-pow((k - n) / 1.5, 2)), 360 * q[1]]];
function hval(T, phi) = vsum([for (h = T) h[1] * sin(h[0] * phi + h[2])]);
function hpeak(T) = max([for (i = [0 : 179]) abs(hval(T, 2 * i))]);
function hnorm(T, P, phi) = hval(T, phi) / P;   // in [-1, 1]
stem_T = hterms(lobe_seed + 101, 3);  stem_P = hpeak(stem_T);
ring_T = hterms(lobe_seed + 202, 4);  ring_P = hpeak(ring_T);
ring_T2 = hterms(lobe_seed + 303, 5); ring_P2 = hpeak(ring_T2);
ground_T = hterms(grass_seed, 4);     ground_P = hpeak(ground_T);

function base_r(z) = stem_r + (collar_r - stem_r) * min(1, z / stem_h);
// outer stem radius: conical shaft + twisting lobes (zero under the sleeve) + asymmetric bulb
function foot_w(phi) = pow((1 + cos(phi - foot_dir)) / 2, 2);   // 1 on the wide-flare side, 0 opposite
function stem_rr(phi, z) = let(w = foot_w(phi), B = bulb_r[0] + (bulb_r[1] - bulb_r[0]) * w,
  S = z < bulb_z ? bulb_dn : bulb_up[0] + (bulb_up[1] - bulb_up[0]) * w,
  fade = max(0, min(1, (stem_h - z) / 15)))
  base_r(z) * (1 + stem_amp * (0.5 + 0.5 * hnorm(stem_T, stem_P, phi - stem_twist * z)) * fade) + B * exp(-pow((z - bulb_z) / S, 2));

// ballast chamber volume (cm³), integrated in slices
ballast_v = let(dz = 0.5, na = 72) vsum([for (z = [foot_t : dz : ballast_top]) vsum([for (i = [0 : na - 1])
  let(r = max(0, min(stem_rr(i * 360 / na, z) - ballast_wall, ballast_top - z))) 0.5 * r * r * (2 * PI / na) * dz])]) / 1000;

stem_zs = [for (z = [0 : 1.5 : stem_h + 6]) z];
module stem_body(off = 0, zs = stem_zs) {   // off: inset from the surface (inside of the ballast chamber)
  J = len(zs);
  function v(i, j) = j * ns + (i % ns);
  polyhedron(convexity = 4,
    points = [for (z = zs, i = [0 : ns - 1]) let(a = i * 360 / ns, r = stem_rr(a, z) - off) [r * cos(a), r * sin(a), z]],
    faces = concat([for (j = [0 : J - 2], i = [0 : ns - 1]) [v(i, j), v(i, j + 1), v(i + 1, j + 1), v(i + 1, j)]],
                   [[for (i = [0 : ns - 1]) v(i, 0)]], [[for (i = [ns - 1 : -1 : 0]) v(i, J - 1)]]));
}

// cup ring: starts at the neck and rises while flaring, wavy edge; 45° at most, no support
function ring_prof(a) = let(ri = stem_rr(a, ring_z) - 1, ext = ring_ext * (1 + ring_amp * hnorm(ring_T, ring_P, a - ring_rot)),
  rise = ext * ring_rise * (1 + 0.3 * ring_amp * hnorm(ring_T2, ring_P2, a - ring_rot)))
  [[ri, ring_z + 2.4], [ri + ext, ring_z + rise + 1.2], [ri + ext, ring_z + rise], [ri, ring_z - 1.2]];
// USB socket seen from the stem: inner edge of the window (radius); the cup stays 2 mm short under the plug (~11 mm wide: ±8°)
usb_in_r = (chg_x + chg_t + 1.5 - usb_cut[0] / 2) * cos(cap_tilt) + collar_h * sin(cap_tilt);
usb_ring_r = max([for (a = [-8 : 1 : 8]) ring_prof(a)[1][0]]);
assert(usb_ring_r <= usb_in_r - 2, str("Cup under the USB socket (edge at ", usb_ring_r, " mm, window at ", usb_in_r, " mm): change ring_rot"));
module ring_skirt() {
  function prof(a) = ring_prof(a);
  function v(i, k) = (i % ns) * 4 + (k % 4);
  polyhedron(convexity = 4,
    points = [for (i = [0 : ns - 1], p = prof(i * 360 / ns)) let(a = i * 360 / ns) [p[0] * cos(a), p[0] * sin(a), p[1]]],
    faces = [for (i = [0 : ns - 1], k = [0 : 3]) [v(i, k + 1), v(i, k), v(i + 1, k), v(i + 1, k + 1)]]);
}

module stem() {
  difference() {
    union() {
      // shaft, cut under the sleeve (plane perpendicular to the cap axis)
      intersection() {
        stem_body();
        cap_xf() translate([-100, -100, -collar_h - 200]) cube([200, 200, 200]);
      }
      ring_skirt();
      // tilted tenon, fitted into the sleeve
      cap_xf() translate([0, 0, -collar_h - 1]) cylinder(r = tenon_r, h = 17);
    }
    // cell bore, vertical up to the top (the cradle goes through the tenon)
    translate([0, 0, bore_z]) cylinder(r = bore_r, h = stem_h + 40);
    // ballast chamber: follows the stem skin (ballast_wall wall, slopes <= 45° like the bulb),
    // conical ceiling at 45° (no support); filled through the bottom hole then plugged
    intersection() {
      stem_body(ballast_wall, [for (z = [foot_t : 1.5 : ballast_top]) z]);
      translate([0, 0, foot_t - eps]) cylinder(r1 = ballast_top - foot_t, r2 = 0, h = ballast_top - foot_t);
    }
    translate([0, 0, -1]) cylinder(d = fill_d, h = foot_t + 2);
    translate([0, 0, -1]) cylinder(d = plug_d + 0.3, h = plug_t + 1);
    // radial sleeve screw
    cap_xf() rotate([0, 0, collar_screw_a]) translate([tenon_r - 5, 0, -collar_h + 8]) rotate([0, 90, 0]) cylinder(d = m3_pilot, h = 6);
  }
}

// ---------- cell cradle ----------
// Trough printed flat (flat face on the bed, open upwards), slid into the stem from the top.
// One tab contact per end, slid into a groove, tab up; the wire soldered on it leaves through
// the nearby notch and runs along the cell in the top slot (up to the sleeve). "−" spring at the bottom, "+" bump at the top.
// Print frame: X = cell axis (X = 0 at the bottom), flat face at Z = 0.
cr_r = bore_r - cr_fit;
cr_flat = cr_r * cos(45);           // flat: flanks at 45° at most
cr_ax = cr_flat;                    // bore axis above the bed
cl_z = cr_ax + cell_up;             // cell axis
cr_end = 1.6 + bt_slot + 1;         // end: outer wall + groove + inner wall
cell_x0 = 1.6 + bt_t + spring_c;    // bottom of the cell (spring compressed)
cr_len = cell_x0 + cell_l + bt_dimple + bt_t + 1.6;
// top of the cradle below the breakout, in the cap frame (Z = 0 under the disc)
cradle_top = (bore_z + cr_len - stem_h) * cos(cap_tilt) + cr_r * sin(cap_tilt) - collar_h;
wire_gap = bme_z - bme_t - cradle_top;   // room for the JST-XH pair
assert(wire_gap >= 8, str("No room for the JST-XH pair: ", wire_gap, " mm under the BME280"));
module cradle() {
  difference() {
    intersection() {
      rotate([0, 90, 0]) translate([-cr_ax, 0, 0]) cylinder(r = cr_r, h = cr_len, $fn = 96);
      translate([0, -cr_r, 0]) cube([cr_len, 2 * cr_r, 2 * cr_r]);
    }
    // cell bore, insertion slot above its axis (+1 mm)
    translate([cr_end, 0, cl_z]) rotate([0, 90, 0]) cylinder(d = cell_d + 0.6, h = cr_len - 2 * cr_end);
    translate([cr_end, -cell_d / 2 - 0.3, cl_z + 1]) cube([cr_len - 2 * cr_end, cell_d + 0.6, 2 * cr_r]);
    for (s = [0, 1]) {
      xs = s == 0 ? 1.6 : cr_len - 1.6 - bt_slot;   // groove
      // plate groove, open at the top (tab up)
      translate([xs, -(bt_w + 0.6) / 2, cl_z - bt_h / 2 - bt_sink]) cube([bt_slot, bt_w + 0.6, 2 * cr_r]);
      // wire notch next to the tab, through the end walls, up to above the cell:
      // the wire then runs along the cell in the top slot (≈3 mm below the bore)
      translate([s == 0 ? -1 : cr_len - cr_end - 1, -(bt_w + 0.6) / 2 - wire_notch[0] + 2, cl_z + cell_d / 2 - wire_notch[1]])
        cube([cr_end + 2, wire_notch[0], 2 * cr_r]);
      // window in front of the plate
      translate([s == 0 ? xs + bt_slot - eps : cr_len - cr_end - eps, -bt_win / 2, cl_z - bt_h / 2]) cube([1 + 2 * eps, bt_win, 2 * cr_r]);
    }
    // through windows in the flanks, under the lips holding the cell (top bridged)
    let(n = cr_win[0], l = cr_win[1], post = (cr_len - 2 * cr_end - 8 - n * l) / (n - 1))
      for (i = [0 : n - 1]) translate([cr_end + 4 + i * (l + post), -cr_r - 1, cr_win[2]]) cube([l, 2 * cr_r + 2, cr_win[3] - cr_win[2]]);
    // pull hole in the top end (string, knot inside)
    translate([cr_len - cr_end - 1, 0, cl_z + cell_d / 2 + 0.8]) rotate([0, 90, 0]) cylinder(d = 2.5, h = cr_end + 2, $fn = 24);
    // polarity, engraved on the end faces
    for (m = [[0, "-"], [1, "+"]]) translate([m[0] * cr_len, 0, cl_z]) mirror([m[0], 0, 0]) rotate([90, 0, -90])
      translate([0, 0, -0.6]) linear_extrude(1) text(m[1], size = 8, halign = "center", valign = "center");
  }
}
// fitted cradle: bottom of the cradle on the floor of the bore, flat face towards -X (open side towards +X)
module cradle_placed() { translate([0, 0, bore_z]) rotate([0, -90, 0]) translate([0, 0, -cr_ax]) children(); }

// ---------- ground: 1 mm disc and vegetation ----------
function ground_r(a) = ground_R * (1 + ground_amp * hnorm(ground_T, ground_P, a));
// blade: tapered blade (0.9 mm thick, 0.6 mm tip), curved in its plane, slope <= ~40° (printable)
module blade(h, w, lean) {
  for (k = [0 : 3]) hull() for (q = [k, k + 1])
    translate([lean * pow(q / 4, 2), 0, h * q / 4]) cube([max(0.6, w * pow(1 - q / 4, 1.3)), 0.9, 0.01], center = true);
}
grass = let(R = rands(0, 1, 6 * (grass_tufts * 9 + grass_single), grass_seed))
  concat(
    // tufts: 6 to 9 tall blades fanning out, against the stem
    [for (t = [0 : grass_tufts - 1], b = [0 : 8]) let(o = 6 * (t * 9 + b), t0 = 6 * t * 9,
       ta = t * 360 / grass_tufts + 40 * R[t0], tr = stem_rr(ta, 0) + 2 + 4 * R[t0 + 1],
       fan = -70 + 140 * b / 8 + 15 * (R[o + 2] - 0.5), h = 10 + 9 * R[o + 3])
       if (b < 6 + 3 * R[t0 + 2])
         [[tr * cos(ta) + 4 * (R[o + 4] - 0.5), tr * sin(ta) + 4 * (R[o + 5] - 0.5)], ta + fan, h, 3 + 1.5 * R[o + 1], h * 0.55 * (R[o] - 0.2)]],
    // single blades
    [for (i = [0 : grass_single - 1]) let(o = 6 * (grass_tufts * 9 + i), a = 360 * R[o],
       rr = stem_rr(a, 0) + 4 + (ground_r(a) - 8 - stem_rr(a, 0) - 4) * pow(R[o + 1], 1.5), h = 4 + 6 * R[o + 2])
       [[rr * cos(a), rr * sin(a)], a + 90 * (R[o + 3] - 0.5), h, 2.5 + R[o + 4], h * 0.5 * (R[o + 5] - 0.5)]]);
// fringe: triangular teeth along a wavy curve around the stem (continuous 0.9 mm wall)
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

// ---------- mock-ups ----------
module cradle_cell() {   // cradle, cell, contacts (print frame)
  color("#303030") cradle();
  color("#2e6fd8") translate([cell_x0, 0, cl_z]) rotate([0, 90, 0]) cylinder(d = cell_d, h = cell_l);
  color("Silver") for (x = [1.6, cr_len - 1.6 - bt_t]) translate([x, -bt_w / 2, cl_z - bt_h / 2 - bt_sink]) {
    cube([bt_t, bt_w, bt_h]);
    translate([0, (bt_w - bt_tab[0]) / 2, bt_h]) cube([bt_t, bt_tab[0], bt_tab[1]]);
  }
}
module charger() { color("#1a4fa0") translate([0, -chg_w / 2, 0]) cube([chg_t, chg_w, chg_l]); color("Silver") translate([chg_t, -4, 0]) cube([3, 8, 5.5]); }
// BME280 plugged under the board (frame: pin 1 of the I2C footprint, underside of the board):
// fh_n-position female header, male header plastic, breakout parallel to the board
module bme() {
  color("#222") translate([-(fh_n - 0.5) * hdr_p, -hdr_p / 2, -fh_h]) cube([fh_n * hdr_p, hdr_p, fh_h]);
  color("#222") translate([-(bme_pins - 0.5) * hdr_p, -hdr_p / 2, -fh_h - mh_h]) cube([bme_pins * hdr_p, hdr_p, mh_h]);
  if (fh_n < bme_pins) color("Gold") for (i = [fh_n : bme_pins - 1]) translate([-i * hdr_p - 0.32, -0.32, -fh_h]) cube([0.64, 0.64, mh_pin]);
  color("Purple") translate([-(bme_pins - 1) * hdr_p / 2 - bme_w / 2, -bme_edge, -fh_h - mh_h - bme_t]) cube([bme_w, bme_d, bme_t]);
}

// radial section through the corbel at angle a: corbel, pilot hole, M3 x 10 screw, gill disc
// (render(): clean cut faces, otherwise z-fighting in preview; ~4 min)
module console_cut(a) {
  r = tab_r(a);
  module cut() rotate([0, 0, a]) translate([r - 24, 0, -12]) cube([40, 30, 34]);
  rotate([0, 0, -a]) {
    color("Ivory") render() intersection() { gills(); cut(); }
    color(cap_color) render() intersection() { translate([0, 0, -cap_skirt]) cap(); cut(); }
    color("Silver") rotate([0, 0, a]) translate([r, 0, 0]) render() intersection() {   // self-tapping, head under the disc
      union() { translate([0, 0, -2]) cylinder(d = 5.5, h = 2, $fn = 32); cylinder(d = 2.9, h = 10, $fn = 32); }
      translate([-5, 0, -5]) cube([10, 5, 20]);
    }
  }
}

module assembled() {
  color("Ivory") stem();
  color("#4a8f3c") translate([0, 0, -ground_t]) ground();
  cradle_placed() cradle_cell();
  cap_xf() {
    color("Ivory") gills();
    color("Beige") chassis();
    translate([i2c_p1[0], i2c_p1[1], pcb_z]) rotate(180) bme();
    translate([chg_x + 0.15, 0, ch_z]) charger();
    color("WhiteSmoke") translate([0, 0, deck_z]) deck();
    translate([0, 0, pcb_z]) rotate(180) pcb_v22_cropped(jst_eh = false);
    translate([0, 0, -cap_skirt]) {
      color(cap_color) cap();
      for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) color("White") dot(d[2]);
    }
  }
}

// exploded view, assembly order: cell out of the stem, cap lifted; in the cap, bottom to top:
// disc, chassis + charger, deck, board + BME280, dome
module exploded(e = explode) {
  color("Ivory") stem();
  color("#4a8f3c") translate([0, 0, -ground_t]) ground();
  translate([0, 0, 2 * e]) cradle_placed() cradle_cell();
  translate([0, 0, 4 * e]) cap_xf() {
    color("Ivory") gills();
    translate([0, 0, e]) { color("Beige") chassis(); translate([chg_x + 0.15, 0, ch_z]) charger(); }
    translate([0, 0, 2 * e + 15]) color("WhiteSmoke") translate([0, 0, deck_z]) deck();
    translate([0, 0, 3 * e + 30]) {
      translate([i2c_p1[0], i2c_p1[1], pcb_z]) rotate(180) bme();
      translate([0, 0, pcb_z]) rotate(180) pcb_v22_cropped(jst_eh = false);
    }
    translate([0, 0, 5 * e + 60 - cap_skirt]) {
      color(cap_color) cap();
      for (d = dots) at_dot(d) translate([0, 0, -dot_pocket]) color("White") dot(d[2]);
    }
  }
}

echo(str("Outline: seed ", lobe_seed, ", orientation ", rot, "°, clearance index ", round(worst(rot) * 100) / 100, worst(rot) <= 1 ? " (ok: >= 3 mm everywhere)" : " (WARNING: electronics less than 3 mm from the dome, raise cap_R / cap_H or change the seed)"));
echo(str("Dots: ", len(dots), " in total — ", [for (s = dot_sizes) str("Ø", s, " x", len([for (d = dots) if (d[2] == s) 1]))]));
echo(str("Dome top: ", [for (v = [apex[0], apex[1]]) round(v * 10) / 10], " (J1 \"RADIO\" at ", [for (v = radio_p) round(v * 10) / 10], "), ", round(apex[2] - cap_skirt - pcb_z - pcb_t), " mm above the board; ", round(collar_h * sin(cap_tilt) + apex[0] * cos(cap_tilt) + (apex[2] - cap_skirt) * sin(cap_tilt)), " mm from the stem axis in X"));
echo(str("BME280: pin 1 at ", [for (v = i2c_p1) round(v * 100) / 100], ", underside of the breakout ", bme_z - bme_t - disc_t, " mm above the disc"));
echo(str("Height ~", round(stem_h - apex[0] * sin(cap_tilt) + (apex[2] - cap_skirt + collar_h) * cos(cap_tilt) + ground_t), " mm; cap Ø~", round(2 * cap_R), "; cell bore Ø", 2 * bore_r, "; cradle ", round(cr_len * 10) / 10, " mm (cell ", cell_l, " mm); cradle ", round(cradle_top), " mm above the underside of the disc, ", round(wire_gap), " mm under the BME280; ballast ", round(ballast_v), " cm³ ≈ ", round(ballast_v * 4.7), " g of steel shot"));

if (part == "cap") cap();
else if (part == "gills") translate([0, 0, disc_t]) rotate([180, 0, 0]) gills();   // gills up
else if (part == "chassis") translate([0, 0, -ch_z]) chassis();
else if (part == "deck") deck();
else if (part == "stem") stem();
else if (part == "ground") ground();
else if (part == "cradle") cradle();
else if (part == "plug") cylinder(d = plug_d, h = plug_t);
else if (part == "dot") dot(dot_d);
else if (part == "sensor") difference() {   // close-up: BME280 plugged under the board
  assembled();
  translate([-100, -90.5, 70]) cube([200, 90, 150]);   // the camera must stay outside the removed volume
  translate([-100, -100, 70]) cube([200, 200, 30]);
}
else if (part == "console") console_cut(tab_a[0]);
else if (part == "exploded") exploded();
else if (part == "cradle_view") { cradle_cell(); translate([0, 40, 0]) color("Gold") cradle(); }   // cradle fitted and bare
else if (part == "section") difference() { assembled(); translate([-200, -200, -1]) cube([400, 200, 400]); }
else {
  assembled();
  translate([75, -25, 0]) euro2();
}
