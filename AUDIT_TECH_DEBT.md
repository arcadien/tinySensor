# Technical debt audit — TinySensor

**Project**: [arcadien/tinySensor](https://github.com/arcadien/tinySensor) — low-power AVR firmware for wireless environment sensors (ATtiny84a, 433 MHz, Oregon v3 / Lacrosse WS7000 / X10 protocols).

**Auditor**: Claude (opus 4.7 model) — static review

**Date**: 19 April 2026 — updated 19 June 2026 (see §1bis)

**Scope**: whole repository at commit `aa53a83` (branch `master`) — firmware code (`src/`, `lib/`, `include/`), tests (`test/`), legacy Atmel Studio project (`TinySensor/`), tooling (`scripts/`, `shared/`), web builder (`web/`), hardware (`hardware/`), CI (`.github/workflows/`), Docker/devcontainer.

---

## 1. Executive summary

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
| C13 | "Leaky" HAL: methods named `RadioGoHigh/Low`, `SerialGoHigh/Low` — semantic IO driving, not a real radio bus abstraction. A port to an MCU without similarly configurable GPIO will require touching the HAL. | `lib/hal/Hal.h` | 3 | 2 | 4 | **10** |
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
| A5 | 100 % build-time configuration: each physical sensor = one PIO env + one rebuild. No runtime config, so no field update without manual reflashing. Acceptable architecture for a hobby / very small production run; limiting if it has to scale. | 3 | 2 | 4 | 10 |
| A6 | ATmega328p HAL missing despite the README mention. Branch `feat/328p_hal` exists but is not merged — parallel code that will drift. | 2 | 1 | 4 | 6 |

### 3.3 Test debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~T1~~ | ~~**The CI builds no AVR target**. The workflow only runs `pio test -e native_debug`. A refactoring can break `S_02`..`SOLAR_TEST` without warning. The "prod" project is never checked.~~ — ✅ **Resolved 2026-04-19** (two-job workflow, AVR matrix) | 5 | 4 | 2 | ~~36~~ |
| ~~T2~~ | ~~No active coverage report. `.coveralls.yml` exists (and is compromised, see C1), but no CI step generates/uploads coverage.~~ — ⛔ **Dropped 2026-10-25** (Coveralls removed from the project) | 2 | 1 | 2 | ~~12~~ |
| T3 | No test for the `BMx280` wrapper (the header exposes `GetTemperature/GetHumidity/GetPressure`, no test checks the bridges). | 3 | 2 | 3 | 15 |
| ~~T4~~ | ~~No test for `SoftSerial` — blocked by C6 (dependency on `_delay_us`).~~ — ✅ **Resolved 2026-10-25** (`test_softSerial` suite, 6 tests) | 2 | 2 | 2 | ~~16~~ |
| T5 | No HW-in-the-loop or simavr-based test run in CI, although `[env:SOLAR_TEST]` is configured for simavr. Infrastructure ready but unused. | 3 | 3 | 4 | 12 |
| T6 | The Oregon/Lacrosse timing tests check a *string of orders* (`"LDH PLD"` = 1, `"HDL PHD"` = 0) through `TestHal.Orders`. Elegant, but it checks neither the real timings nor the interop with RTL_433/RFLink that the README cites as a reference. | 3 | 3 | 4 | 12 |
| T7 | `test_busses/bussesTest.cpp` only tests an `I2CIsConfigured` boolean after `Init()` — no regression value. | 2 | 1 | 2 | 12 |

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
| Doc2 | No onboarding guide (required stack, local setup, first build). The README targets users/hardware, not contributors. | 3 | 2 | 2 | **20** |
| Doc3 | Undocumented flashing workflow: `scripts/flash_tinySensor` + `waitForUpload` assume a specific NAS layout (`/var/services/homes/aurelien/…`). | 2 | 2 | 2 | 16 |
| Doc4 | Oregon and Lacrosse specifications referenced in comments (*"A copy of the document should be provided along the implementation"*) but the PDF docs are not in the repo. External link or embedded file needed. | 2 | 2 | 2 | 16 |
| Doc5 | Missing: `CONTRIBUTING.md`, `SECURITY.md` (especially relevant given C1), `CODE_OF_CONDUCT.md`, `CHANGELOG.md`. | 2 | 1 | 2 | 12 |
| Doc6 | Hardware: no consolidated BOM, no assembly guide in `doc/` (only 3 images: `boardv2.png`, `boardv2_copper.png`, `schematic.png`). Partly addressed 2026-11-14: `hardware/tinysensor/WIRING.md` (connectors, jumpers, module wiring). | 2 | 1 | 3 | 9 |
| ~~Doc7~~ | ~~Broken Travis badge in the README (duplicate of D8, handled on the deps side).~~ — ✅ **Resolved 2026-06-16** (badge replaced by GitHub Actions in `README.md`) | 1 | 1 | 1 | ~~10~~ |

### 3.6 Infrastructure debt

| ID | Item | Impact | Risk | Effort | Priority |
|----|------|:------:|:------:|:------:|:--------:|
| ~~I1~~ | ~~**Build artefacts committed** in `TinySensor/Debug/`: `.o`, `.d`, `.elf`, `.hex`, `.lss`, `.map`, `.srec`.~~ — ✅ **Resolved 2026-04-19** (audit revisited: no artefact was actually tracked; `TinySensor/Debug/` now covered by `.gitignore`). | 3 | 2 | 1 | ~~25~~ |
| ~~I2~~ | ~~Author-specific absolute paths in `Makefile` and `scripts/flash_tinySensor`.~~ — ✅ **Resolved 2026-06-18** (`PIO ?= $(HOME)/.platformio/penv/bin/pio`, `TARGET ?= S_03` in `Makefile`; `PIO_HOME` and `PROJECT_DIR` with defaults `${HOME}/.platformio` and `$(dirname $0)/..` in the script) | 3 | 2 | 1 | ~~25~~ |
| ~~I3~~ | ~~KiCAD backup archive committed (`tinySensorv1-2025-05-01_183922.zip`).~~ — ✅ **Resolved 2026-06-18** (audit revisited: the ZIP was not tracked by git — on disk only; `*.zip` already covered by `.gitignore`) | 2 | 2 | 1 | ~~20~~ |
| ~~I4~~ | ~~The CI produces no artefact (downloadable `.hex` firmware).~~ — ✅ **Resolved 2026-06-16** (`release` job added on `v*` tags: downloads every artefact and publishes a GitHub Release with auto-generated notes — completes the partial resolution of 2026-04-19) | 2 | 1 | 2 | ~~12~~ |
| I5 | Single-runner, single-python, single-env CI. No matrix to test different tooling versions. | 3 | 2 | 2 | **20** |
| ~~I6~~ | ~~No pre-commit or `.editorconfig`. `clang-format` is installed in the devcontainer but not applied automatically (commit `68cefb8 chore: clang-format` suggests a one-off manual pass).~~ — ✅ **Resolved 2026-10-25** (`.editorconfig`, clang-format 18.1.8 pre-commit, CI `lint` job) | 2 | 1 | 2 | ~~12~~ |
| ~~I7~~ | ~~Minimal `.gitignore` (12 lines): does not exclude `Debug/` (hence I1), nor `*.o`, `*.d`, `*.hex` at the root.~~ — ✅ **Resolved 2026-04-19** (buggy line 1 removed, artefact patterns added, commented structure). | 2 | 2 | 1 | ~~20~~ |
| I8 | No release / git tag visible over the last 30 commits. No versioning. | 2 | 1 | 2 | 12 |

---

## 4. Priority ranking (top 20)

> Struck-out items were handled after the initial publication (see §1bis). They stay listed to trace the effort spent.

| Rank | ID | Item | Cat. | Score |
|:----:|----|------|:----:|:-----:|
| — | ~~T1~~ | ~~CI builds no AVR target~~ ✅ | Test | ~~36~~ |
| — | ~~D1~~ | ~~EOL PHP 7.4 Docker image~~ ✅ | Dep | ~~36~~ |
| — | ~~C1~~ | ~~Coveralls token in clear in the repo~~ ✅ | Code | ~~35~~ |
| — | ~~A1~~ | ~~Duplicated `TinySensor/` source tree~~ → requalified, accepted risk (score 20, see §3.2) | Archi | ~~32~~ |
| — | ~~D2~~ | ~~Outdated GitHub Actions + EOL Python 3.9~~ ✅ | Dep | ~~30~~ |
| — | ~~C2~~ | ~~`OREGON_DELAY_US` macro refers to a non-existent method~~ ✅ | Code | ~~30~~ |
| — | ~~M1~~ | ~~Inverted PRR bit in `UseLessPowerAsPossible()`~~ ✅ | Code | ~~30~~ |
| — | ~~M2~~ | ~~`SetChannel()` with no bounds check~~ ✅ | Code | ~~30~~ |
| — | ~~D9~~ | ~~No Dependabot/Renovate~~ ✅ | Dep | ~~25~~ |
| — | ~~I1~~ | ~~Build artefacts committed (`Debug/`)~~ ✅ | Infra | ~~25~~ |
| — | ~~I2~~ | ~~Author absolute paths in Makefile/scripts~~ ✅ | Infra | ~~25~~ |
| — | ~~D3~~ | ~~Deps point to unpinned personal forks~~ ✅ | Dep | ~~24~~ |
| — | ~~D4~~ | ~~`bh1750` pinned to a branch instead of a tag/SHA~~ ✅ | Dep | ~~24~~ |
| — | ~~C3~~ | ~~Orphan `Init()` in `Attiny84aHal.cpp`~~ ✅ | Code | ~~20~~ |
| — | ~~C4~~ | ~~Dead code `if (int(count) != count)` in `Sum`~~ ✅ | Code | ~~20~~ |
| 8 | C5 | `main.cpp` accesses registers without going through the HAL | Code | 20 |
| 8 | C6 | `SoftSerial` uses `_delay_us` directly (untestable) | Code | 20 |
| 8 | A3 | BH1750 logic mixed into `main.cpp` | Archi | 20 |
| — | ~~D5~~ | ~~`avrdude.conf` (534 KB) committed~~ ✅ | Dep | ~~20~~ |
| — | ~~Doc1~~ | ~~`S_0x` envs undocumented (tribal knowledge)~~ ✅ | Doc | ~~20~~ |
| 8 | Doc2 | No onboarding guide | Doc | 20 |
| — | ~~I3~~ | ~~KiCAD backup ZIP committed~~ → not tracked by git ✅ | Infra | ~~20~~ |
| 8 | I5 | No CI matrix | Infra | 20 |
| — | ~~I7~~ | ~~Minimal `.gitignore`~~ ✅ | Infra | ~~20~~ |
| — | ~~M3~~ | ~~`message[]` global shared between instances~~ ✅ | Code | ~~16~~ |
| — | ~~C7~~ | ~~Massive `platformio.ini` duplication~~ ✅ | Code | ~~18~~ |
| — | ~~C10~~ | ~~`USE_BMP280=1` duplicated in `env:S_03`~~ ✅ | Code | ~~15~~ |
| — | ~~I4~~ | ~~No release automation on tag~~ ✅ | Infra | ~~12~~ |
| — | ~~D8~~ | ~~Dead Travis badge~~ ✅ | Dep | ~~10~~ |

---

## 5. Phased remediation plan

Plan sized for 1 developer at ~20 % of their time. Each phase interleaves with feature work; none blocks the others.

### Phase 0 — Immediate security (day 0–1, < 2 h)

A single, non-negotiable item: **revoke the Coveralls token** (`C1`). Revoke it on coveralls.io, remove the token from the file, move it to a GitHub Actions secret `COVERALLS_REPO_TOKEN`, and adapt the workflow. Even though the repo is public and the exposure already happened, rotation is mandatory.

### Phase 1 — CI & tooling stabilisation (week 1–2, ~1 day) — **partly done 2026-04-19**

Goal: no longer be able to break the production firmware without the CI showing it, and bring the tooling up to date.

Actions, by value:

1. ~~**T1 — Add the AVR build to the CI**. Extend `.github/workflows/main.yml` with a matrix running `pio run -e S_02 -e S_03 -e S_04 -e S_05 -e robot -e devmodule` alongside `pio test -e native_debug`. Cost: 30 min.~~ ✅ **Done 2026-04-19** (matrix over 7 envs, `fail-fast: false`).
2. ~~**D2 — Update the Actions**: `checkout@v4`, `cache@v4`, `setup-python@v5`, Python `3.12`. Cost: 15 min.~~ ✅ **Done 2026-04-19**.
3. ~~**I1, I7 — Clean the versioned artefacts**. Add to `.gitignore`: `Debug/`, `Release/`, `*.o`, `*.d`, `*.elf`, `*.hex` (root only), `*.lss`, `*.map`, `*.srec`, `TinySensor/Debug/`. Then `git rm -r --cached TinySensor/Debug/` and commit.~~ ✅ **Done 2026-04-19** (see §1bis; `git rm --cached` turned out a no-op, no artefact was actually tracked).
4. **I2 — Parameterise the scripts**. Replace the absolute paths of the `Makefile` and `scripts/flash_tinySensor` with environment variables (`PIO_HOME`, `PROJECT_DIR`). Document in the README. Cost: 1 h.
5. **D9 — Enable Dependabot** (`.github/dependabot.yml`) for `github-actions`, `docker`, `pip` (the Python used in the scripts). Cost: 10 min.
6. **D8 + Doc7 — Remove the Travis badge**, add the GitHub Actions badge. Cost: 5 min.
7. **I4 (remaining part) — Release automation on tag**: an `on: push: tags: ['v*']` workflow that replays the matrix and publishes the `.hex` / `.elf` as a GitHub Release. Cost: 1 h. *Moved here from phase 4 since the artefact base is now in place.*

End-of-phase deliverable: green CI on **every AVR target + native** (reached), no change can silently break a production firmware any more (reached), downloadable artefacts per build (reached), and automatic tagged releases (to do).

### Phase 2 — Source tree consolidation (week 3–5, ~3 days in total)

Goal: a single source of truth for each file, reproducible dependencies.

1. **A1 — Decision on `TinySensor/` (legacy Atmel Studio)**. Two options:
    - (a) Remove the folder entirely. This is the recommendation: the PlatformIO project covers every use, Atmel Studio is no longer maintained (Microchip Studio 7.0 is also at end of support). Cost: 10 min + an audit of what would be lost.
    - (b) Keep it as a separate branch or submodule, outside `master`.
    Action: open a "remove legacy Atmel Studio tree" PR with a one-week confirmation window.
2. **D3, D4 — Pin the PlatformIO dependencies**. Move from `url.git#branch` to `url.git#<SHA>` (or better, create `v1.0.0` tags on each fork and use `#v1.0.0`). Cost: 1 h.
3. **D5 — Take `avrdude.conf` out** of the repo. The config comes from PlatformIO's `tool-avrdude` package; document it in the README and add it to `.gitignore`. Cost: 15 min.
4. **I3 — Delete the KiCAD backup ZIP**. The git history is enough. Cost: 5 min.
5. **D1 — Migrate the Dockerfile** from `php:7.4-apache` to `php:8.3-apache` (or newer). Test the build. Cost: 2 h + test.

Deliverable: a single source tree, reproducible deps, up-to-date Docker image.

### Phase 3 — Firmware code quality (week 6–9, ~3 days in total)

Goal: remove the identified code debt, strengthen the tests.

Quick wins (1 h each):

1. **C2** — Fix the `OREGON_DELAY_US` macro in `TestHal.h`.
2. **C3** — Remove the orphan free function `Init()`.
3. **C4** — Remove the dead `if` in `OregonV3::Sum`.
4. **C10** — Deduplicate `USE_BMP280=1` in `env:S_03`.
5. **C11** — Remove the dead `if (pressure < 200)`.
6. **C15** — Add `override` on every virtual method of `TestHal`.

Medium refactorings (1 day each):

7. **C5 + A3** — Wrap the DS18B20 and BH1750 accesses behind sensor classes (`Ds18b20Sensor`, `Bh1750Sensor`) using the HAL. Would align with the `BMx280` pattern.
8. **C6 + T4** — Refactor `SoftSerial` so that every delay goes through the HAL (`hal->DelayBitPeriod()` or similar). Add a native `SoftSerial` test checking the `SerialGoHigh/Low` sequence for a known character.
9. **C7 — Deduplicate `platformio.ini`**: use an `[env:base_avr]` inherited by `[env:S_02]`, etc. (native PlatformIO feature through `extends = env:base_avr`). Cost: 2 h + test.
10. **C12** — Centralise `BITREAD` in `lib/utils/include/bitops.h` and remove the redefinitions.
11. **T3** — Add a unit test for `BMx280` in non-AVR mode (checks that the stubs return 0 without crashing).
12. **C8** — Document the magic numbers (comment with datasheet reference + link).

### Phase 4 — Architecture & documentation (Q2/Q3, as it comes)

1. **A2** — Introduce an `IEnvironmentEncoder` interface shared by `OregonV3` and `LacrosseWS7000`, injected into `main()`. Simplifies adding future protocols.
2. **A4** — Decide on `web/`: (a) finish it (form + sanitisation) or (b) remove it and document a local builder. (b) is recommended given its state.
3. **Doc1, Doc2** — Write `docs/environments.md` (which env for which physical board) and `docs/CONTRIBUTING.md` (stack, first build, HAL/encoder architecture).
4. **Doc5** — Create `SECURITY.md` (reporting process) and `CHANGELOG.md` (kept up to date over the next releases).
5. **I5 — Tooling matrix**: extend the existing matrix to several Pythons (`3.11`, `3.12`, `3.13`) and/or several PlatformIO Core versions. (The release automation part of I4 moved up to Phase 1.)
6. **T5, T6 — Raise the test bar**: integrate `simavr` in the CI for at least one env (`SOLAR_TEST` is ready), and add an integration test that feeds the Oregon frame to the RTL_433 decoder (binary installable in CI) to validate real interop.
7. **A5 — Assessment**: runtime config through EEPROM? Relevant only when aiming at more than 5 units in production. Do not do it before the need arises.

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

| Metric | Value |
|---|---|
| Lines of code (lib + src + include) | 1,543 |
| Lines of tests | 1,835 |
| Test/code ratio | 1.19 |
| `.cpp`/`.h` files audited (excluding legacy) | 19 |
| Remaining TODO/FIXME/HACK | 1 |
| PlatformIO environments | 8 (1 native + 7 AVR) |
| External dependencies | 5 (all personal forks) |
| CI workflows | 1 (2 jobs: native + 7-env AVR matrix, updated 2026-04-19) |
| Exposed secrets | 0 (C1 revoked) |

### 7.2 Items not retained (out of scope)

The following were seen but do not justify immediate action:

- *KiCAD quality* (hardware): beyond the reach of a code review. To hand to a hardware reviewer.
- *Language choice* (C++ on ATtiny): suited to the memory/resource context. No debt.
- *`float` format for the temperature* (`Oregon_v3.cpp`): acceptable given the available room; to reconsider only if flash size becomes critical.

### 7.3 Useful commands for the remediation

```bash
# Phase 1 — check that the CI covers every AVR target
pio run -e S_02 -e S_03 -e S_04 -e S_05 -e robot -e devmodule -e SOLAR_TEST

# Phase 2 — purge the legacy tree (after decision)
git rm -r TinySensor/
git rm avrdude.conf
git rm hardware/tinysensor/tinySensorv1-backups/

# Phase 3 — deduplicate platformio.ini with extends
# [env:base_avr]
#   platform = atmelavr
#   board = attiny84
#   framework = arduino
#   board_build.f_cpu = 1000000L
#   board_build.mcu = attiny84a
#   build_type = release
# [env:S_02]
#   extends = env:base_avr
#   build_flags = -DUSE_OREGON -DOREGON_RCODE=0x20 …
```

---

*End of report. Author: Claude. Date: 19 April 2026. This report reflects the repository state at commit `aa53a83`. Scores are indicative and must be recalibrated to team capacity and product priorities.*
