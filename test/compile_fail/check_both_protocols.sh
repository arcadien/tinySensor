#!/usr/bin/env bash
# @req CONF-BUILD-001
# Compile-fail check: a build defining both USE_OREGON and USE_LACROSSE must
# be rejected by the preprocessor with an #error naming CONF-BUILD-001.
#
# Production side must provide (not created here), in include/EnvironmentEncoder.h:
#   #if defined(USE_OREGON) && defined(USE_LACROSSE)
#   #error "CONF-BUILD-001: define at most one of USE_OREGON or USE_LACROSSE"
#   #endif
#
# Usage: test/compile_fail/check_both_protocols.sh   (PIO=/path/to/pio to override)
set -u

# Given: the Oregon cycle env, plus Lacrosse flags, in a throwaway build dir
PIO=${PIO:-pio}
cd "$(dirname "$0")/../.."
BUILD_DIR=$(mktemp -d)
trap 'rm -rf "$BUILD_DIR"' EXIT

# When: building (not running) the test_main_cycle suite
OUTPUT=$(PLATFORMIO_BUILD_DIR="$BUILD_DIR" \
  PLATFORMIO_BUILD_FLAGS="-DUSE_LACROSSE -DLACROSSE_ID=LacrosseWS7000::Address::ONE" \
  "$PIO" test -e native_cycle_oregon --without-testing 2>&1)
STATUS=$?

# Then: the build fails, and for the CONF-BUILD-001 reason
if [ "$STATUS" -eq 0 ]; then
  echo "FAIL CONF-BUILD-001: build with USE_OREGON and USE_LACROSSE succeeded"
  exit 1
fi
if ! grep -q "#error.*CONF-BUILD-001" <<<"$OUTPUT"; then
  echo "$OUTPUT"
  echo "FAIL CONF-BUILD-001: build failed, but not on the CONF-BUILD-001 #error"
  exit 1
fi
echo "PASS CONF-BUILD-001: both protocols rejected at compile time"
