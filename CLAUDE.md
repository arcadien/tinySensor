# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

PlatformIO is the build system. The `pio` binary lives at `~/.platformio/penv/bin/pio`.

```bash
# Run all native tests (fast — runs on host, no hardware needed)
~/.platformio/penv/bin/pio test -e native_debug

# Run a single test suite
~/.platformio/penv/bin/pio test -e native_debug -f test_oregon

# Build firmware for a specific physical target
~/.platformio/penv/bin/pio run -e S_02

# Build all physical targets
~/.platformio/penv/bin/pio run

# Flash to connected hardware (requires avrdude + programmer)
~/.platformio/penv/bin/pio run -e S_02 --target upload
```

## Architecture

The firmware is entirely build-time configured: every physical sensor unit is a separate PlatformIO environment (`S_02`–`S_05`, `robot`, `devmodule`, `SOLAR_TEST`), each with its own `build_flags` selecting the protocol, sensor type, calibration values, and sleep interval. There is no runtime configuration.

### HAL abstraction

`lib/hal/Hal.h` defines the pure-virtual `Hal` interface (GPIO, delays, ADC, power, hibernate). Two concrete implementations exist:

- `Attiny84aHal` — production, compiled only when `AVR` is defined
- `TestHal_` (singleton `TestHal`) — host-native stub used in all Unity tests; records HAL calls as single-character tokens in `Orders` vector (`'H'`=RadioHigh, `'L'`=RadioLow, `'D'`=Delay512Us, `'P'`=Delay1024Us, `'A'`=Delay400Us)

Protocol encoders receive a `Hal*` at construction and call only HAL methods — never AVR registers directly. This is the pattern to follow when adding new encoders.

### Protocol encoders

| Library | Class | Protocol | Output |
|---|---|---|---|
| `lib/oregon` | `OregonV3` | Oregon Scientific v3 | 433 MHz OOK via `RadioGoHigh/Low` |
| `lib/LacrosseWS7000` | `LacrosseWS7000` | Lacrosse WS7000 | same |
| `lib/x10` | `x10rf` | X10 RF | same — used for battery/analog reporting |
| `lib/softSerial` | `SoftSerial` | UART (debug only) | via `SerialGoHigh/Low` |

`OregonV3` encodes temperature/humidity/pressure into an 11-byte `message[]` private buffer and selects the emulated device ID (THN132N / THGN123N / BTHR918) based on which fields are set at `Send()` time.

### Test approach

Tests run on the host (`-e native_debug`, `-DNATIVE`). The test harness instantiates `TestHal_` and passes it to the encoder under test. Protocol correctness is verified by checking the `Orders` vector against expected character sequences — e.g., a zero bit must produce `{'H','D','L','P','H','D'}`. This verifies timing ordering without hardware.

Test suites: `test_oregon`, `test_lacrosseWS7000`, `test_x10`, `test_conversionTools`, `test_analogFiltering`, `test_power`, `test_led`, `test_busses`.

### `src/main.cpp`

`main()` is a single `while(1)` loop: power on sensors → read → transmit via environment encoder → power off → hibernate. Protocol and sensor selection are entirely via `#ifdef` chains (`USE_OREGON` / `USE_LACROSSE`, `USE_BMP280` / `USE_BME280` / `USE_DS18B20` / `USE_BH1750`). The `BMx280` wrapper (`include/BMx280.h`) abstracts BMP280/BME280 behind a common interface.

### Dual source tree

`TinySensor/` is an Atmel Studio project kept for debugWIRE/JTAG step-debugging. It is intentionally not in sync with `src/` and `lib/` — treat it as a debug tool, not a source of truth.

### RBD workflow

This project uses Requirement-Based Development. **Every code change must originate from a validated requirement.**

- Requirements live in `requirements/*.md` (functional, technical, performance, ui, configuration, platform).
- Architecture is tracked in `docs/architecture.md` — updated with an `arch(ID):` commit whenever a requirement changes the component diagram.
- Commit prefixes are mandatory: `req()`, `test()`, `feat()`, `tech()`, `perf()`, `ui()`, `conf()`, `arch()`, `plan:`.
- Every Unity test function must carry a `// @req FUNC-DOMAIN-NNN` tag on the line above it.
- ID format: `FUNC-DOMAIN-NNN` (e.g. `FUNC-OREGON-001`, `TECH-HAL-001`).

To start a new requirement: invoke `/rbd` and describe the intent. The workflow dispatches `requirement-analyst` → `test-builder` → `code-builder` in sequence.

**Linter**: `clang-format --dry-run --Werror` on all `*.cpp`/`*.h` under `src/`, `lib/`, `include/`, `test/`. Must pass before every test or implementation commit. Run it with:

```bash
find src lib include test -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run --Werror
# Auto-fix:
find src lib include test -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

### Key `build_flags` per environment

| Flag | Meaning |
|---|---|
| `USE_OREGON` / `USE_LACROSSE` | Selects radio protocol |
| `OREGON_RCODE` / `OREGON_CHANNEL` | Oregon rolling code and channel (1–3) |
| `LACROSSE_ID` | Lacrosse address enum value |
| `INTERNAL_1v1` | Calibrated 1.1 V internal reference in mV (chip-specific) |
| `BATTERY_IS_VCC` | Battery voltage = VCC (no external divider) |
| `USE_CHARGE_PUMP` | Board uses a charge pump for sensor power |
| `SLEEP_TIME_IN_SECONDS` | Hibernate duration between measurements |
| `BATTERY_VOLTAGE_X10_ID` / `ANALOG1_X10_ID` | X10 meter IDs for reporting |
