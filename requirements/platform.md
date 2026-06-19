# Platform Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### PLAT-POWER-001
**Title:** Hibernate loops SLEEP_MODE_PWR_DOWN for ceil(s/8) WDT periods
**Status:** validated
**Dependencies:** TECH-HAL-004
**derives_from:** [TECH-HAL-004](technical.md#tech-hal-004)
**Description:** `Attiny84aHal::Hibernate(s)` delegates to `sleep(s)`, which computes `n = s >> 3` (minimum 1) and enters `SLEEP_MODE_PWR_DOWN` via the 8-second hardware watchdog timer exactly `n` times. A request of fewer than 8 seconds still produces one sleep period.

---

### PLAT-POWER-002
**Title:** Sensor power rail PA2 driven high before first read and low after last transmission
**Status:** validated
**Dependencies:** FUNC-SENSOR-001
**derives_from:** [FUNC-SENSOR-001](functional.md#func-sensor-001)
**Description:** In each iteration of the measurement loop, `hal.PowerOnSensors()` drives PA2 (`SENSOR_VCC`) high before any sensor read; `hal.PowerOffSensors()` drives PA2 low immediately after the last transmission and before `Hibernate()`. No sensor read or transmission occurs outside this power window.

---

### PLAT-POWER-003
**Title:** Hibernate disables USI/Timer0/Timer1/ADC and reconfigures all GPIO as inputs with pull-ups; restores all state on wake
**Status:** validated
**Dependencies:** PLAT-POWER-001, TECH-HAL-004
**derives_from:** [TECH-HAL-004](technical.md#tech-hal-004)
**Description:** Before entering the first sleep period, `sleep()` sets PRR bits for PRUSI, PRTIM0, PRTIM1, and PRADC; sets DDRA and DDRB to 0; and applies pull-ups to unused pins. After the last sleep period, all five registers (PRR, DDRA, DDRB, PORTA, PORTB) are restored to their pre-sleep values.

---

### PLAT-POWER-004
**Title:** Each Send() call is flanked by LedOn() before and LedOff() + Delay30ms() after via TRANSMIT_WITH_LED
**Status:** validated
**Dependencies:** TECH-HAL-005
**derives_from:** [TECH-HAL-005](technical.md#tech-hal-005)
**Description:** The `TRANSMIT_WITH_LED(code)` macro defined in `src/main.cpp` expands to: call `hal.LedOn()`, execute `code` (the `Send()` call), call `hal.LedOff()`, then call `hal.Delay30ms()`. Every protocol `Send()` invocation in the measurement loop must be wrapped in this macro.

---

### PLAT-POWER-005
**Title:** Target MCU is ATtiny84a at 1 MHz; all Attiny84aHal delay methods are calibrated to F_CPU = 1 000 000
**Status:** validated
**Dependencies:** TECH-HAL-001
**derives_from:** [TECH-HAL-001](technical.md#tech-hal-001)
**Description:** All physical PlatformIO environments inherit `[env:avr_base]`, which sets `board_build.f_cpu = 1000000L` and `board_build.mcu = attiny84a`. All `Attiny84aHal` delay methods (`Delay512Us`, `Delay1024Us`, `Delay400Us`, `Delay30ms`) are calibrated exclusively for this 1 MHz clock frequency.
