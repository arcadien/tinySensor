# Bench test PLAT-POWER-005 — radio timings at 1 MHz

Requirement: [PLAT-POWER-005](../../requirements/platform.md#plat-power-005). Manual test, run at bring-up step **B8** ([bring-up.md](bring-up.md)); add a line to *Results* every time it is run.

## Equipment
- The board under test, the receiver, an RFLink gateway if available.

## Setup
1. Board fuses at the 1 MHz default (internal 8 MHz RC, CKDIV8).

## Steps
1. Let the board send for 10 minutes; count decoded frames on the receiver and, if available, on the RFLink gateway.

## Pass criteria
- Every protocol the board sends (Oregon or Lacrosse, X10, luminosity) is decoded by the receiver and by RFLink.

## Results
| Date | Commit / build | Board | Frames decoded (receiver / RFLink) | Pass | By | Notes |
|---|---|---|---|---|---|---|
