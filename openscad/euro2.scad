// €2 coin (Ø25.75 × 2.2 mm, centre Ø18.75), lying flat on Z = 0: scale reference in the renders.
module euro2() {
  color("Silver") difference() { cylinder(d = 25.75, h = 2.2, $fn = 96); translate([0, 0, -0.1]) cylinder(d = 18.75, h = 2.4, $fn = 96); }
  color("Gold") cylinder(d = 18.75, h = 2.2, $fn = 96);
}

euro2();
