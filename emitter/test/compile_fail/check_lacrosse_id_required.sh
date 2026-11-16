#!/usr/bin/env bash
# @req CONF-BUILD-004
# Compile-fail check: a build defining USE_LACROSSE or USE_BH1750 without
# LACROSSE_ID must be rejected by the preprocessor with an #error naming
# CONF-BUILD-004 (a plain "'LACROSSE_ID' was not declared" does not count).
#
# Production side must provide (not created here), in a header seen by every
# build (e.g. include/config.h):
#   #if (defined(USE_LACROSSE) || defined(USE_BH1750)) && !defined(LACROSSE_ID)
#   #error "CONF-BUILD-004: LACROSSE_ID is required with USE_LACROSSE or USE_BH1750"
#   #endif
#
# Usage: test/compile_fail/check_lacrosse_id_required.sh   (PIO=/path/to/pio to override)
set -u

PIO=${PIO:-pio}
cd "$(dirname "$0")/../.."
FAILED=0

# $1: case label, $2: env (without LACROSSE_ID), $3: extra build flags
check_rejected() {
  # Given: an env without LACROSSE_ID, plus the flag that requires it
  local build_dir
  build_dir=$(mktemp -d)

  # When: building (not running) the test_main_cycle suite
  local output status
  output=$(PLATFORMIO_BUILD_DIR="$build_dir" PLATFORMIO_BUILD_FLAGS="$3" \
    "$PIO" test -e "$2" --without-testing 2>&1)
  status=$?
  rm -rf "$build_dir"

  # Then: the build fails, and for the CONF-BUILD-004 reason
  if [ "$status" -eq 0 ]; then
    echo "FAIL CONF-BUILD-004 ($1): build without LACROSSE_ID succeeded"
    FAILED=1
  elif ! grep -q "#error.*CONF-BUILD-004" <<<"$output"; then
    echo "$output"
    echo "FAIL CONF-BUILD-004 ($1): build failed, but not on the CONF-BUILD-004 #error"
    FAILED=1
  else
    echo "PASS CONF-BUILD-004 ($1): missing LACROSSE_ID rejected at compile time"
  fi
}

check_rejected "USE_LACROSSE" native_cycle_x10 "-DUSE_LACROSSE"
check_rejected "USE_BH1750 with Oregon" native_cycle_oregon "-DUSE_BH1750"
exit "$FAILED"
