# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

PlatformIO is the build system. The emitter firmware is the PlatformIO project in `emitter/` (`src/`, `include/`, `test/`, `platformio.ini`); `lib/` at the repository root is shared with the receiver `tinyDecoders/`. On this machine `pio` runs as `uvx --from platformio pio`. Run from the repository root:

```bash
# Run all native tests (fast — runs on host, no hardware needed).
# native_debug ignores test_main_cycle, which runs in the native_cycle_* environments.
pio test -d emitter -e native_debug -e native_cycle_oregon -e native_cycle_oregon_vcc -e native_cycle_oregon_divider -e native_cycle_lacrosse -e native_cycle_x10

# Run a single test suite
pio test -d emitter -e native_debug -f test_oregon

# Build firmware for a specific physical target
pio run -d emitter -e S_02

# Build all physical targets
pio run -d emitter

# Flash to connected hardware (requires avrdude + programmer)
pio run -d emitter -e S_02 --target upload
```

Paths below without a folder (`src/`, `include/`, `test/`, `platformio.ini`) are in `emitter/`.

## Architecture

The firmware is entirely build-time configured: every physical sensor unit is a separate PlatformIO environment (`S_02`–`S_05`, `robot`, `devmodule`, `SOLAR_TEST`, `mushroom`), each with its own `build_flags` selecting the protocol, sensor type, calibration values, and sleep interval. There is no runtime configuration.

### HAL abstraction

`lib/hal/Hal.h` defines the pure-virtual `Hal` interface (GPIO, delays, ADC, power, hibernate). Two concrete implementations exist:

- `Attiny84aHal` — production, compiled only when `AVR` is defined
- `TestHal_` (singleton `TestHal`) — host-native stub used in all Unity tests; records HAL calls as single-character tokens in `Orders` vector (`'H'`=RadioHigh, `'L'`=RadioLow, `'D'`=Delay512Us, `'P'`=Delay1024Us, `'A'`=Delay400Us, `'S'`/`'W'`=SerialWriteBit high/low)

Protocol encoders receive a `Hal*` at construction and call only HAL methods — never AVR registers directly. This is the pattern to follow when adding new encoders.

### Protocol encoders

| Library | Class | Protocol | Output |
|---|---|---|---|
| `lib/oregon` | `OregonV3` | Oregon Scientific v2.1 (frame layout; the class keeps its old name) | 433 MHz OOK via `RadioGoHigh/Low` |
| `lib/LacrosseWS7000` | `LacrosseWS7000` | Lacrosse WS7000 | same |
| `lib/x10` | `x10rf` | X10 RF | same — used for battery/analog reporting |
| `lib/softSerial` | `SoftSerial` | UART (debug only) | via `SerialWriteBit` |

`OregonV3` encodes temperature/humidity/pressure into an 11-byte `message[]` private buffer and selects the emulated device ID (THN132N / THGR228N / BTHR918) based on which fields are set at `Send()` time.

### Test approach

Tests run on the host (`-e native_debug` and the `native_cycle_*` environments, `-DNATIVE`). The test harness instantiates `TestHal_` and passes it to the encoder under test. Protocol correctness is verified by checking the `Orders` vector against expected character sequences — e.g., a zero bit must produce `{'H','D','L','P','H','D'}`. This verifies timing ordering without hardware.

Test suites: `test_oregon`, `test_lacrosseWS7000`, `test_x10`, `test_conversionTools`, `test_power`, `test_led`, `test_softSerial`, `test_ds18b20`, `test_x10_hal`, `test_environment`, `test_main_cycle` (only in `native_cycle_*`).

### `src/main.cpp`

`main()` loops on `runCycle(&hal)`; one cycle: power on sensors → `ReadEnvironment()` fills a `Measurements` (`include/Measurements.h`) → X10 meters → luminosity → environment frame → power off → hibernate. The environment protocol is a build-time alias, `EnvironmentEncoder` (`include/EnvironmentEncoder.h`), loaded with `ApplyMeasurements()` — no virtual call. Luminosity (`LightSensor`, BH1750) always goes out through a dedicated Lacrosse encoder. Sensor selection stays via `#ifdef` (`USE_BMP280` / `USE_BME280` / `USE_DS18B20` / `USE_BH1750`); wrappers (`BMx280`, `Ds18b20`, `LightSensor`) take a `Hal*` and stub out on native builds. Details: `docs/architecture.md`, section *Main loop structure*.

### Debugging

Step-debugging uses PlatformIO + AVaRICE + AVR Dragon over debugWIRE (the legacy Atmel Studio tree was removed). Toolchain, DWEN fuse procedure and hardware constraints (v3.0 meets them with R5 10 kΩ and no RESET capacitor; v2.2 and earlier need a dedicated debug board) are in [`docs/debugging.md`](docs/debugging.md).

### RBD workflow

This project uses Requirement-Based Development. **Every code change must originate from a validated requirement.**

- Requirements live in `requirements/*.md` (functional, technical, performance, ui, configuration, platform).
- Architecture is tracked in `docs/architecture.md` — updated with an `arch(ID):` commit whenever a requirement changes the component diagram.
- Commit prefixes are mandatory: `req()`, `test()`, `feat()`, `tech()`, `perf()`, `ui()`, `conf()`, `arch()`, `plan:`.
- Every Unity test function must carry a `// @req FUNC-DOMAIN-NNN` tag on the line above it, and every bench sheet a `<!-- @req ID -->` tag on its first line.
- ID format: `FUNC-DOMAIN-NNN` (e.g. `FUNC-OREGON-001`, `TECH-HAL-001`).

Requirements are a spec, not a plan. Each one passes four checks:
1. **Observable** from outside the unit (radio output, pins, screen, serial lines, build result), or a design constraint that has a reason (testability, memory, no hardware access).
2. **Survives a refactor**: no class, method, file or register name, except at a boundary interface (`Hal`, `ReceiverHal`, `runCycle`, build flags, environments).
3. **States its reason** when it is not obvious.
4. **Verifiable on its own**.
- The "how" (classes, helpers, registers, file layout) goes in `docs/architecture.md` and code comments.
- **No test information in a requirement**: no "Verification:", "Test:", "Verified by", "native test", "bench check pending". Input → output examples that define the behaviour stay, as "Examples:".
- A helper's unit tests are tagged with the requirement whose behaviour uses the helper (`dec16ToHex` tests → FUNC-X10-006/007).
- A requirement no software test can cover gets a bench test sheet `tests/bench/<ID>.md`, tagged like a test: `<!-- @req <ID> -->` on its first line, one line per requirement it covers (requirement link, equipment, setup, steps, pass criteria, results table filled at each run); Bench tests are run by hand before each release, in the order of `tests/bench/bring-up.md`; a release is tagged only when every sheet has a passing line for it.

To start a new requirement: invoke `/rbd` and describe the intent. The workflow dispatches `requirement-analyst` → `test-builder` → `code-builder` in sequence.

Every report in `audits/` (`/rbd-audit`, `/rbd-review`) states on its first line under the title the full hash of the commit it audited (`Audited commit:` or `Reviewed commit:`), so `git diff <hash>..HEAD -- requirements docs emitter lib tinyDecoders` shows what changed since the last audit.

**Linter**: `clang-format` **18.1.8** (`--dry-run --Werror`) on all `*.cpp`/`*.h` under `emitter/` (`src/`, `include/`, `test/`), `lib/` and `tinyDecoders/`. Must pass before every test or implementation commit; enforced by the CI `lint` job and by the pre-commit hook (`pip install pre-commit && pre-commit install`). Other clang-format versions may format macros differently. Run it with:

```bash
find emitter/src emitter/include emitter/test lib tinyDecoders -path '*/.pio' -prune -o \( -name '*.cpp' -o -name '*.h' \) -print | xargs clang-format --dry-run --Werror
# Auto-fix:
find emitter/src emitter/include emitter/test lib tinyDecoders -path '*/.pio' -prune -o \( -name '*.cpp' -o -name '*.h' \) -print | xargs clang-format -i
```

### Key `build_flags` per environment

| Flag | Meaning |
|---|---|
| `USE_OREGON` / `USE_LACROSSE` | Selects radio protocol |
| `OREGON_RCODE` / `OREGON_CHANNEL` | Oregon rolling code and channel (1–3) |
| `LACROSSE_ID` | Lacrosse address enum value |
| `INTERNAL_1v1` | Calibrated 1.1 V internal reference in mV (chip-specific) |
| `BATTERY_IS_VCC` | Battery voltage = VCC (no external divider) |
| `BATTERY_DIVIDER` | Battery / PA1 voltage ratio of the divider (default 1; 2 on v2.2 rework and v3.0, CONF-BUILD-012) |
| `OREGON_NO_PRESSURE` | BME280 Oregon board sends THGR228N temperature + humidity, no pressure (requires `USE_OREGON`, CONF-BUILD-013) |
| `USE_CHARGE_PUMP` | Board uses a charge pump for sensor power |
| `USE_SWITCHED_5V` | PB2 (J8) drives a PNP that powers an off-board 5 V module during measurements (LM35 variant, `hardware/tinysensor/VARIANTE_LM35.md`, PLAT-POWER-006) |
| `SLEEP_TIME_IN_SECONDS` | Hibernate duration between measurements |
| `BATTERY_VOLTAGE_X10_ID` / `ANALOG1_X10_ID` | X10 meter IDs for reporting |

## Hardware Conventions

### Wire colors
Power goes in pairs (thick wire):
| Pair (+ / −)  | Use |
|---------------|-----|
| red / black   | battery and 5V: B+/B−, VBAT, switched battery rail, VUSB |
| white / blue  | regulated or stepped voltages: 3V3, buck/boost outputs (never yellow) |
| grey          | speaker |

Signals (thin wire), by role:
| Color  | Use |
|--------|-----|
| yellow | clocks: SCK, BCLK, SCL, SD_CLK |
| green  | data: MOSI, DIN, SDA, SD_D0 |
| purple | select / frame: CS, LRC/WS, SD_CMD |
| red    | status: DC, interrupts, PIR out |
| orange | reset, detect (RST, USB_DET) |
| brown  | enables / analog: BL_EN, VBAT/2, *_SD, *_EN |

The kit has 10 colors: red, black, white, blue, yellow, green, purple, orange, brown, grey. **Blue is reserved for power** (regulated GND) and never carries a signal. When several buses share colors, tell them apart with 1 / 2 / 3 heat-shrink rings at each end.

### Wire ends
Label every wire end as **soldered**, **Dupont** or **JST-XH** (with F/M contact), on both ends, and give each wire's length, in every doc and diagram.

### Resistors
Current-limiting and pull resistors sit **at the board end**, never at the LED or the sensor.

### Images
Strip EXIF/GPS from every image before committing: `python3 ~/.claude/skills/hardware-change/scripts/strip_exif.py <paths>` (`--check` to verify).

### Commits
The rbd plugin's pre-push hook (a Claude Code hook on `git push`) checks the prefixes. It accepts the RBD ones above (`req()`…`arch()` must cite a validated ID), `plan:`, `hw:` for hardware (`enclosing/`, `hardware/`, from `.rbd/config.yml` `extra_commit_prefixes`), `docs:` for doc-only commits, and `fix:`, `chore:`, `style:`, `refactor:`. Make granular commits, one per area, and push after each.

## Modeling

### OpenSCAD / Printing
- Define all parameters at the top of the file, before any formula that uses them. Expose dimensions as named parameters.
- After editing, render with `openscad -o` and check for errors before saying it's done.
- Don't claim parts fit an Ender 3 (220x220) or CR-10 plate unless you've checked the bounding boxes. Flag any overhang over 45° that a design change introduces.
- Regenerating 3MF files wipes the Orca plate layout, so warn me before doing it.
- **Never use `mirror()` to put a part in its print orientation**: a mirror flips its handedness, and the printed part can't be turned back to fit (the mushroom gills disc was printed mirrored and didn't fit the lobed cap). To flip a part upside down, use `rotate([180, 0, 0])`. Before exporting a part whose print transform changed, compare it with the assembly (e.g. the sign of the xy moment of its vertices).
