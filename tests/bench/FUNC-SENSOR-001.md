<!-- @req FUNC-SENSOR-001 -->
# Bench test FUNC-SENSOR-001 — BME280 readings over several cycles

Requirement: [FUNC-SENSOR-001](../../requirements/functional.md#func-sensor-001). Manual test, run at bring-up step **B5** ([bring-up.md](bring-up.md)); add a line to *Results* every time it is run.

## Equipment
- The board under test with its BME280, the receiver ([PLAT-DECODE-003](PLAT-DECODE-003.md)), a reference thermometer / hygrometer and the local sea-level pressure (weather service).

## Setup
1. Board and reference side by side, indoors, 30 min to settle.

## Steps
1. Read 10 consecutive cycles on the receiver (RFLink lines).
2. Note the reference values at the same time.

## Pass criteria
- Every cycle sends a frame; values stable from cycle to cycle (temperature ± 0.3 °C, humidity ± 2 %).
- Within the BME280 accuracy of the reference: ± 1 °C, ± 3 %, pressure ± 2 hPa (pressure is station pressure, not sea level: compare with altitude in mind).

## Results
| Date | Commit / build | Board | T / H / P (board) | T / H / P (reference) | Pass | By | Notes |
|---|---|---|---|---|---|---|---|
