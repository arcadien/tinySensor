# Technical Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->
<!-- Technical requirements are design constraints with a reason (testability, memory, hardware isolation). Class, helper and register details belong in docs/architecture.md. -->

### TECH-HAL-001
**Title:** Protocol and sensor logic reach the hardware only through the injected `Hal` interface, so it runs unchanged in native builds

**Status:** validated

**Dependencies:** —

**Verification:** inspection — outside the AVR implementation of `Hal`, the only AVR registers in `emitter/` and `lib/` are the addresses passed to the ds18b20 driver

**Description:** Every protocol encoder, and every sensor wrapper that times or drives I/O itself, gets the hardware through a `Hal*` given at construction (`lib/hal/Hal.h`, the boundary between the firmware logic and the MCU) and touches no AVR register. So the same code builds and runs on the host against a test double. Only the AVR implementation of `Hal` accesses registers, with one exception: register addresses passed as arguments to a third-party bus driver. A wrapper whose I/O is entirely done by a third-party driver takes no `Hal*`.

---

### TECH-HAL-002
**Title:** TestHal_ records each HAL call as a single-character token in the Orders vector

**Status:** deprecated

**Dependencies:** [TECH-HAL-001](#tech-hal-001)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md). The test double's token alphabet is test infrastructure; its tests are tagged with the behaviour they check.

---

### TECH-HAL-003
**Title:** ComputeVccMv and ConvertAnalogValueToMv are non-virtual concrete methods on the Hal base class

**Status:** deprecated

**Dependencies:** —

**Superseded by:** [FUNC-BATTERY-001](functional.md#func-battery-001)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the VCC and pin voltage formulas are [FUNC-BATTERY-001](functional.md#func-battery-001).

---

### TECH-HAL-004
**Title:** Attiny84aHal constructor disables Timer1, ADC, and analog comparator; no pull-up on PORTA, PA5 digital input buffer off, pull-ups on PB2 and PB3

**Status:** deprecated

**Dependencies:** [PLAT-POWER-007](platform.md#plat-power-007)

**Superseded by:** [PLAT-POWER-008](platform.md#plat-power-008)

**Description:** Deprecated 2026-11-20: the awake pin states and their reason are now [PLAT-POWER-008](platform.md#plat-power-008); the constructor details are in [docs/architecture.md](../docs/architecture.md).

---

### TECH-HAL-005
**Title:** LedOn and LedOff toggle PORTB PB1 on AVR hardware and toggle IsLedOn in TestHal_

**Status:** deprecated

**Dependencies:** —

**Superseded by:** [PLAT-POWER-004](platform.md#plat-power-004)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the LED behaviour is [PLAT-POWER-004](platform.md#plat-power-004).

---

### TECH-HAL-006
**Title:** Each cycle initialises the sensor bus once, before powering the sensors

**Status:** validated

**Dependencies:** [TECH-MAIN-001](#tech-main-001), [PLAT-POWER-003](platform.md#plat-power-003)

**Description:** The first `Hal` call of every cycle is `Init()`, made exactly once per cycle, before the sensors are powered and so before any sensor access. It sets up the I2C bus on builds that use one and does nothing otherwise. It runs every cycle because hibernate stops the bus peripheral ([PLAT-POWER-003](platform.md#plat-power-003)).

---

### TECH-HAL-007
**Title:** Attiny84aHal names the PLAT-POWER behaviour: Hibernate, PowerOnSensors / PowerOffSensors, TRANSMIT_WITH_LED

**Status:** deprecated

**Dependencies:** [TECH-HAL-004](#tech-hal-004), [TECH-HAL-005](#tech-hal-005), [PLAT-POWER-001](platform.md#plat-power-001), [PLAT-POWER-002](platform.md#plat-power-002), [PLAT-POWER-004](platform.md#plat-power-004), [PLAT-POWER-006](platform.md#plat-power-006)

**Description:** Deprecated 2026-11-20: it only named the code behind [PLAT-POWER-001](platform.md#plat-power-001) to 006, now in [docs/architecture.md](../docs/architecture.md).

---

### TECH-FILTER-001
**Title:** AnalogFilter discards the first `exclusion` values then accumulates up to `samples` values and returns their integer floor mean

**Status:** deprecated

**Dependencies:** —

**Description:** Deprecated 2026-11-20: `AnalogFilter` had no production caller and was removed.

---

### TECH-FILTER-002
**Title:** Values pushed to AnalogFilter after the samples count is reached are silently ignored

**Status:** deprecated

**Dependencies:** [TECH-FILTER-001](#tech-filter-001)

**Description:** Deprecated 2026-11-20: `AnalogFilter` had no production caller and was removed.

---

### TECH-CONVERT-001
**Title:** dec16ToHex returns the BCD hex representation of a uint16_t with each decimal digit in one nibble

**Status:** deprecated

**Dependencies:** —

**Superseded by:** [FUNC-X10-006](functional.md#func-x10-006)

**Description:** Deprecated 2026-11-20: helper detail; the BCD encoding it serves is [FUNC-X10-006](functional.md#func-x10-006) and [FUNC-X10-007](functional.md#func-x10-007).

---

### TECH-CONVERT-002
**Title:** dec32ToHex returns the 32-bit BCD hex representation of a uint32_t with each decimal digit in one nibble

**Status:** deprecated

**Dependencies:** —

**Description:** Deprecated 2026-11-20: `dec32ToHex` had no production caller and was removed.

---

### TECH-SERIAL-001
**Title:** SoftSerial transmits output-only UART at 9600 baud using the SerialWriteBit HAL call

**Status:** deprecated

**Dependencies:** [TECH-HAL-001](#tech-hal-001)

**Superseded by:** [PLAT-DEBUG-001](platform.md#plat-debug-001)

**Description:** Deprecated 2026-11-20: the debug serial output is now [PLAT-DEBUG-001](platform.md#plat-debug-001); the `SoftSerial` / `SerialWriteBit` split is in [docs/architecture.md](../docs/architecture.md).

---

### TECH-SERIAL-002
**Title:** With USE_SERIAL_LOG only, each cycle writes its measured values as text lines on the debug serial output

**Status:** draft

**Dependencies:** [PLAT-DEBUG-001](platform.md#plat-debug-001)

**Description:** With `USE_SERIAL_LOG`, each cycle writes one line per value it measures on the debug serial output ([PLAT-DEBUG-001](platform.md#plat-debug-001)), as `<name>: <value><unit>` with the value an unsigned decimal, in this order and only for the values the build measures: `Vbatt: <mV> mV`, `Raw analog: <ADC reading> ADC`, `Analog voltage: <mV> mV`, `Luminosity: <lux> Lux`, `Temp.: <tenths of °C> °C`, `RH : <%> %`, `Pressure : <hPa> hPA`. Without it, the firmware writes nothing and links no serial code, which keeps flash and cycle time for the sensor builds.

---

### TECH-X10-001
**Title:** Hal must expose five timing methods for X10 RF signal intervals

**Status:** deprecated

**Dependencies:** [TECH-HAL-001](#tech-hal-001)

**Superseded by:** [FUNC-X10-005](functional.md#func-x10-005)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the X10 timings are [FUNC-X10-005](functional.md#func-x10-005).

---

### TECH-X10-002
**Title:** x10rf must call only Hal timing methods for all delays; direct _delay_us() calls are forbidden

**Status:** deprecated

**Dependencies:** [TECH-X10-001](#tech-x10-001)

**Superseded by:** [FUNC-X10-005](functional.md#func-x10-005)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the X10 timings are [FUNC-X10-005](functional.md#func-x10-005), and [TECH-HAL-001](#tech-hal-001) keeps all timing behind the HAL.

---

### TECH-SENSOR-001
**Title:** With a DS18B20, the cycle touches no AVR register and PA3 is released after each read

**Status:** draft

**Dependencies:** [TECH-HAL-001](#tech-hal-001), [FUNC-SENSOR-002](functional.md#func-sensor-002)

**Description:** On a DS18B20 build, all 1-Wire access goes through a sensor wrapper given the `Hal` ([TECH-HAL-001](#tech-hal-001)): the cycle (`runCycle`) touches no AVR register and calls no 1-Wire driver function, so native builds run it with the sensor reading 0. After each read, PA3 is released: an input without internal pull-up, never left driven high (the 1-Wire driver leaves it driven high). R6 pulls the line up to `SENSOR_VCC` while the sensors are powered ([PLAT-POWER-007](platform.md#plat-power-007)), so a released PA3 never powers the DS18B20 with the sensor rail off.

---

### TECH-SENSOR-002
**Title:** BMx280 wraps BMP280 and BME280 behind one class; stubs return 0 on native builds

**Status:** deprecated

**Dependencies:** [TECH-HAL-001](#tech-hal-001), [FUNC-SENSOR-001](functional.md#func-sensor-001)

**Superseded by:** [FUNC-SENSOR-001](functional.md#func-sensor-001)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the sensor behaviour is [FUNC-SENSOR-001](functional.md#func-sensor-001), and [TECH-HAL-001](#tech-hal-001) covers wrappers built on a third-party driver.

---

### TECH-MAIN-001
**Title:** One measurement and transmission cycle is a standalone `runCycle(Hal*)`, so the whole cycle runs in native builds

**Status:** validated

**Dependencies:** [TECH-HAL-001](#tech-hal-001)

**Description:** A complete cycle (sensor power-on, measurement, transmissions, sensor power-off, hibernate) is the free function `void runCycle(Hal* hal)` (`emitter/include/main.h`). Every `Hal` call it makes, directly or through the encoders and sensor wrappers it uses, goes to the injected `hal`. After the leading `Init()` ([TECH-HAL-006](#tech-hal-006)), its calls come in this order: power on, sensor reads, X10 meters, luminosity, the environment frame twice ([FUNC-MAIN-001](functional.md#func-main-001)), power off, hibernate. The firmware entry point only calls `runCycle` in an endless loop and is left out of native builds.

---

### TECH-DECODE-001
**Title:** Loopback helper converts TestHal_ Orders into level/duration pulses for decoder tests

**Status:** deprecated

**Dependencies:** [TECH-HAL-002](#tech-hal-002), [TECH-X10-001](#tech-x10-001)

**Description:** Deprecated 2026-11-20: test helper, described in [docs/architecture.md](../docs/architecture.md); its tests are tagged with [FUNC-DECODE-001](functional.md#func-decode-001).

### TECH-DECODE-002
**Title:** Receiver hardware is reached only through a compile-time `ReceiverHal` interface given by pointer, with a native test double

**Status:** validated

**Dependencies:** —

**Description:** The receiver logic gets the hardware through a `ReceiverHal*` given at construction and touches no AVR register. `ReceiverHal` is chosen at compile time, without virtual call: the ATmega implementation on the ATmega328P and ATmega32U4, a test double on native builds; any other target fails to compile. Its operations: pop the oldest ended pulse (level, duration in µs; false when none), read the current second, draw an 8x16 cell (text page 0–2, column 0–15), draw a 16x32 cell (column 0–5), and, when the compile-time `HasLineOutput` is true, write one text line without terminator. The test double lets a caller push pulses, set the time, and read back the drawn cells, the number of draws and the written lines.

### TECH-DECODE-003
**Title:** Receiver logic is plain C++ without dynamic allocation or `printf`, independent of the hardware

**Status:** validated

**Dependencies:** [TECH-DECODE-002](#tech-decode-002)

**Verification:** build — `pio run -d tinyDecoders` compiles the receiver logic for AVR, and the native build runs it

**Description:** The decoders, the source table, the RFLink formatter, the page formatter and the receiver loop depend only on `<stdint.h>`, on each other and on `ReceiverHal` ([TECH-DECODE-002](#tech-decode-002)): no `Hal`, no AVR register, no dynamic allocation, no `printf`. They therefore run unchanged in native builds, and their RAM use is fixed at build time: the source table holds 16 sources on AVR, about 288 B, which fits the Pro Mini and the Pro Micro, and 8 in native builds. The RFLink formatter is linked only when `ReceiverHal::HasLineOutput` is true.

### TECH-DECODE-004
**Title:** FormatPage, ReadGlyph and ReadBigGlyphPage build the OLED cells without dynamic allocation

**Status:** deprecated

**Dependencies:** [TECH-DECODE-003](#tech-decode-003), [UI-DECODE-001](ui.md#ui-decode-001), [UI-DECODE-002](ui.md#ui-decode-002), [UI-DECODE-003](ui.md#ui-decode-003)

**Superseded by:** [UI-DECODE-003](ui.md#ui-decode-003)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the screen content is [UI-DECODE-001](ui.md#ui-decode-001) to 003.

### TECH-DECODE-005
**Title:** Arduino receiver: shared entry point, and AtmegaReceiverHal built on Timer1 interrupts, Wire and Serial

**Status:** deprecated

**Dependencies:** [TECH-DECODE-002](#tech-decode-002), [TECH-DECODE-003](#tech-decode-003), [PLAT-DECODE-003](platform.md#plat-decode-003), [PLAT-DECODE-004](platform.md#plat-decode-004), [PLAT-DECODE-005](platform.md#plat-decode-005), [CONF-DECODE-003](configuration.md#conf-decode-003), [CONF-DECODE-004](configuration.md#conf-decode-004)

**Superseded by:** [PLAT-DECODE-003](platform.md#plat-decode-003)

**Description:** Deprecated 2026-11-20: design detail, described in [docs/architecture.md](../docs/architecture.md); the receiver hardware use is [PLAT-DECODE-003](platform.md#plat-decode-003) to 005.
