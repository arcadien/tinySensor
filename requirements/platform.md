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
**Title:** Hibernate disables USI/Timer0/Timer1/ADC, holds the sensor rail and radio data low, leaves sensor-rail pull-up pins unpulled; restores all state on wake
**Status:** validated
**Dependencies:** PLAT-POWER-001, PLAT-POWER-002, TECH-HAL-004
**derives_from:** [TECH-HAL-004](technical.md#tech-hal-004)
**Description:** Before entering the first sleep period, `sleep()` sets PRR bits for PRUSI, PRTIM0, PRTIM1, and PRADC. It drives PA2 (`SENSOR_VCC`) and PB0 (radio data, the radio being always powered) as outputs held low, so the sensor rail stays off and the transmitter cannot key itself from a floating input. PA3 (1-Wire), PA4 (SCL), PA5 (MISO) and PA6 (SDA) are inputs without internal pull-up: their external pull-ups go to `SENSOR_VCC`, or (PA5) they may reach the sensor SDO in SPI wiring, so an internal pull-up would back-power the unpowered sensors. A floating input costs nothing in power-down, where the input buffers are clamped. Every other pin is an input, with pull-ups on the unused ones. After the last sleep period, all five registers (PRR, DDRA, DDRB, PORTA, PORTB) are restored to their pre-sleep values.

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

---

### PLAT-POWER-006
**Title:** With USE_SWITCHED_5V, PB2 is driven low only inside the sensor power window to switch the off-board 5 V module
**Status:** validated
**Dependencies:** PLAT-POWER-002, PLAT-POWER-003, TECH-HAL-004
**derives_from:** [FUNC-ANALOG-001](functional.md#func-analog-001)
**Description:** When `USE_SWITCHED_5V` is defined (AVR build), PB2 (J8) switches the off-board 5 V module through a PNP (`hardware/tinysensor/VARIANTE_LM35.md`). `Attiny84aHal()` clears `PORTB.2` (no pull-up) and it stays 0. `PowerOnSensors()` makes PB2 an output (`DDRB.2 = 1`), hence driven low; `PowerOffSensors()` makes it an input again (`DDRB.2 = 0`, high impedance). PB2 is never driven high. During hibernate, PB2 is an input without pull-up. Without the flag, PB2 behaviour is unchanged. Verified by inspection and on the bench (AVR-only, no native test).

---

### PLAT-DECODE-001
**Title:** Attiny84aReceiverHal captures radio pulses on ICP1 (PA7) with Timer1 at 1 µs and counts seconds from its overflow
**Status:** validated
**Dependencies:** TECH-DECODE-002, CONF-DECODE-001
**derives_from:** [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** In `tinyDecoders/lib/ReceiverHal/Attiny84aReceiverHal.{h,cpp}` (AVR only), `Init()` makes PA7 an input without pull-up, runs Timer1 in normal mode with prescaler 8 (1 µs ticks at 8 MHz), noise canceller on, capture edge opposite to the current PA7 level, capture and overflow interrupts on. `TIM1_CAPT` pushes the ended pulse (low if the captured edge rose), duration = ticks since the previous capture, 65535 if a full timer period has elapsed, toggles the edge, into a 16-pulse ring buffer; when full the pulse is dropped. `TIM1_OVF` adds 65536 µs to an accumulator and increments seconds per 1 000 000 µs. `PopPulse` and `NowSeconds` read with interrupts briefly disabled. Verified by inspection and on the bench (AVR only).

---

### PLAT-DECODE-002
**Title:** Attiny84aReceiverHal drives an SSD1306 128x32 OLED at I2C 0x3C, one 8x16 character per DrawChar, without framebuffer
**Status:** validated
**Dependencies:** TECH-DECODE-002, UI-DECODE-002, CONF-DECODE-001
**derives_from:** [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** `Attiny84aReceiverHal` talks to the SSD1306 at 0x3C through `TinyI2C` (USI: SDA PA6, SCL PA4). `Init()` sends: `AE D5 80 A8 1F D3 00 40 8D 14 20 02 A1 C8 DA 02 81 8F D9 F1 DB 40 A4 A6`, writes zeros to the 4 pages × 128 columns, then `AF`. `DrawChar(row, col, c)` reads the glyph with `ReadGlyph(c)` and, for page `2*row` then `2*row + 1`, sends commands `B0+page`, `col*8 & 0x0F`, `0x10 | (col*8 >> 4)`, then one data transfer (`0x40` and 8 glyph bytes). No display buffer is kept in RAM. Verified by inspection and on the bench (AVR only).
