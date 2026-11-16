#!/usr/bin/env bash
# Updates the Orca projects (one per model, next to its .scad) from the .scad files: plates, positions and settings
# kept, project created when missing. Details: update_3mf.py.
# Usage: ./export.sh [model/project.3mf ...]   (default: all of them: box, shield, stand, mushroom, receiver)
set -euo pipefail
cd "$(dirname "$0")"
python3 update_3mf.py "$@"
