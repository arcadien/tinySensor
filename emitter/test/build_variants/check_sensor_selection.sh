#!/usr/bin/env bash
# @req CONF-BUILD-002
# Build-variant check: runs the test_main_cycle suite on sensor flag
# combinations no native_cycle_* env covers. The suite derives its expected
# HAL sequence from the same flags, so a passing run means the cycle honours
# the selection.
#
# The DS18B20 + BME280 case is compile+run only: the native Ds18b20 and BMx280
# stubs both read 0, so which sensor provides the temperature is NOT verified.
#
# PLATFORMIO_BUILD_FLAGS is appended after the env build_flags, so -U undoes
# an env -D (gcc: last option wins).
#
# Usage: test/build_variants/check_sensor_selection.sh   (PIO=/path/to/pio to override)
set -u

PIO=${PIO:-pio}
cd "$(dirname "$0")/../.."
FAILED=0
LIGHT_FLAGS="-DUSE_BH1750 -DLACROSSE_ID=LacrosseWS7000::Address::TWO"

# $1: case label, $2: native_cycle env, $3: extra build flags
check_variant() {
  # Given: a native_cycle env with the extra sensor flags
  local build_dir
  build_dir=$(mktemp -d)

  # When: building and running the test_main_cycle suite
  local output status
  output=$(PLATFORMIO_BUILD_DIR="$build_dir" PLATFORMIO_BUILD_FLAGS="$3" \
    "$PIO" test -e "$2" -f test_main_cycle 2>&1)
  status=$?
  rm -rf "$build_dir"

  # Then: the suite builds and passes for this flag combination
  if [ "$status" -ne 0 ]; then
    echo "$output"
    echo "FAIL CONF-BUILD-002 ($1)"
    FAILED=1
  else
    echo "PASS CONF-BUILD-002 ($1)"
  fi
}

check_variant "BME280 takes precedence over BMP280" \
  native_cycle_oregon_vcc "-DUSE_BMP280=1"
check_variant "no temperature sensor: no environment frame" \
  native_cycle_oregon "-UUSE_BMP280"
check_variant "BH1750 without environment protocol" \
  native_cycle_x10 "$LIGHT_FLAGS"
check_variant "BH1750 with Oregon" \
  native_cycle_oregon "$LIGHT_FLAGS"
check_variant "DS18B20 + BME280 builds and runs (source not verified)" \
  native_cycle_oregon_vcc "-DUSE_DS18B20"
exit "$FAILED"
