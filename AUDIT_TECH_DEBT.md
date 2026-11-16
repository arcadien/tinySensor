# Technical debt audit — TinySensor

**Project**: [arcadien/tinySensor](https://github.com/arcadien/tinySensor) — low-power AVR firmware for wireless environment sensors (ATtiny84a, 433 MHz, Oregon v3 / Lacrosse WS7000 / X10 protocols).

**Auditor**: Claude (opus 4.7 model) — static review

**Date**: 19 April 2026 — updated 19 June and 16 November 2026 (see §1bis)

**Scope** (initial audit; the tree has changed since, see §1bis 2026-11-16): whole repository at commit `aa53a83` (branch `master`) — firmware code (`src/`, `lib/`, `include/`), tests (`test/`), legacy Atmel Studio project (`TinySensor/`), tooling (`scripts/`, `shared/`), web builder (`web/`), hardware (`hardware/`), CI (`.github/workflows/`), Docker/devcontainer.

---

## 1. Executive summary

> Initial summary of April 2026, kept as is. Every point it names is resolved; what is still open is in §4.

TinySensor is a functionally mature embedded firmware project (tested radio protocols, clean HAL, native unit tests with Unity, PlatformIO port). The "core" code (`lib/oregon`, `lib/LacrosseWS7000`, `lib/x10`, HAL) is clean, well abstracted and tested. The debt is concentrated in three areas that weigh on maintainability and operational risk:

First, **a Coveralls secret is committed in clear text** in `.coveralls.yml` — it must be revoked immediately. Second, **the CI builds no AVR firmware** (only the native environment is tested), so a refactoring can silently break the production builds `S_02`..`S_05`. Third, **a second source tree** (`TinySensor/`, legacy Atmel Studio project) duplicates the firmware code and holds versioned build binaries; it is a certain source of divergence and bloats the git history.

Beyond these three points, the per-device configuration lives as near-identical blocks in `platformio.ini` (7 copy-pasted environments), the dependencies all pull from unpinned personal forks, the `php:7.4-apache` Docker image is EOL (November 2022), and several tooling scripts contain author-specific absolute paths. The whole is manageable in ~3 sprints of a part-time developer, with the risk gains concentrated in the first two weeks.

**Key figures**: 3,779 lines of code+tests over ~20 files. 8 native Unity test suites. 1 GitHub Actions workflow (native build + test only). 6 PlatformIO envs for distinct physical targets. 1 exposed secret. 2 critical EOL dependencies.

---

## 1bis. Audit updates

This section tracks the debt as fixes land. Each entry gives the resolved item, the date, the file(s) touched and the impact on the overall score.

### 2026-04-19 — GitHub Actions workflow rebuilt

File changed: `.github/workflows/main.yml`

Three top-20 items handled in a single PR:

- **T1 (36) — CI now builds every AVR target**. The workflow was split into two jobs: `test-native` (unchanged) and `build-avr` with the matrix `[S_02, S_03, S_04, S_05, robot, devmodule, SOLAR_TEST]`. `fail-fast: false` to isolate failures per env. → **Resolved**.
- **D2 (30) — Actions and Python updated**. `checkout@v4`, `cache@v4`, `setup-python@v5`, Python `3.12` (replaces EOL 3.9). Cache key now invalidated on `hashFiles('platformio.ini')`. → **Resolved**.
- **I4 (20) — Firmware artefacts published**. Each AVR build uploads `firmware.hex` + `firmware.elf` through `actions/upload-artifact@v4`, 30-day retention. Covers the "downloadable artefact" part of I4; the "release automation on tag" part remains. → **Partly resolved**.

Also: triggering extended to pull requests (previously `on: push` only), and cache configured with a `restore-keys` fallback.

Overall impact: **cumulated score reduced by 86 points** (36 + 30 + 20). Top 20 rebuilt in §4.

### 2026-04-19 — `.gitignore` cleaned and artefact patterns added

File changed: `.gitignore`

- **I7 (20)** — Buggy line 1 `Debug/ Release/` (a single literal pattern with a space) removed. File reorganised into commented blocks. Added the patterns `*.o`, `*.d`, `*.hex`, `*.eep`, `*.lss`, `*.map`, `*.srec` (`*.elf` was already there). CRLF line endings preserved. → **Resolved**.
- **I1 (25)** — Audit revisited. `git ls-files TinySensor/Debug/` returns nothing: **no artefact was actually tracked** (the `.o`/`.hex`/`.elf` on disk come from an uncommitted local Atmel Studio build). The `git rm --cached` planned is a no-op. The new `.gitignore` prevents any regression: `git check-ignore` confirms every file of `TinySensor/Debug/` matches the `Debug/` rule. → **Resolved (by prevention; the initial audit overestimated the item).**

Cumulated impact 2026-04-19 (all entries): **131 points** (86 + 25 + 20).

### 2026-06-16 — MAGI review + CI/tooling clean-up (PR #24)

Branch: `fix/magi-review-findings` — independent review by three MAGI agents (Gaspard, Melchior, Balthazar) who voted REJECT 2/3 on firmware bugs, followed by a fixing session.

**New firmware bugs found by MAGI (not in the initial audit):**

- **M1 — `PRR &= ~_BV(PRTIM1/PRADC)` inverted logic in `UseLessPowerAsPossible()`** (`lib/hal/Attiny84aHal.cpp`). In the PRR register, setting a bit turns the peripheral off; `&= ~` turned it on (Timer1 and ADC stayed on all the time). Fixed to `|=`. Retroactive score: (3+3)×5 = **30**. → **Resolved**.
- **M2 — `SetChannel()` with no bounds check** (`lib/oregon/Oregon_v3.cpp`). Channel > 3 shifted bits into the sensor ID nibbles, silently corrupting `message[2]`. Guard added. Retroactive score: (3+3)×5 = **30**. → **Resolved**.
- **M3 — `message[]` file-scope global, not a class member** (`lib/oregon/Oregon_v3.cpp`). Accidental singleton: two `OregonV3` instances would have shared the same buffer. Moved to a private member. Retroactive score: (2+2)×4 = **16**. → **Resolved**.

**Audit items resolved in this PR:**

- **C4 (20) — Dead code `if (int(count) != count)` in `OregonV3::Sum`**. Inherited from an internet implementation where `count` was a `float`; with `uint8_t`, the branch is always false. Removed. → **Resolved**.
- **C7 (18) + C10 (15) — Massive `platformio.ini` duplication**. Introduced `[env:avr_base]` + `extends`; removed the duplicate `USE_BMP280=1` in `env:S_03`. → **Resolved**.
- **D9 (25) — No Dependabot**. Added `.github/dependabot.yml`: weekly updates for `github-actions`, monthly for `docker` and `pip`. → **Resolved**.
- **I4 (12) — Release automation on tag** (remaining part). `release` job added to the workflow: triggers on `v*`, downloads every firmware artefact and publishes a GitHub Release with auto-generated notes. → **Resolved (completely)**.
- **D8/Doc7 (10) — Dead Travis badge**. Replaced by the GitHub Actions badge in `README.md`. → **Resolved**.

Impact 2026-06-16: **176 points** resolved (M1:30 + M2:30 + M3:16 + C4:20 + C7:18 + C10:15 + D9:25 + I4:12 + D8:10).

Cumulated impact, all dates: **307 points** (131 + 176).

### 2026-06-18 — Coveralls token revoked + `web/` removed + A1 requalified

- **C1 (35) — Coveralls token revoked and out of the repo**. The token `K1UMmvKnmugffPWriB48kR274KXcAuvuK` in `.coveralls.yml` was revoked on coveralls.io and replaced by a GitHub Actions secret `COVERALLS_REPO_TOKEN`. → **Resolved**.
- **D1 (36) + A4 (15) + D6 (12) — `web/` removed**. Decision: the web firmware builder was never finished or deployed. Both files removed (`web/build.php`, `web/index.html`). Removes at once the EOL PHP 7.4 Docker image, the `shell_exec` vulnerability, and jQuery 1.12.4. → **Resolved (D1, A4, D6)**.
- **A1 — Requalified (accepted risk)**. `TinySensor/` is not dead code: it is kept on purpose for the Atmel Studio debug/flash workflow (debugWIRE/JTAG), which PlatformIO cannot replace. Score lowered from 32 → 20, category "divergence risk to watch". Recommended mitigations: cross comments in both `main.cpp` + a warn-only CI `diff` step. → **Requalified, not removed**.
- **C2 (30) — Dead `OREGON_DELAY_US` macro removed**. The macro was never called (Oregon uses `_hal->Delay512Us()/Delay1024Us()` directly) and referred to a non-existent method. Removed from `TestHal.h`. → **Resolved**.
- **C15 (15) — `override` added to the `TestHal` delay stubs**. `Delay400Us`, `Delay1024Us`, `Delay512Us` lacked `override`; signature drift is now caught at compile time. → **Resolved**.

### 2026-10-25 — Legacy Atmel Studio tree removed

- **A1 (20) + D7 (16) — `TinySensor/` removed**. Decision: step debugging goes through PlatformIO + AVaRICE + AVR Dragon (debugWIRE), documented in `docs/debugging.md` (to validate on the bench; a dedicated debug board is needed). The vendored third-party copies go with the folder. → **Resolved**.

### 2026-10-25 — Coveralls dropped

- **T2 (12) — Dropped**. Decision: no coverage report through Coveralls. `.coveralls.yml` was already gone and no CI step used it. The GitHub Actions secret `COVERALLS_REPO_TOKEN` remains to delete by hand (Settings → Secrets and variables → Actions). → **Closed without action**.

- **C3 (20) — Orphan `Init()` removed**. Free function `void Init(void)` at the bottom of `Attiny84aHal.cpp`, outside the class, never called — a silent shadow of `Attiny84aHal::Init`. Removed. → **Resolved**.
- **C11 (15) — Dead `SetPressure` branch removed**. `if (pressure < 200) pressure = 200;` was immediately overwritten by the clamp at 850; removed. → **Resolved**.
- **D5 (20) — `avrdude.conf` out of the repo**. The file (534 KB) comes from the PlatformIO toolchain (`~/.platformio/packages/tool-avrdude/`); removed with `git rm`. → **Resolved**.
- **I3 — Requalified**. Audit revisited: the KiCAD ZIP was not tracked by git (on disk only); `*.zip` already covered by `.gitignore`. Non-issue, like I1. → **Resolved (by prevention)**.
- **I2 (25) — Absolute paths parameterised**. `Makefile`: `PIO ?= $(HOME)/.platformio/penv/bin/pio`, `TARGET ?= S_03`, `cp` replaces `copy`. `scripts/flash_tinySensor`: `PIO_HOME` and `PROJECT_DIR` with portable defaults (`${HOME}/.platformio` and `$(dirname $0)/..`). → **Resolved**.
- **D3 (24) + D4 (24) — PlatformIO dependencies pinned to a SHA**. The 5 dependencies (BH1750, tiny-i2c, Unity, avr-ds18b20, SparkFun_BME280) pinned to their current HEAD commit in `platformio.ini`. Reproducibility guaranteed, supply-chain risk removed. → **Resolved**.
- **Doc1 (20) — `docs/environments.md` created**. Convention `S_*` = number on the PCB, `INTERNAL_1v1` calibration guide per board, table of physical boards + special envs (`robot`, `devmodule`, `SOLAR_TEST`), battery thresholds per chemistry, procedure to add a new board. → **Resolved**.

Impact 2026-06-18: **291 points** resolved (C1:35 + D1:36 + A4:15 + D6:12 + C2:30 + C15:15 + C3:20 + C11:15 + D5:20 + I2:25 + D3:24 + D4:24 + Doc1:20).

Cumulated impact, all dates: **598 points** (307 + 291).

### 2026-06-19 — RBD setup, requirements reverse-engineering, architectural violations V1-V4

#### RBD infrastructure

The project moved to **Requirement-Based Development (RBD)** through the Claude Code plugin `arcadien/rbd@0.12.0`. This includes:

- `.rbd/config.yml` — ID format `FUNC/TECH/CONF/PLAT-DOMAIN-NNN`, Unity/C++ framework, `clang-format` linter
- `.clang-format` — Google style, 2 spaces, 80 columns, `SortIncludes: false`
- clang-format baseline applied to every existing `.cpp`/`.h` (37 files)
- `CLAUDE.md` updated with the RBD section (mandatory commit prefix, `// @req {id}`, etc.)
- Pre-push hook `~/.claude/hooks/rbd-pre-push-check.sh` installed

#### Requirements reverse-engineering

Full architectural analysis (`docs/analysis-2026-06-18.md`) followed by 8 waves of parallel `requirement-analyst` agents. **67 requirements validated and committed**:

| File | Domains | Requirements |
|---------|----------|:------------:|
| `requirements/functional.md` | FUNC-OREGON, LACROSSE, X10, BATTERY, SENSOR, ANALOG | 37 |
| `requirements/technical.md` | TECH-HAL, FILTER, CONVERT, SERIAL, X10 | 14 |
| `requirements/configuration.md` | CONF-BUILD | 11 |
| `requirements/platform.md` | PLAT-POWER | 5 |
| **Total** | | **67** |

`docs/architecture.md` extended with a full traceability table (67 Requirement → Component rows).

#### Architectural violations V1-V4 resolved

- **V1 — `x10rf` bypassed the HAL for every timing**: 5 local static functions (`x10_preamble_high`, etc.) called `_delay_us()` directly, violating `TECH-HAL-001`. Full RBD cycle: TECH-X10-001 (5 new `Hal` methods: `DelayX10PreambleHigh` 8960 µs, `DelayX10PreambleLow` 4500 µs, `DelayX10BitShort` 560 µs, `DelayX10BitLong` 1120 µs, `DelayX10Gap` 40,000 µs) + TECH-X10-002 (x10rf replaced — `#if defined(AVR) _delay_us` block removed, 5 `_hal->DelayX10*()` calls substituted). 14 new Unity tests. → **Resolved**. Note: **C6 (SoftSerial) stays open** — same pattern, handled separately.

- **V2 — Dead `#include <AnalogFilter.h>` in `Attiny84aHal.cpp`**: included on line 2, never referenced — `AdcRead` used local variables. Removed. → **Resolved**.

- **V3 — `BH1750` missing from `docs/architecture.md`**: added to the Mermaid diagram and the responsibility table. → **Resolved**.

- **V4 — `ds18b20` missing from `docs/architecture.md`**: same. → **Resolved**.

#### Devcontainer

- **DevContainer**: Node.js 20 LTS + `@anthropic-ai/claude-code` added to `.devcontainer/Dockerfile` (system install through NodeSource). Claude Code now available after `Rebuild Container`.

#### C5 — DS18B20 access wrapped in the `Ds18b20` class

- **C5 (20) — `main.cpp` accessed the AVR registers directly for the DS18B20** (`ds18b20convert(&PORTA, &DDRA, &PINA, …)`). Full RBD cycle: TECH-SENSOR-001 validated, 4 native Unity tests (`test/test_ds18b20/`), class `include/Ds18b20.h` created following exactly the `BMx280` pattern (`#if defined(AVR)` / `#else` stub sections, `Hal*` constructor). `main.cpp` reduced to `ds18b20sensor.Convert()` + `ds18b20sensor.Read()` — no remaining reference to `PORTA`/`DDRA`/`PINA`. → **Resolved**.

Impact 2026-06-19: **20 points** resolved (C5:20). Cumulated score: **618 points** (598 + 20).

### 2026-10-25 — SoftSerial through the HAL, bit macros, BCD saturation, format tooling

- **C6 (20) + T4 (16) — `SoftSerial` goes through the HAL**. `SerialGoHigh/Low` replaced by `Hal::SerialWriteBit(level)`, which sets the level and holds one bit period: a single virtual call per bit as before, AVR timing preserved (within 1–2 cycles, 40 bytes of flash less). The period (`SERIAL_BIT_PERIOD_US`) lives in `Attiny84aHal`. New native suite `test_softSerial` (6 tests: frame, strings, numbers, bases, `println`). TECH-SERIAL-001, `docs/architecture.md` and CLAUDE.md updated. → **Resolved**.
- **C9 (16) — `dec16ToHex` saturates at `0x9999`** beyond 9999 instead of overflowing. TECH-CONVERT-001 and test updated. → **Resolved**.
- **C12 (12) — Bit macros centralised** in `lib/utils/include/bitOps.h`; copies removed (Oregon, Lacrosse, x10rf, public `x10rf.h`, X10 test). Encoder AVR code identical (`.text` size unchanged). → **Resolved**.
- **C8 (16) — Constants**: dead ones removed (`HALF_DELAY_US`, `DELAY_US`, `MAX_ROLLING_CODE_VALUE`, `SYNC_NIBBLE`, X10 timings of `x10rf.cpp`), `PRESSURE_SCALING_VALUE` documented, X10 timings named and documented in the HAL. → **Resolved**.
- **C14 (12) — X10 TODO** replaced by a scope note (only the normal packet type is used). → **Resolved**.
- **I6 (12) — Format tooling**: `.editorconfig`, clang-format **18.1.8** pre-commit hook, CI `lint` job at the same version; one-off reformatting (whitespace only) of `BMx280.h`, `Ds18b20.h`, `x10rf.cpp`. → **Resolved**.

Impact 2026-10-25: **104 points** resolved (C6:20 + T4:16 + C9:16 + C8:16 + C12:12 + C14:12 + I6:12). Cumulated score: **722 points** (618 + 104), excluding PR #35 (T2) and #36 (A1, D7).

### 2026-10-25 — `main.cpp` restructuring

- **A2 (15) — Common interface for the environment encoders**. `include/EnvironmentEncoder.h`: `EnvironmentEncoder` alias chosen at build time (Oregon or Lacrosse), `ConfigureEncoder()` and one `ApplyMeasurements()` overload per encoder. Static rather than virtual choice (no vtable on 8 KB of flash). The repeated `#if USE_OREGON/USE_LACROSSE` disappear from the loop. → **Resolved**.
- **A3 (20) — BH1750 out of the loop**. Wrapper `include/LightSensor.h` (`BMx280`/`Ds18b20` model, native stub); sensor and Lacrosse light encoder created once. Luminosity still goes out as Lacrosse (Oregon has no light frame), twice (`LIGHT_FRAME_REPEAT`). → **Resolved**.
- `include/Measurements.h` replaces the `NOT_SET = -1000` sentinel floats with presence-flag fields.
- Defects fixed along the way: direct `_delay_ms(150)` in `main.cpp` (now goes through `Hal::Delay30ms`); `vccMv` used by the `ANALOG1_X10_ID` block but declared only in the battery block; missing native `BMx280::Shutdown()` stub.
- New suite `test_environment` (7 tests): equivalence of the HAL orders between `ApplyMeasurements()` and the former direct calls. Structure documented in `docs/architecture.md` (*Main loop structure*).

Impact 2026-10-25 (b): **35 points** resolved (A2:15 + A3:20). Cumulated score: **757 points** (722 + 35).


### 2026-11-16 — Review of the remaining items, repository layout

Repository changes since the last entry (PRs #69, #70): the emitter firmware is its own PlatformIO project in `emitter/` (`src/`, `include/`, `test/`, `platformio.ini`; `lib/` stays at the root, shared with the receiver `tinyDecoders/`), the 3D models moved from `openscad/` to `enclosing/`, the old `doc/` images, `docs/analysis-2026-06-18.md`, the root `Dockerfile`, the docker scripts, `waitForUpload` and the CMake scripts were removed, and the unread `SENSOR_ID` / `SENSOR_CHANNEL` / `OREGON_MODE` flags left `platformio.ini`.

Each open item was checked against the code:

- **T2 — closed**: the `COVERALLS_REPO_TOKEN` secret is gone (`gh secret list` is empty).
- **C13 (10) — closed, accepted**: `SerialGoHigh/Low` is now `SerialWriteBit`; `RadioGoHigh/Low` stays, no port to another MCU is planned.
- **A5 (10) — closed, accepted**: build-time configuration is a deliberate choice for a handful of boards.
- **T5 (12) — closed, won't do**: running simavr in CI is not worth it here; `SOLAR_TEST` is built only.
- **T6 (12) — closed, partly covered**: the `tinyDecoders` native tests decode the frames of the repository encoders with a decoder modelled on RFLink (Plugin_048); no RTL_433 run.
- **Doc2 (20) — closed, partly covered**: the README *Development* section and `CLAUDE.md` give the stack and the commands.
- **Doc5 (12) — closed, won't do**: `CONTRIBUTING.md`, `SECURITY.md`, `CODE_OF_CONDUCT.md`, `CHANGELOG.md` add nothing to a personal project.
- **I5 (20) — closed, won't do**: a matrix of Python / PlatformIO versions has no value here.
- **Doc6** — text updated: `doc/` no longer exists.

Still open, to handle one after the other: Doc3, Doc4, T3, T7, I8, Doc6, A6 (§4, §5).

---

## 2. Methodology

Each item is scored with the tech-debt framework formula:

**Priority = (Impact + Risk) × (6 − Effort)**

- **Impact** (1–5): how much does it slow the team down or degrade quality?
- **Risk** (1–5): what happens if it is not fixed?
- **Effort** (1–5): cost of the fix (1 = trivial, 5 = big job).

The highest theoretical score is 50 ((5+5)×(6−1)). Items fall into four bands:

| Band | Score | Handling |
|---|---|---|
| P0 — Critical | ≥ 30 | Within 1–2 weeks |
| P1 — High | 20–29 | Current or next sprint |
| P2 — Medium | 12–19 | Quality backlog over a quarter |
| P3 — Low | < 12 | Opportunistic, when touching the area |

---

## 3. Debt by category

### 3.1 Code debt

| ID | Item | Location | Impact | Risk | Effort | Priority |
|----|------|--------------|:------:|:------:|:------:|:--------:|
| ~~C1~~ | ~~Coveralls token committed in clear (`repo_token: K1UMmvKnmugffPWriB48kR274KXcAuvuK`)~~ — ✅ **Resolved 2026-06-18** (token revoked, moved to the GitHub Actions secret `COVERALLS_REPO_TOKEN`) | `.coveralls.yml` | 2 | 5 | 1 | ~~35~~ |
| ~~C2~~ | ~~`TestHal` refers to an `OREGON_DELAY_US(x) TestHal.Delay(x)` that does not exist.~~ — ✅ **Resolved 2026-06-18** (macro never called — Oregon uses `_hal->Delay512Us()/Delay1024Us()` directly; dead macro removed) | `lib/hal/TestHal.h:74` | 3 | 3 | 1 | ~~30~~ |
| ~~C3~~ | ~~Orphan free function `void Init(void)` at the bottom of `Attiny84aHal.cpp`, outside the class, never called.~~ — ✅ **Resolved 2026-06-18** (function removed) | `lib/hal/Attiny84aHal.cpp:199-203` | 2 | 2 | 1 | ~~20~~ |
| ~~C4~~ | ~~Dead code in `OregonV3::Sum`: `if (int(count) != count)` is always false for a `uint8_t`.~~ — ✅ **Resolved 2026-06-16** (dead branch removed; inherited from an internet impl. with `float count`) | `lib/oregon/Oregon_v3.cpp:82` | 2 | 2 | 1 | ~~20~~ |
| ~~C5~~ | ~~`main.cpp` accesses registers directly (`ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr)`), bypassing the HAL.~~ — ✅ **Resolved 2026-06-19** (class `Ds18b20` created in `include/Ds18b20.h` — same pattern as `BMx280`; `main.cpp` calls `ds18b20sensor.Convert()` / `ds18b20sensor.Read()`, no more reference to `PORTA`/`DDRA`/`PINA`; TECH-SENSOR-001 + 4 Unity tests) | `src/main.cpp:116,120` | 3 | 2 | 2 | ~~20~~ |
| ~~C6~~ | ~~`SoftSerial` uses `_delay_us()` directly instead of the HAL → impossible to test natively and breaks the abstraction contract.~~ — ✅ **Resolved 2026-10-25** (`SerialWriteBit` HAL, no more `_delay_us` in SoftSerial) | `lib/softSerial/SoftSerial.cpp:3-6,16,24,32,36` | 3 | 2 | 2 | ~~20~~ |
| ~~C7~~ | ~~Per-env config with massive duplication in `platformio.ini`: S_02, S_03, S_04, S_05, robot, devmodule, SOLAR_TEST share 80 % of their `build_flags`.~~ — ✅ **Resolved 2026-06-16** (`[env:avr_base]` + `extends`; `default_envs` to keep the base out of the builds) | `platformio.ini` | 4 | 2 | 3 | ~~18~~ |
| ~~C8~~ | ~~Undocumented magic numbers: `PRESSURE_SCALING_VALUE = 795`, `MAX_ROLLING_CODE_VALUE = 0xA5` (unused), `SYNC_NIBBLE = 0x0A` (unused), X10 timings (`PREAMBLE_LOW = 4500`, etc.) with no datasheet reference.~~ — ✅ **Resolved 2026-10-25** (dead constants removed, `PRESSURE_SCALING_VALUE` and X10 timings documented) | `lib/oregon/Oregon_v3.cpp`, `lib/x10/x10rf.cpp` | 2 | 2 | 2 | ~~16~~ |
| ~~C9~~ | ~~`dec16ToHex` silently overflows for `input > 9999` (the test `conversionToolsTest.cpp` shows it with `10000 → 0xA000` commented "overflow"). No `assert` or clamp.~~ — ✅ **Resolved 2026-10-25** (saturates at `0x9999`, TECH-CONVERT-001 updated) | `lib/utils/src/conversionTools.cpp:5-22` | 2 | 2 | 2 | ~~16~~ |
| ~~C10~~ | ~~`USE_BMP280=1` declared **twice** in `env:S_03` (lines 88 and 90) — leftover copy-paste.~~ — ✅ **Resolved 2026-06-16** (subsumed by C7; the `extends` refactoring removed the duplicate) | `platformio.ini:88,90` | 2 | 1 | 1 | ~~15~~ |
| ~~C11~~ | ~~Dead branch in `LacrosseWS7000::SetPressure`: `if (pressure < 200) pressure = 200;` is immediately overwritten by `if (pressure < 850) pressure = 850;`.~~ — ✅ **Resolved 2026-06-18** (dead branch removed) | `lib/LacrosseWS7000/LacrosseWS7000.cpp:24-25` | 1 | 2 | 1 | ~~15~~ |
| ~~C12~~ | ~~`BITREAD` macro redefined in 4 files (`Oregon_v3.cpp`, `LacrosseWS7000.cpp`, `x10rf.cpp`, `x10rf.h`) with variants (`bitRead`, `BITREAD`).~~ — ✅ **Resolved 2026-10-25** (`lib/utils/include/bitOps.h`) | (multiple) | 2 | 1 | 2 | ~~12~~ |
| ~~C13~~ | ~~"Leaky" HAL: methods named `RadioGoHigh/Low`, `SerialGoHigh/Low` — semantic IO driving, not a real radio bus abstraction.~~ — ⛔ **Closed 2026-11-16** (accepted: `SerialWriteBit` replaced the serial pair, no port planned) | `lib/hal/Hal.h` | 3 | 2 | 4 | ~~10~~ |
| ~~C14~~ | ~~Lone TODO: `// TODO: tests for other modes`~~ — ✅ **Resolved 2026-10-25** (TODO replaced by a scope note) | `test/test_x10/x10LibTest.cpp:130` | 2 | 2 | 3 | ~~12~~ |
| ~~C15~~ | ~~`TestHal::Delay400Us/Delay1024Us/Delay512Us` lack `override`.~~ — ✅ **Resolved 2026-06-18** (`override` added on the three methods) | `lib/hal/TestHal.h:41-44` | 2 | 1 | 1 | ~~15~~ |
| ~~M1~~ | ~~`PRR &= ~_BV(PRTIM1/PRADC)` inverted logic in `UseLessPowerAsPossible()`: the peripherals stayed on (wasted battery power).~~ — ✅ **Resolved 2026-06-16** (found by the MAGI review, fixed to `\|=`) | `lib/hal/Attiny84aHal.cpp` | 3 | 3 | 1 | ~~30~~ |
| ~~M2~~ | ~~`OregonV3::SetChannel()` with no bounds check: channel > 3 silently corrupts `message[2]` by overwriting the sensor ID nibbles.~~ — ✅ **Resolved 2026-06-16** (guard `if (channel < 1 \|\| channel > 3) return;` added) | `lib/oregon/Oregon_v3.cpp` | 3 | 3 | 1 | ~~30~~ |
| ~~M3~~ | ~~`message[]` file-scope global in `Oregon_v3.cpp` — accidental singleton: two `OregonV3` instances would have shared the same buffer.~~ — ✅ **Resolved 2026-06-16** (moved to the private member `uint8_t message[MESSAGE_SIZE_IN_BYTES]`) | `lib/oregon/Oregon_v3.cpp/.h` | 2 | 2 | 2 | ~~16~~ |

### 3.2 Architecture debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~A1~~ | ~~Duplicated `TinySensor/` source tree (Atmel Studio project)~~ — ✅ **Resolved 2026-10-25** (folder removed, debugging moved to PlatformIO + AVaRICE, see `docs/debugging.md`) | 2 | 3 | 2 | ~~20~~ |
| ~~A2~~ | ~~No common `EnvironmentEncoder` interface: `main.cpp` dispatches Oregon vs Lacrosse with `#if defined(USE_OREGON)/USE_LACROSSE` repeated 4× in the same function. Adding a 3rd protocol will mean a pass through the whole `main()`.~~ — ✅ **Resolved 2026-10-25** (`EnvironmentEncoder` alias + `ApplyMeasurements()`, no virtual) | 3 | 2 | 3 | ~~15~~ |
| ~~A3~~ | ~~The BH1750 block in `main.cpp:155-168` instantiates a `LacrosseWS7000 lightEncoder(&hal)` inline — sensor logic mixed with transmission logic.~~ — ✅ **Resolved 2026-10-25** (`LightSensor` wrapper, light encoder created once) | 3 | 2 | 2 | ~~20~~ |
| ~~A4~~ | ~~Web firmware builder (`web/build.php`): `shell_exec` with no input sanitisation.~~ — ✅ **Resolved 2026-06-18** (decision: remove `web/`; builder never finished, never used in production — `web/build.php` + `web/index.html` removed) | 2 | 3 | 3 | ~~15~~ |
| ~~A5~~ | ~~100 % build-time configuration: each physical sensor = one PIO env + one rebuild.~~ — ⛔ **Closed 2026-11-16** (accepted: deliberate choice for a handful of boards) | 3 | 2 | 4 | ~~10~~ |
| A6 | ATmega328p HAL missing despite the README mention. Branch `feat/328p_hal` (last commit 2023-09) is not merged — parallel code that will drift. | 2 | 1 | 4 | 6 |

### 3.3 Test debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~T1~~ | ~~**The CI builds no AVR target**. The workflow only runs `pio test -e native_debug`. A refactoring can break `S_02`..`SOLAR_TEST` without warning. The "prod" project is never checked.~~ — ✅ **Resolved 2026-04-19** (two-job workflow, AVR matrix) | 5 | 4 | 2 | ~~36~~ |
| ~~T2~~ | ~~No active coverage report. `.coveralls.yml` exists (and is compromised, see C1), but no CI step generates/uploads coverage.~~ — ⛔ **Dropped 2026-10-25** (Coveralls removed from the project; secret gone, checked 2026-11-16) | 2 | 1 | 2 | ~~12~~ |
| T3 | No test for the `BMx280` wrapper (the header exposes `GetTemperature/GetHumidity/GetPressure`, no test checks the bridges). | 3 | 2 | 3 | 15 |
| ~~T4~~ | ~~No test for `SoftSerial` — blocked by C6 (dependency on `_delay_us`).~~ — ✅ **Resolved 2026-10-25** (`test_softSerial` suite, 6 tests) | 2 | 2 | 2 | ~~16~~ |
| ~~T5~~ | ~~No HW-in-the-loop or simavr-based test run in CI, although `[env:SOLAR_TEST]` is configured for simavr.~~ — ⛔ **Closed 2026-11-16** (won't do) | 3 | 3 | 4 | ~~12~~ |
| ~~T6~~ | ~~The Oregon/Lacrosse timing tests check a *string of orders* through `TestHal.Orders`; neither real timings nor interop with RTL_433/RFLink are checked.~~ — ◐ **Closed 2026-11-16, partly covered** (`tinyDecoders` native tests decode the repository encoders' frames with an RFLink-modelled decoder) | 3 | 3 | 4 | ~~12~~ |
| T7 | `emitter/test/test_busses/bussesTest.cpp` only tests an `I2CIsConfigured` boolean after `Init()` — no regression value. | 2 | 1 | 2 | 12 |

### 3.4 Dependency debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~D1~~ | ~~**Docker image `php:7.4-apache` EOL since November 2022.** Used as the base of the web firmware builder.~~ — ✅ **Resolved 2026-06-18** (removed with `web/` — no more PHP Dockerfile in the repo) | 4 | 5 | 2 | ~~36~~ |
| ~~D2~~ | ~~Outdated GitHub Actions: `actions/checkout@v3`, `actions/cache@v3`, `actions/setup-python@v4`. Python 3.9 used — EOL October 2025 (we are in April 2026).~~ — ✅ **Resolved 2026-04-19** (v4/v4/v5, Python 3.12) | 3 | 3 | 1 | ~~30~~ |
| ~~D3~~ | ~~PlatformIO dependencies pulled from personal forks with no commit pinning.~~ — ✅ **Resolved 2026-06-18** (all 5 dependencies pinned to their current HEAD SHA in `platformio.ini`) | 4 | 4 | 3 | ~~24~~ |
| ~~D4~~ | ~~`bh1750` pinned to branch `feat/attiny_support` instead of a SHA.~~ — ✅ **Resolved 2026-06-18** (subsumed by D3 — pinned to `6d06b87`) | 3 | 3 | 2 | ~~24~~ |
| ~~D5~~ | ~~`avrdude.conf` (534 KB) committed in the repo.~~ — ✅ **Resolved 2026-06-18** (`git rm avrdude.conf`; it comes from the PlatformIO toolchain, not from the source) | 2 | 2 | 1 | ~~20~~ |
| ~~D6~~ | ~~`web/build.php` loads jQuery 1.12.4 from a CDN (released 2016, EOL).~~ — ✅ **Resolved 2026-06-18** (removed with `web/`) | 1 | 2 | 2 | ~~12~~ |
| ~~D7~~ | ~~Vendored third-party copies in `TinySensor/third_party/` (BMX, I2C, X10) frozen, divergence risk vs. upstream.~~ — ✅ **Resolved 2026-10-25** (removed with `TinySensor/`) | 2 | 2 | 2 | ~~16~~ |
| ~~D8~~ | ~~README badge points to **Travis CI** (`api.travis-ci.org`) — closed to non-paying OSS since 2020. Broken badge = negative signal for contributors.~~ — ✅ **Resolved 2026-06-16** (GitHub Actions badge in `README.md`) | 1 | 1 | 1 | ~~10~~ |
| ~~D9~~ | ~~No Dependabot/Renovate configuration. Version drift goes unnoticed.~~ — ✅ **Resolved 2026-06-16** (`.github/dependabot.yml`: weekly for `github-actions`, monthly for `docker` + `pip`) | 2 | 3 | 1 | ~~25~~ |

### 3.5 Documentation debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~Doc1~~ | ~~Semantics of the `S_02`..`S_05` environments undocumented.~~ — ✅ **Resolved 2026-06-18** (`docs/environments.md` created: PCB naming convention, `INTERNAL_1v1` calibration guide, table of physical boards and special envs, battery thresholds) | 3 | 2 | 2 | ~~20~~ |
| ~~Doc2~~ | ~~No onboarding guide (required stack, local setup, first build).~~ — ◐ **Closed 2026-11-16, partly covered** (README *Development* section, `CLAUDE.md` commands) | 3 | 2 | 2 | ~~20~~ |
| Doc3 | Undocumented flashing workflow: `scripts/flash_tinySensor` (AVR Dragon ISP + fuses) is explained nowhere (`waitForUpload` removed 2026-11-16). | 2 | 2 | 2 | 16 |
| Doc4 | Oregon and Lacrosse specifications referenced in comments (*"A copy of the document should be provided along the implementation"*) but the PDF docs are not in the repo. External link or embedded file needed. | 2 | 2 | 2 | 16 |
| ~~Doc5~~ | ~~Missing: `CONTRIBUTING.md`, `SECURITY.md`, `CODE_OF_CONDUCT.md`, `CHANGELOG.md`.~~ — ⛔ **Closed 2026-11-16** (won't do: personal project) | 2 | 1 | 2 | ~~12~~ |
| Doc6 | Hardware: no consolidated BOM. Partly addressed: `hardware/tinysensor/WIRING.md` (connectors, jumpers, module wiring), `REWORK_v2.2.md`, `VARIANTE_LM35.md`; the old `doc/` images were removed 2026-11-16. | 2 | 1 | 3 | 9 |
| ~~Doc7~~ | ~~Broken Travis badge in the README (duplicate of D8, handled on the deps side).~~ — ✅ **Resolved 2026-06-16** (badge replaced by GitHub Actions in `README.md`) | 1 | 1 | 1 | ~~10~~ |

### 3.6 Infrastructure debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~I1~~ | ~~**Build artefacts committed** in `TinySensor/Debug/`: `.o`, `.d`, `.elf`, `.hex`, `.lss`, `.map`, `.srec`.~~ — ✅ **Resolved 2026-04-19** (audit revisited: no artefact was actually tracked; `TinySensor/Debug/` now covered by `.gitignore`). | 3 | 2 | 1 | ~~25~~ |
| ~~I2~~ | ~~Author-specific absolute paths in `Makefile` and `scripts/flash_tinySensor`.~~ — ✅ **Resolved 2026-06-18** (`PIO ?= $(HOME)/.platformio/penv/bin/pio`, `TARGET ?= S_03` in `Makefile`; `PIO_HOME` and `PROJECT_DIR` with defaults `${HOME}/.platformio` and `$(dirname $0)/..` in the script) | 3 | 2 | 1 | ~~25~~ |
| ~~I3~~ | ~~KiCAD backup archive committed (`tinySensorv1-2025-05-01_183922.zip`).~~ — ✅ **Resolved 2026-06-18** (audit revisited: the ZIP was not tracked by git — on disk only; `*.zip` already covered by `.gitignore`) | 2 | 2 | 1 | ~~20~~ |
| ~~I4~~ | ~~The CI produces no artefact (downloadable `.hex` firmware).~~ — ✅ **Resolved 2026-06-16** (`release` job added on `v*` tags: downloads every artefact and publishes a GitHub Release with auto-generated notes — completes the partial resolution of 2026-04-19) | 2 | 1 | 2 | ~~12~~ |
| ~~I5~~ | ~~Single-runner, single-python, single-env CI. No matrix to test different tooling versions.~~ — ⛔ **Closed 2026-11-16** (won't do) | 3 | 2 | 2 | ~~20~~ |
| ~~I6~~ | ~~No pre-commit or `.editorconfig`. `clang-format` is installed in the devcontainer but not applied automatically (commit `68cefb8 chore: clang-format` suggests a one-off manual pass).~~ — ✅ **Resolved 2026-10-25** (`.editorconfig`, clang-format 18.1.8 pre-commit, CI `lint` job) | 2 | 1 | 2 | ~~12~~ |
| ~~I7~~ | ~~Minimal `.gitignore` (12 lines): does not exclude `Debug/` (hence I1), nor `*.o`, `*.d`, `*.hex` at the root.~~ — ✅ **Resolved 2026-04-19** (buggy line 1 removed, artefact patterns added, commented structure). | 2 | 2 | 1 | ~~20~~ |
| I8 | No versioning: tags exist (`late_otpim`, `second_pcb_batch`, …) and one release (2026-06), but no `v*` tag, so the CI `release` job never ran. | 2 | 1 | 2 | 12 |

---

## 4. Open items (2026-11-16)

Every other item is resolved or closed (§1bis, §3). The April top 20 is in the git history.

| Rank | ID | Item | Cat. | Score |
|:----:|----|------|:----:|:-----:|
| 1 | Doc3 | `scripts/flash_tinySensor` (Dragon ISP + fuses) undocumented | Doc | 16 |
| 1 | Doc4 | Oregon / Lacrosse specifications not referenced | Doc | 16 |
| 3 | T3 | No test of the `BMx280` wrapper | Test | 15 |
| 4 | T7 | `test_busses` has no regression value | Test | 12 |
| 4 | I8 | No `v*` tag, the `release` job never ran | Infra | 12 |
| 6 | Doc6 | No consolidated hardware BOM | Doc | 9 |
| 7 | A6 | Stale `feat/328p_hal` branch, README mention | Archi | 6 |

---

## 5. Remaining plan

One item at a time, in this order (cheapest first):

1. **Doc3** — describe `scripts/flash_tinySensor` in `docs/debugging.md`: programmer, fuses it writes, `PIO_HOME` / `PROJECT_DIR`.
2. **Doc4** — reference the protocol documents (links in the encoder headers or under `docs/`).
3. **A6** — decide on `feat/328p_hal`: delete the branch and the README line, or revive it.
4. **T7** — drop `test_busses` or give it a real check.
5. **T3** — `BMx280` wrapper test, through the RBD workflow (requirement, test, code).
6. **I8** — decide on `v*` tags (the `release` job is ready) or close.
7. **Doc6** — hardware BOM from the KiCad schematic.

---

## 6. What works well (to preserve)

For balance, here are the project's strengths not to break during refactorings:

**The HAL is well designed** (`Hal.h` → `Attiny84aHal` / `TestHal_`): clear abstraction, template methods (`ComputeVccMv`, `ConvertAnalogValueToMv`) in the base class, substitutable for tests. It is the pattern to generalise.

**The protocol encoders are tested and deterministic**: `OregonV3` and `LacrosseWS7000` are checked through a trace of HAL orders (`Orders.push_back('H')`), which lets the bit sequence be asserted without hardware. Elegant approach.

**The inline comments** in the encoders document the frame format (nibble layout, battery status bits, checksum) — precious in a field where specs are rare.

**The devcontainer `Dockerfile`** is complete and self-sufficient to onboard a new developer — the technical entry point is clear.

**The commits** follow a convention (`feat:`, `fix:`, `chore:`, `hardware:`, `optim:`) — a base for automatic semver once releases are tooled (phase 4).

---

## 7. Appendices

### 7.1 Metrics

| Metric | April 2026 | 2026-11-16 |
|---|---|---|
| Lines of code (`lib/` + emitter `src/` + `include/`) | 1,543 | 2,126 |
| Lines of emitter tests | 1,835 | 3,045 |
| Emitter test suites | 8 | 13 (+ compile-fail and variant scripts) |
| `.cpp`/`.h` files (`lib/` + emitter) | 19 | 26 |
| Remaining TODO/FIXME/HACK | 1 | 0 |
| PlatformIO environments (emitter) | 8 (1 native + 7 AVR) | 13 (1 base, 5 native, 7 AVR) |
| `tinyDecoders` receiver (code / tests) | — | 1,621 / 3,281 lines |
| External dependencies | 5 (unpinned forks) | 5 (forks pinned to a SHA) |
| CI | 1 workflow, native tests only | 1 workflow: lint, native tests, 7 AVR builds, receiver builds, release on `v*` |
| Exposed secrets | 1 | 0 |

### 7.2 Items not retained (out of scope)

The following were seen but do not justify immediate action:

- *KiCAD quality* (hardware): beyond the reach of a code review. To hand to a hardware reviewer.
- *Language choice* (C++ on ATtiny): suited to the memory/resource context. No debt.
- *`float` format for the temperature* (`Oregon_v3.cpp`): acceptable given the available room; to reconsider only if flash size becomes critical.

---

*End of report. Author: Claude. Date: 19 April 2026, last update 16 November 2026. The initial audit reflects commit `aa53a83`; §1bis, §4, §5 and §7.1 reflect the 2026-11-16 state. Scores are indicative and must be recalibrated to team capacity and product priorities.*
