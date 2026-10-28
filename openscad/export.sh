#!/usr/bin/env bash
# Exporte toutes les pièces imprimables en 3MF (orientation d'impression) dans print/.
# Usage : ./export.sh [stl]   — "stl" pour des STL au lieu des 3MF.
# Le suffixe _xN indique le nombre d'exemplaires à imprimer.
set -euo pipefail
cd "$(dirname "$0")"
ext=${1:-3mf}
out=print
mkdir -p "$out"

jobs=()
run() { # run <fichier.scad> <sortie> [-D ...]
  local src=$1 dst=$2; shift 2
  openscad "$@" -o "$out/$dst.$ext" "$src" >"$out/$dst.log" 2>&1 && rm "$out/$dst.log" &
  jobs+=($!)
}

# Boîtier mural
run tinysensor_box.scad box_base_ext -D 'part="base"' -D 'usage="ext"'
run tinysensor_box.scad box_base_int -D 'part="base"' -D 'usage="int"'
run tinysensor_box.scad box_lid      -D 'part="lid"'
# Abri de sonde
run sensor_shield.scad shield_cap      -D 'part="cap"'
run sensor_shield.scad shield_plate_x4 -D 'part="plate"'
run sensor_shield.scad shield_base     -D 'part="base"'
run sensor_shield.scad shield_clip     -D 'part="clip"'
# Pied de table
run stand.scad stand_mast -D 'part="mast"'
run stand.scad stand_foot -D 'part="foot"'
# Champignon (nombre de pois : voir l'écho « Pois » de mushroom.scad)
run mushroom.scad mushroom_cap     -D 'part="cap"'
run mushroom.scad mushroom_gills   -D 'part="gills"'
run mushroom.scad mushroom_chassis -D 'part="chassis"'
run mushroom.scad mushroom_deck    -D 'part="deck"'
run mushroom.scad mushroom_stem    -D 'part="stem"'
run mushroom.scad mushroom_ground  -D 'part="ground"'
run mushroom.scad mushroom_cradle  -D 'part="cradle"'
run mushroom.scad mushroom_plug    -D 'part="plug"'
run mushroom.scad mushroom_dot6_x53  -D 'part="dot"' -D 'dot_d=6'
run mushroom.scad mushroom_dot8_x37  -D 'part="dot"' -D 'dot_d=8'
run mushroom.scad mushroom_dot10_x19 -D 'part="dot"' -D 'dot_d=10'

fail=0
for j in "${jobs[@]}"; do wait "$j" || fail=1; done
if ls "$out"/*.log >/dev/null 2>&1; then
  echo "Échec : voir $out/*.log" >&2; exit 1
fi
[ $fail -eq 0 ] && echo "OK : $(ls "$out"/*."$ext" | wc -l) fichiers dans $out/"
