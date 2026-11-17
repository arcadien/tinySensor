# MR Review — req/functional-rewording — 2026-11-17

Reviewed commit: `3cd968485189df3c0e10cfcd96a2589bb03d4bd2` (PR #75 head at review time; history rewritten by the fix, same tree as `23a0ae6f733b9a64ba58e1ce039be1f5bd2d4c34`). Diff: `requirements/functional.md`, `requirements/technical.md` only. No code or test file changed.

## Summary

| # | Finding | Status |
|---|---------|--------|
| 1 | a4a3e1d adds TECH-SENSOR-002 without naming it in the commit header | resolved |
| 2 | cacf5db rewords three domains (SENSOR, BATTERY, ANALOG) in one commit | resolved |

## Findings

### Finding 1
**Type:** Commit prefix consistency (R3), multi-requirement commit
**Commit:** a4a3e1d req(TECH-DECODE-003): receiver classes and memory layout, moved out of FUNC-DECODE
**Issue:** The commit also creates TECH-SENSOR-002 (named only in the body). Tools that trace requirements by the commit header (pre-push hook, rbd-audit) don't see where TECH-SENSOR-002 was created. Splitting is feasible: one `req(TECH-SENSOR-002)` commit, one `req(TECH-DECODE-003)` commit.
**Status:** resolved

### Finding 2
**Type:** Commit prefix consistency (R3), multi-requirement commit
**Commit:** cacf5db req(FUNC-SENSOR-001..003, FUNC-BATTERY-001..003, FUNC-ANALOG-001): state measurements and battery behaviour, not the code
**Issue:** Three requirement domains in one commit. Splitting is feasible (one commit per domain), but it is low value: these are wording-only changes, and every rewritten ID appears in the header.
**Status:** resolved

## Resolution

Branch history rewritten and force-pushed (2026-11-17): TECH-SENSOR-002 and TECH-DECODE-003 are now created in separate commits (finding 1); FUNC-SENSOR, FUNC-BATTERY and FUNC-ANALOG are reworded in one commit each (finding 2). The final tree is identical to the reviewed one.

## Checks without findings

- **R1 coverage:** every referenced ID (TECH-DECODE-003, FUNC-OREGON-001..015, FUNC-LACROSSE-001..009, FUNC-X10-001..007, FUNC-SENSOR-001..003, FUNC-BATTERY-001..003, FUNC-ANALOG-001, FUNC-DECODE-001..010) exists with status `validated`. Note: range notation `001..015` is read by the pre-push hook as the first ID only.
- **R2 uncovered behaviour:** none. No code changed. The two new TECH requirements describe code that already exists on master (API names checked against `tinyDecoders/lib/*/*.h` and `emitter/include/BMx280.h`).
- **R4 test presence:** no `feat/tech/perf/ui/conf` commit on the branch, so nothing to check.

## Notes (not findings)

- TECH-DECODE-003 and TECH-SENSOR-002 have no `// @req` tagged test. They are verified indirectly, by the FUNC-DECODE native tests and the AVR builds. A later `/rbd-audit` traceability pass will likely list them as untested.
- FUNC-DECODE requirements now depend on TECH-DECODE-003; no circular dependency (TECH-DECODE-003 depends only on TECH-DECODE-002).
