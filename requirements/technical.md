# Technical Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### TECH-HAL-001
**Title:** All encoders and sensor wrappers receive Hal* at construction and never access AVR registers directly
**Status:** validated
**Dependencies:** —
**Description:** Every protocol encoder (`OregonV3`, `LacrosseWS7000`, `x10rf`, `SoftSerial`) and sensor wrapper (`BMx280`) must accept a `Hal*` parameter at construction time and must call only `Hal` virtual methods for all hardware I/O. Direct access to AVR registers (`PORTA`, `PORTB`, `ADCSRA`, etc.) is forbidden outside `Attiny84aHal`.

---

### TECH-HAL-002
**Title:** TestHal_ records each HAL call as a single-character token in the Orders vector
**Status:** validated
**Dependencies:** TECH-HAL-001
**Description:** `TestHal_` appends one character per call to its `Orders` vector: `H`=`RadioGoHigh`, `L`=`RadioGoLow`, `D`=`Delay512Us`, `P`=`Delay1024Us`, `A`=`Delay400Us`, `S`=`SerialGoHigh`, `W`=`SerialGoLow`. Protocol tests verify correctness by asserting against expected `Orders` sequences.

---

### TECH-HAL-003
**Title:** ComputeVccMv and ConvertAnalogValueToMv are non-virtual concrete methods on the Hal base class
**Status:** validated
**Dependencies:** —
**Description:** `Hal::ComputeVccMv(uint16_t known11refValue)` and `Hal::ConvertAnalogValueToMv(uint16_t adcReading, uint16_t vccInMv)` are implemented directly on `Hal` and are not overridden by any subclass. Both `Attiny84aHal` and `TestHal_` inherit them unchanged.

---

### TECH-HAL-004
**Title:** Attiny84aHal constructor disables Timer1, ADC, and analog comparator; pull-ups all unused pins; disables brownout detection during sleep
**Status:** validated
**Dependencies:** —
**Description:** `Attiny84aHal()` calls `UseLessPowerAsPossible()`, which sets `DDRA`/`DDRB` to 0, applies pull-ups via `PORTA`/`PORTB`, disables the analog comparator (`ACSR|=ACD`), disables Timer1 (`PRR|=PRTIM1`), clears `ADCSRA` and disables the ADC (`PRR|=PRADC`), and deactivates brownout detection during sleep (`MCUCR BODS/BODSE` sequence).

---

### TECH-HAL-005
**Title:** LedOn and LedOff toggle PORTB PB1 on AVR hardware and toggle IsLedOn in TestHal_
**Status:** validated
**Dependencies:** —
**Description:** `Attiny84aHal::LedOn()` sets bit `PB1` in `PORTB`; `LedOff()` clears it. In `TestHal_`, `LedOn()` sets `IsLedOn = true` and `LedOff()` sets `IsLedOn = false`. The `test_led` suite verifies this boolean toggle.

---

### TECH-HAL-006
**Title:** Hal::Init initialises the I2C bus when USE_I2C is defined, and sets I2CIsConfigured in TestHal_
**Status:** validated
**Dependencies:** —
**Description:** `Attiny84aHal::Init()` calls `TinyI2C.init()` when compiled with `USE_I2C`; it is a no-op otherwise. `TestHal_::Init()` sets `I2CIsConfigured = true`. The `test_busses` suite verifies that `Init()` results in `I2CIsConfigured` being `true`.

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
**Dependencies:** TECH-FILTER-001
**Description:** Once `actualValuesCount` equals `samples`, subsequent `Push()` calls do nothing: the accumulator is not modified and `actualValuesCount` is not incremented. `Get()` continues to return the mean of the first `samples` values only.

---

### TECH-CONVERT-001
**Title:** dec16ToHex returns the BCD hex representation of a uint16_t with each decimal digit in one nibble
**Status:** validated
**Dependencies:** —
**Description:** `ConversionTools::dec16ToHex(n)` packs each decimal digit of `n` into one nibble of the returned `uint16_t` (e.g. 4200 → `0x4200`). Inputs above 9999 cause the most-significant nibble to overflow silently; no error is raised.

---

### TECH-CONVERT-002
**Title:** dec32ToHex returns the 32-bit BCD hex representation of a uint32_t with each decimal digit in one nibble
**Status:** validated
**Dependencies:** —
**Description:** `ConversionTools::dec32ToHex(n)` packs each decimal digit of `n` into one nibble of the returned `uint32_t` (e.g. 1234567 → `0x01234567`). Inputs above 99,999,999 cause the most-significant nibble to overflow silently; no error is raised.
