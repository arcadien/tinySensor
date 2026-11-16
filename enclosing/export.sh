#!/usr/bin/env bash
# Updates the Orca projects of print/ (one per model) from the .scad files: plates, positions and settings
# kept, project created when missing. Details: update_3mf.py.
# Usage: ./export.sh [project.3mf ...]   (default: all of them: box, shield, stand, mushroom, receiver)
set -euo pipefail
cd "$(dirname "$0")"
python3 update_3mf.py "$@"
