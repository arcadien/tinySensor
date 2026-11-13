#!/usr/bin/env bash
# Met à jour les projets Orca de print/ (un par modèle) depuis les .scad : plaques, positions et réglages
# gardés, projet créé s'il manque. Détail : update_3mf.py.
# Usage : ./export.sh [projet.3mf ...]   (par défaut tous : box, shield, stand, mushroom, receiver)
set -euo pipefail
cd "$(dirname "$0")"
python3 update_3mf.py "$@"
