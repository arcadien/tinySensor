// Pièce de 2 € (Ø25,75 × 2,2 mm, cœur Ø18,75), posée à plat sur Z = 0 : sert d'échelle dans les rendus.
module euro2() {
  color("Silver") difference() { cylinder(d = 25.75, h = 2.2, $fn = 96); translate([0, 0, -0.1]) cylinder(d = 18.75, h = 2.4, $fn = 96); }
  color("Gold") cylinder(d = 18.75, h = 2.2, $fn = 96);
}

euro2();
