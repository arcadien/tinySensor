# Technical Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### TECH-HAL-001
**Title:** Encoders and sensor wrappers that use the HAL receive Hal* at construction; no direct AVR register access
**Status:** validated
**Dependencies:** —
**Description:** Every protocol encoder (`OregonV3`, `LacrosseWS7000`, `x10rf`, `SoftSerial`) and every sensor wrapper that performs hardware timing or I/O itself (`Ds18b20`, `LightSensor`) must accept a `Hal*` at construction and must use only `Hal` methods for that I/O. A sensor wrapper whose hardware access is entirely delegated to a third-party bus driver (`BMx280`) takes no `Hal*`. Direct access to AVR registers (`PORTA`, `PORTB`, `ADCSRA`, etc.) is forbidden outside `Attiny84aHal`, except register addresses passed as arguments to a third-party bus driver. Verification: inspection; outside `Attiny84aHal`, the only AVR registers in `emitter/` and `lib/` are the addresses `Ds18b20` passes to the ds18b20 driver.

---

### TECH-HAL-002
**Title:** TestHal_ records each HAL call as a single-character token in the Orders vector
**Status:** validated
**Dependencies:** [TECH-HAL-001](#tech-hal-001)
**Description:** `TestHal_` appends one character per call to its `Orders` vector: `H`=`RadioGoHigh`, `L`=`RadioGoLow`, `D`=`Delay512Us`, `P`=`Delay1024Us`, `A`=`Delay400Us`, `S`=`SerialWriteBit(true)`, `W`=`SerialWriteBit(false)`, `I`=`Init` ([TECH-HAL-006](#tech-hal-006)), `Q`=`DelayX10PreambleHigh`, `R`=`DelayX10PreambleLow`, `T`=`DelayX10BitShort`, `U`=`DelayX10BitLong`, `G`=`DelayX10Gap`. Protocol tests verify correctness by asserting against expected `Orders` sequences.

---

### TECH-HAL-003
**Title:** ComputeVccMv and ConvertAnalogValueToMv are non-virtual concrete methods on the Hal base class
**Status:** validated
**Dependencies:** —
**Description:** `Hal::ComputeVccMv(uint16_t known11refValue)` and `Hal::ConvertAnalogValueToMv(uint16_t adcReading, uint16_t vccInMv)` are implemented directly on `Hal` and are not overridden by any subclass. Both `Attiny84aHal` and `TestHal_` inherit them unchanged. Verification: inspection; `test_power` calls both methods on `TestHal_` through the base class.

---

### TECH-HAL-004
**Title:** Attiny84aHal constructor disables Timer1, ADC, and analog comparator; no pull-up on PORTA, PA5 digital input buffer off, pull-ups on PB2 and PB3
**Status:** validated
**Dependencies:** [PLAT-POWER-007](platform.md#plat-power-007)
**Description:** `Attiny84aHal()` calls `UseLessPowerAsPossible()`, which sets `DDRA`/`DDRB` to 0, sets `PORTA = 0` (no internal pull-up on any PORTA pin) and `PORTB = 0b11111100` (pull-ups on PB2 and PB3, the only other PORTB pins: PB4–PB7 don't exist on the ATtiny84A; PB0 and PB1 are then made outputs for the radio and the LED; with `USE_SWITCHED_5V`, PB2's pull-up is cleared again, [TECH-HAL-007](#tech-hal-007), [PLAT-POWER-006](platform.md#plat-power-006)), disables the analog comparator (`ACSR|=ACD`), disables Timer1 (`PRR|=PRTIM1`), clears `ADCSRA` and disables the ADC (`PRR|=PRADC`). Brownout detection is disabled before each sleep period by `sleep()`, not here ([TECH-HAL-007](#tech-hal-007)). PORTA has no unused pin: PA0/PA1 are analog inputs, PA2 (`SENSOR_VCC`) and PA7 are outputs, and PA3–PA6 are the sensor bus pins, PA3/PA4/PA6 pulled up externally to `SENSOR_VCC` ([PLAT-POWER-007](platform.md#plat-power-007)). PA5 (MISO) has no pull-up at all. It reaches the ISP header J6 and pad 3 of the BME280 SDO jumper JP6, and reaches the sensor SDO only when JP6 is bridged 2–3. On v3.0, JP6 is delivered open and one side is bridged at assembly: 1–2 ties SDO to GND, which gives I2C address 0x76, the address the firmware reads; 2–3 ties SDO to PA5, for SPI, and in I2C mode SDO would then follow the floating PA5 and the address would be undefined. The constructor disables its digital input buffer (`DIDR0` bit `ADC5D`, next to `ADC1D` and `ADC2D`), so the floating pin draws no current in the input stage while awake; ISP programming is not affected, since `DIDR0` is cleared by reset. While the MCU is awake with PA2 low (boot and wake before the sensors are powered, after they are powered off before sleep), an internal pull-up on PA3–PA6 would drive current through the external pull-ups into PA2 and back-power the sensors. Verification: AVR only, no native test (`TestHal_` has no registers): inspection and the AVR builds; bench check done on 2026-11-19 on the v2.2 mushroom board: the BME280 is read with the external pull-ups only.

---

### TECH-HAL-005
**Title:** LedOn and LedOff toggle PORTB PB1 on AVR hardware and toggle IsLedOn in TestHal_
**Status:** validated
**Dependencies:** —
**Description:** `Attiny84aHal::LedOn()` sets bit `PB1` in `PORTB`; `LedOff()` clears it. In `TestHal_`, `LedOn()` sets `IsLedOn = true` and `LedOff()` sets `IsLedOn = false`. The `test_led` suite verifies this boolean toggle.

---

### TECH-HAL-006
**Title:** runCycle calls Hal::Init (I2C bus set-up) once, as its first HAL call, before sensor power-on
**Status:** validated
**Dependencies:** [TECH-MAIN-001](#tech-main-001), [PLAT-POWER-003](platform.md#plat-power-003)
**Description:** `runCycle(Hal*)` calls `hal->Init()` exactly once per cycle, as its first `Hal` call, before `PowerOnSensors()` and so before any sensor access. `main()` does not call `Init()`. `Attiny84aHal::Init()` calls `TinyI2C.init()` when compiled with `USE_I2C` and is a no-op otherwise. Running it every cycle re-initialises the USI after `Hibernate()` stopped its clock with `PRUSI` ([PLAT-POWER-003](platform.md#plat-power-003)). `TestHal_::Init()` appends `'I'` to `Orders`; `I2CIsConfigured` is removed. In every `native_cycle_*` env, `test_main_cycle` checks that `'I'` is the first token of the cycle and appears exactly once.

---

### TECH-HAL-007
**Title:** Attiny84aHal names the PLAT-POWER behaviour: Hibernate, PowerOnSensors / PowerOffSensors, TRANSMIT_WITH_LED
**Status:** validated
**Dependencies:** [TECH-HAL-004](#tech-hal-004), [TECH-HAL-005](#tech-hal-005), [PLAT-POWER-001](platform.md#plat-power-001), [PLAT-POWER-002](platform.md#plat-power-002), [PLAT-POWER-004](platform.md#plat-power-004), [PLAT-POWER-006](platform.md#plat-power-006)
**Description:** `Attiny84aHal::Hibernate(s)` calls the file-static `sleep(s)` of `lib/hal/Attiny84aHal.cpp`, which computes `s >> 3` (minimum 1) watchdog periods, saves / restores PRR, DDRA, DDRB, PORTA and PORTB, and right before each sleep instruction runs the timed BOD-disable sequence (`sleep_bod_disable()`: BODS and BODSE, then BODS alone, then `sleep_cpu()` within 3 cycles) ([PLAT-POWER-001](platform.md#plat-power-001), [PLAT-POWER-003](platform.md#plat-power-003)). `PowerOnSensors()` drives PA2 high and `PowerOffSensors()` low ([PLAT-POWER-002](platform.md#plat-power-002)); with `USE_SWITCHED_5V`, the constructor clears `PORTB.2`, `PowerOnSensors()` sets `DDRB.2` and `PowerOffSensors()` clears it ([PLAT-POWER-006](platform.md#plat-power-006)). In `emitter/src/main.cpp`, the macro `TRANSMIT_WITH_LED(code)` expands to `hal->LedOn()`, `code`, `hal->LedOff()`, `hal->Delay30ms()`, and wraps every `Send()` of the cycle ([PLAT-POWER-004](platform.md#plat-power-004)). `Delay512Us`, `Delay1024Us`, `Delay400Us` and `Delay30ms` are calibrated for 1 MHz ([PLAT-POWER-005](platform.md#plat-power-005)). Verification: inspection of `lib/hal/Attiny84aHal.cpp` and `emitter/src/main.cpp`; `test_main_cycle` records the LED and 30 ms delay calls around every frame in the cycle HAL sequence, and the AVR builds compile the AVR side.

---

### TECH-FILTER-001
**Title:** AnalogFilter discards the first `exclusion` values then accumulates up to `samples` values and returns their integer floor mean
**Status:** validated
**Dependencies:** —
**Description:** `AnalogFilter::Push(value)` decrements `exclusion` and discards the value until `exclusion` reaches zero; thereafter it accumulates values into `accumulator` up to `samples` count. `Get()` returns `accumulator / actualValuesCount` (integer division), or 0 if no values were accumulated.

---

### TECH-FILTER-002
**Title:** Values pushed to AnalogFilter after the samples count is reached are silently ignored
**Status:** validated
**Dependencies:** [TECH-FILTER-001](#tech-filter-001)
**Description:** Once `actualValuesCount` equals `samples`, subsequent `Push()` calls do nothing: the accumulator is not modified and `actualValuesCount` is not incremented. `Get()` continues to return the mean of the first `samples` values only.

---

### TECH-CONVERT-001
**Title:** dec16ToHex returns the BCD hex representation of a uint16_t with each decimal digit in one nibble
**Status:** validated
**Dependencies:** —
**Description:** `ConversionTools::dec16ToHex(n)` packs each decimal digit of `n` into one nibble of the returned `uint16_t` (e.g. 4200 → `0x4200`). Inputs above 9999 saturate to `0x9999` (the largest 4-digit BCD value) instead of overflowing.

---

### TECH-CONVERT-002
**Title:** dec32ToHex returns the 32-bit BCD hex representation of a uint32_t with each decimal digit in one nibble
**Status:** validated
**Dependencies:** —
**Description:** `ConversionTools::dec32ToHex(n)` packs each decimal digit of `n` into one nibble of the returned `uint32_t` (e.g. 1234567 → `0x01234567`). Inputs above 99,999,999 cause the most-significant nibble to overflow silently; no error is raised.

---

### TECH-SERIAL-001
**Title:** SoftSerial transmits output-only UART at 9600 baud using the SerialWriteBit HAL call
**Status:** validated
**Dependencies:** [TECH-HAL-001](#tech-hal-001)
**Description:** `SoftSerial` accepts a `Hal*` at construction and drives UART output exclusively via `SerialWriteBit(level)`, one call per bit (start bit low, 8 data bits LSB first, stop bit high; `begin()` sets the idle level high). `SoftSerial` itself performs no delay: `SerialWriteBit` sets the line level and holds it for one bit period, `(1 000 000 / 9600) − 25` µs on AVR (`SERIAL_BIT_PERIOD_US` in `lib/hal/Attiny84aHal.cpp`, the −25 µs compensating the per-bit call overhead at 1 MHz). No receive path is implemented.

---

### TECH-SERIAL-002
**Title:** Debug logging via SoftSerial is compiled only when USE_SERIAL_LOG is defined
**Status:** draft
**Dependencies:** [TECH-SERIAL-001](#tech-serial-001)
**Description:** When `USE_SERIAL_LOG` is defined, `emitter/src/main.cpp` includes `SoftSerial.h`, instantiates `SoftSerial swSerial(&hal)`, and implements `SerialPrintInfo(name, value, unit)` using it. When `USE_SERIAL_LOG` is absent, `SerialPrintInfo` is a no-op inline function and `SoftSerial` is not instantiated. Verification: inspection; no build environment sets `USE_SERIAL_LOG`, so the logging path is not compiled in CI.

---

### TECH-X10-001
**Title:** Hal must expose five timing methods for X10 RF signal intervals
**Status:** validated
**Dependencies:** [TECH-HAL-001](#tech-hal-001)
**Description:** The `Hal` interface must declare five new pure-virtual methods for X10 RF timing: `DelayX10PreambleHigh()` (8960 µs), `DelayX10PreambleLow()` (4500 µs), `DelayX10BitShort()` (560 µs), `DelayX10BitLong()` (1120 µs), and `DelayX10Gap()` (40000 µs). `Attiny84aHal` must implement each with the corresponding `_delay_us()` call calibrated to `F_CPU = 1 000 000`. `TestHal_` must implement each by appending a distinct single-character token to `Orders`.

---

### TECH-X10-002
**Title:** x10rf must call only Hal timing methods for all delays; direct _delay_us() calls are forbidden
**Status:** validated
**Dependencies:** [TECH-X10-001](#tech-x10-001)
**Description:** `x10rf::SendCommand()` and `x10rf::SendX10RfBit()` must replace all calls to the file-static `x10_*` delay functions with direct calls to the corresponding `Hal` methods (`DelayX10PreambleHigh()`, `DelayX10PreambleLow()`, `DelayX10BitShort()`, `DelayX10BitLong()`, `DelayX10Gap()`). The `#if defined(AVR)` block defining those static functions and including `<util/delay.h>` must be removed entirely from `lib/x10/x10rf.cpp`.

---

### TECH-SENSOR-001
**Title:** DS18B20 access encapsulated in a Ds18b20 wrapper class; main.cpp must not reference AVR registers or ds18b20 library functions directly
**Status:** draft
**Dependencies:** [TECH-HAL-001](#tech-hal-001), [FUNC-SENSOR-002](functional.md#func-sensor-002)
**Description:** A `Ds18b20` wrapper class (header `emitter/include/Ds18b20.h`) must encapsulate all 1-Wire DS18B20 interactions. On AVR, `Ds18b20` must accept a `Hal*` at construction and expose `Begin()`, `Convert()`, `Read()` (returning `int16_t` temperature in °C as integer), and use `hal->Delay1s()` between `ds18b20convert` and `ds18b20read` internally. On non-AVR targets all methods must be stubs returning 0. `main.cpp` must not call `ds18b20convert()`, `ds18b20read()`, or reference `PORTA`, `DDRA`, or `PINA` in any `USE_DS18B20` code path; all such calls are delegated exclusively to `Ds18b20`. Once `Read()` has finished, `Ds18b20` releases PA3: input, no internal pull-up, never left driven high (the 1-Wire library leaves it an output driven high). R6 pulls the line up to `SENSOR_VCC` while the sensors are powered ([PLAT-POWER-007](platform.md#plat-power-007)), so a released PA3 never powers the DS18B20 with the sensor rail off. Not implemented yet: to do when the DS18B20 path is validated again.

---

### TECH-SENSOR-002
**Title:** BMx280 wraps BMP280 and BME280 behind one class; stubs return 0 on native builds
**Status:** validated
**Dependencies:** [TECH-HAL-001](#tech-hal-001), [FUNC-SENSOR-001](functional.md#func-sensor-001)
**Description:** Class `BMx280` (`emitter/include/BMx280.h`) exposes `Begin()`, `GetTemperature()` (°C), `GetPressure()` (Pa), `GetHumidity()` (%) and `Shutdown()` (sensor sleep mode). On AVR it forwards to the SparkFun BME280 library at I2C address 0x76; on non-AVR targets every method is a stub and the getters return 0. It takes no `Hal*` ([TECH-HAL-001](#tech-hal-001)). Verification: inspection of `emitter/include/BMx280.h`; the AVR builds compile the SparkFun side, and the native builds (`test_main_cycle`) link the stubs.

---

### TECH-MAIN-001
**Title:** One measurement/transmission cycle is a standalone runCycle(Hal*) callable from native tests
**Status:** validated
**Dependencies:** [TECH-HAL-001](#tech-hal-001)
**Description:** One complete cycle (sensor power-on, measurement, transmissions, sensor power-off, hibernate) is a free function `void runCycle(Hal* hal)` declared in `emitter/include/main.h`. Every `Hal` call it makes, directly or through the encoders and sensor wrappers it uses, goes to the injected `hal`. For every build variant family, the HAL call sequence after the leading `Init()` ([TECH-HAL-006](#tech-hal-006)) equals the reference sequence the native cycle test builds directly from the encoders: power on, sensor reads, X10 meters, luminosity, the environment frame twice ([FUNC-MAIN-001](functional.md#func-main-001)), power off, hibernate. `main()` only calls `runCycle` in an endless loop, and is excluded from test builds. Dedicated native test environments with zero sleep time exercise `runCycle` for each build variant family: Oregon with divided battery, Oregon with battery equal to VCC, Oregon with a battery divider and without pressure, Lacrosse, and no environment protocol.

---

### TECH-DECODE-001
**Title:** Loopback helper converts TestHal_ Orders into level/duration pulses for decoder tests
**Status:** validated
**Dependencies:** [TECH-HAL-002](#tech-hal-002), [TECH-X10-001](#tech-x10-001)
**Description:** A native-only function `OrdersToPulses()` in `tinyDecoders/lib/Loopback/` converts a `TestHal_::Orders` token sequence into an ordered list of pulses `{bool high; uint16_t durationUs}`. The line starts low. `H` and `L` set the level; each delay token adds its nominal duration to the current level: `A`=400, `D`=512, `P`=1024, `T`=560, `U`=1120, `Q`=8960, `R`=4500, `G`=40000 µs (saturating at 65535). A pulse is emitted at each level change and after the last token; zero-duration pulses are not emitted. Other tokens are ignored. It runs in the `native` env of `tinyDecoders/platformio.ini`, which builds the repository `lib/` encoders.

### TECH-DECODE-002
**Title:** Receiver hardware is a compile-time ReceiverHal alias injected by pointer, with a native test double
**Status:** validated
**Dependencies:** [TECH-DECODE-001](#tech-decode-001)
**Description:** `tinyDecoders/lib/ReceiverHal/ReceiverHal.h` defines `typedef AtmegaReceiverHal ReceiverHal` when `__AVR_ATmega328P__` or `__AVR_ATmega32U4__` is defined, `typedef TestReceiverHal ReceiverHal` when `AVR` is not defined, and an `#error` otherwise (no virtual call, as `EnvironmentEncoder`); `AtmegaReceiverHal.cpp` is guarded by these two MCU macros. Both classes expose `bool PopPulse(bool& high, uint16_t& durationUs)` (oldest ended pulse, false when none), `uint32_t NowSeconds() const`, `void DrawChar(uint8_t page, uint8_t col, char c)` (8x16 cell on display pages `page` and `page + 1`, page 0–2, col 0–15), `void DrawBigChar(uint8_t col, char c)` (16x32 cell spanning both rows at pixel column `16*col`, col 0–5), `static const bool HasLineOutput` and `void WriteLine(const char* line)` (one text line without terminator). `AtmegaReceiverHal`: `HasLineOutput = true`. Native-only `TestReceiverHal` (`TestReceiverHal.h`, `HasLineOutput = true`) adds `PushPulse(high, durationUs)` (FIFO), `SetNowSeconds(s)`, `const char* Row(uint8_t page) const` (page 0–2, 16 characters, initially spaces, updated by `DrawChar` at that start page), `const char* Big() const` (6 characters, initially spaces, updated by `DrawBigChar`), `uint16_t DrawCount() const` (counts both draws) and `const std::vector<std::string>& Lines() const` (each `WriteLine` appends its line). Components needing the hardware receive a `ReceiverHal*` at construction and touch no AVR register. Test: double FIFO order, `Row` and `Big` after draws, `DrawCount` after one draw of each, `Lines` after two `WriteLine`.

### TECH-DECODE-003
**Title:** Receiver logic is plain C++ without dynamic allocation: decoders, SourceTable, RflinkFormatter and Receiver
**Status:** validated
**Dependencies:** [TECH-DECODE-002](#tech-decode-002)
**Description:** Classes `X10Decoder`, `LacrosseDecoder` and `OregonDecoder` (`tinyDecoders/lib/<Name>/<Name>.h`, default constructor, `<stdint.h>` only: no `Hal`, no AVR register) expose `bool Feed(bool high, uint16_t durationUs)`, true when a frame is decoded, and const getters valid until the next true: `Address()`, `Value()` (X10); `Type()`, `Address()`, `TemperatureTenths()`, `HumidityTenths()`, `PressureTenths()`, `Lux()` (Lacrosse); `DeviceId()`, `Channel()`, `RollingCode()`, `BatteryLow()`, `TemperatureTenths()`, `HumidityPercent()`, `PressureHpa()` (Oregon). `tinyDecoders/lib/SourceTable/SourceTable.h` (`<stdint.h>` and the decoder headers only, no dynamic allocation) defines `struct SourceRecord { Kind kind; uint8_t id; uint8_t code; uint8_t fields; int16_t temperatureTenths; uint16_t humidityTenths; uint16_t pressureTenths; uint16_t lux; uint16_t millivolts; uint32_t lastSeenSeconds; }` (18 B), `enum Kind : uint8_t { Oregon, Lacrosse, LacrosseLux, X10 }`, `fields` a bitmask `Temperature=1, Humidity=2, Pressure=4, Lux=8, Millivolts=16`, and class `SourceTable` (`kCapacity` 16 when `AVR` is defined, else 8; 16 × 18 = 288 B of RAM on the Pro Mini and the Pro Micro) with `Update(const <Decoder>&, uint32_t nowSeconds)` per decoder, `Expire(uint32_t nowSeconds)`, `Count()` and `At(uint8_t index)`. `tinyDecoders/lib/RflinkFormatter/RflinkFormatter.h` (`<stdint.h>` and the decoder headers only, no `printf`, no dynamic allocation) defines `RflinkLineSize = 64` and `uint8_t FormatRflink(const <Decoder>&, uint8_t counter, char line[RflinkLineSize])` per decoder, writing the line plus `'\0'` and returning its length. Class `Receiver` (`tinyDecoders/lib/Receiver/Receiver.h`, `explicit Receiver(ReceiverHal* hal)`, owns the three decoders and a `SourceTable`) exposes `void Poll()`, which pops pulses with `PopPulse`, reads time with `NowSeconds`, draws with `DrawChar` / `DrawBigChar` and writes lines with `WriteLine` ([TECH-DECODE-002](#tech-decode-002)); it calls `FormatRflink` only when the compile-time `ReceiverHal::HasLineOutput` is true, so the formatter is not linked otherwise. Verification: the [FUNC-DECODE-001](functional.md#func-decode-001) to 010 native tests use this API; the promini and promicro builds ([CONF-DECODE-002](configuration.md#conf-decode-002)) compile it for AVR.

### TECH-DECODE-004
**Title:** FormatPage, ReadGlyph and ReadBigGlyphPage build the OLED cells without dynamic allocation
**Status:** validated
**Dependencies:** [TECH-DECODE-003](#tech-decode-003), [UI-DECODE-001](ui.md#ui-decode-001), [UI-DECODE-002](ui.md#ui-decode-002), [UI-DECODE-003](ui.md#ui-decode-003)
**Description:** Free function `void FormatPage(const SourceRecord& record, bool alternate, char big[7], char top[5], char bottom[5])` (`tinyDecoders/lib/PageFormatter/PageFormatter.h`, no dynamic allocation, no `printf`) writes the [UI-DECODE-001](ui.md#ui-decode-001) fields, 6, 4 and 4 characters plus `'\0'`, space-filled; the degree sign is `'\x7F'`, and the lux unit cell is the character `LuxUnit` (`'\x01'`, declared in `Font8x16.h`) right after the lux digits (`"65535\x01"`, `"1000\x01 "`). Free function `void ReadGlyph(char c, uint8_t glyph[16])` (`tinyDecoders/lib/Font8x16/Font8x16.h`) copies the 8x16 glyph from a `PROGMEM` table (on native, `PROGMEM` is empty and reads are plain loads), about 32 × 17 = 544 B of flash. Bytes 0–7 are columns 0–7 of the top 8 pixel rows, bytes 8–15 the bottom 8 rows, bit 0 the top pixel: the SSD1306 page layout. The table is produced by `tinyDecoders/scripts/gen_font8x16.py <spleen-8x16.bdf>`, with the Spleen licence text in the generated file header. Free function `void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16])` (same header) writes page `page` (0–3, top to bottom) of the 16x32 glyph from `ReadGlyph(c)`, without RAM or flash table: with `s` = glyph byte `(page / 2) * 8 + x / 2` and `n` its low nibble (even page) or high nibble (odd page), bits `2i` and `2i+1` of `bytes[x]` are both bit `i` of `n`. For `LuxUnit`, pages 0–1 are zeros, and page `p` = 2 or 3 holds bytes `(p − 2) * 8` to `(p − 2) * 8 + 7` of `ReadGlyph('l')` in `bytes[0..7]` and of `ReadGlyph('x')` in `bytes[8..15]`. `ReadGlyph(LuxUnit)` is blank. The receiver draws the unit cell with `DrawBigChar` like any big cell, so `Receiver` and `ReceiverHal` are unchanged. Verification: the [UI-DECODE-001](ui.md#ui-decode-001) to 003 native tests call these three functions; the promini and promicro builds compile the `PROGMEM` table for AVR.

### TECH-DECODE-005
**Title:** Arduino receiver: shared entry point, and AtmegaReceiverHal built on Timer1 interrupts, Wire and Serial
**Status:** validated
**Dependencies:** [TECH-DECODE-002](#tech-decode-002), [TECH-DECODE-003](#tech-decode-003), [PLAT-DECODE-003](platform.md#plat-decode-003), [PLAT-DECODE-004](platform.md#plat-decode-004), [PLAT-DECODE-005](platform.md#plat-decode-005), [CONF-DECODE-003](configuration.md#conf-decode-003), [CONF-DECODE-004](configuration.md#conf-decode-004)
**Description:** `tinyDecoders/src/main.cpp`, the only file in `src/` (so no environment sets `build_src_filter`), defines `hal` (`AtmegaReceiverHal`) and `receiver` (`Receiver`, built on `&hal`) at file scope; `setup()` calls `hal.Init()` and `loop()` calls `receiver.Poll()`, under the Arduino `main`. `tinyDecoders/lib/ReceiverHal/AtmegaReceiverHal.{h,cpp}` is compiled only when `__AVR_ATmega328P__` or `__AVR_ATmega32U4__` is defined, and these macros pick the ICP1 pin. `Init()` sets up ICP1 and Timer1 ([PLAT-DECODE-003](platform.md#plat-decode-003)), calls `Wire.begin()`, `Wire.setClock(400000)` (`OledClockHz`) and sends the OLED set-up ([PLAT-DECODE-004](platform.md#plat-decode-004)), then `Serial.begin(57600)` ([PLAT-DECODE-005](platform.md#plat-decode-005)). `ISR(TIMER1_CAPT_vect)` pushes the ended pulse into the ring buffer, `ISR(TIMER1_OVF_vect)` advances the time base; `PopPulse` and `NowSeconds` read them inside `ATOMIC_BLOCK(ATOMIC_RESTORESTATE)`. `DrawChar` uses `ReadGlyph`, `DrawBigChar` uses `ReadBigGlyphPage` ([TECH-DECODE-004](#tech-decode-004)). `WriteLine(line)` drops the line when `Serial.availableForWrite()` is below its length + 2, else writes it then `\r\n`; `Serial`'s `operator bool`, which waits 10 ms on the Pro Micro, is never used. Verification: inspection, the promini and promicro builds ([CONF-DECODE-002](configuration.md#conf-decode-002)), and the bench checks of [PLAT-DECODE-003](platform.md#plat-decode-003) to 005.
