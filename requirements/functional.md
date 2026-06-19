# Functional Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### FUNC-OREGON-001
**Title:** Zero bit encoded as H-D512-L-D1024-H-D512 HAL call sequence
**Status:** validated
**Dependencies:** —
**Description:** When `SendZero()` is called, `OregonV3` emits exactly six HAL calls in order: `RadioGoHigh`, `Delay512Us`, `RadioGoLow`, `Delay1024Us`, `RadioGoHigh`, `Delay512Us`. This sequence is the sole representation of a zero bit on the radio channel.

### FUNC-OREGON-002
**Title:** One bit encoded as L-D512-H-D1024-L-D512 HAL call sequence
**Status:** validated
**Dependencies:** —
**Description:** When `SendOne()` is called, `OregonV3` emits exactly six HAL calls in order: `RadioGoLow`, `Delay512Us`, `RadioGoHigh`, `Delay1024Us`, `RadioGoLow`, `Delay512Us`. This sequence is the sole representation of a one bit on the radio channel.

### FUNC-OREGON-003
**Title:** Data bytes transmitted LSB-nibble first then MSB-nibble
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** `SendData()` iterates over each byte and transmits its low nibble (bits 0–3) before its high nibble (bits 4–7), each bit sent via `SendZero()` or `SendOne()`. This nibble order applies to every byte in the message, preamble, and postamble.

### FUNC-OREGON-004
**Title:** Frame begins with 24 one-bits preamble and ends with 2-zero-bit postamble byte
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** Every `Send()` call first transmits three `0xFF` preamble bytes (24 one-bits total via `SendData`), then the 11-byte message payload, then one `0x00` postamble byte (8 zero-bits, equivalent to two zero nibbles).

### FUNC-OREGON-005
**Title:** Positive temperature encoded as decimal, unit, and dozen nibbles in bytes 4–5
**Status:** validated
**Dependencies:** —
**Description:** `SetTemperature()` with a non-negative value encodes the tenths digit into the high nibble of byte 4, the units digit into the low nibble of byte 4, and the dozens digit into the high nibble of byte 5. The low nibble of byte 5 is reserved for the sign flag.

### FUNC-OREGON-006
**Title:** Negative temperature sets sign bit in byte 5 and encodes absolute value
**Status:** validated
**Dependencies:** FUNC-OREGON-005
**Description:** When the temperature argument to `SetTemperature()` is negative, `OregonV3` sets bit 3 of byte 5 (`0x08`) as a negative sign flag, then encodes the absolute value using the same decimal/unit/dozen layout as for positive temperatures.

### FUNC-OREGON-007
**Title:** Humidity encoded in byte 6 with units and dozens nibbles swapped
**Status:** validated
**Dependencies:** —
**Description:** `SetHumidity()` accepts a value in the range 0–99 and stores the units digit in the high nibble of byte 6 and the dozens digit in the low nibble of byte 6, reversing the conventional tens-before-units order.

### FUNC-OREGON-008
**Title:** Pressure encoded with 795 hPa offset in bytes 7–8; weather prediction byte at byte 9
**Status:** validated
**Dependencies:** —
**Description:** `SetPressure()` subtracts 795 from the hPa value and stores the result across bytes 7 and 8. Byte 9 holds a weather-prediction code: 0x30 (rain, <1000 hPa), 0x20 (cloudy, <1010), 0x60 (partly cloudy, <1025), or 0xC0 (sunny, ≥1025).

### FUNC-OREGON-009
**Title:** Channel (1–3) encoded as 1-bit mask in high nibble of byte 2; out-of-range silently ignored
**Status:** validated
**Dependencies:** —
**Description:** `SetChannel()` accepts values 1, 2, or 3 and sets bit `(4 + channel - 1)` of byte 2, producing 0x10, 0x20, or 0x40 respectively. Any value outside the range 1–3 is silently ignored and the message byte is left unchanged.

### FUNC-OREGON-010
**Title:** Rolling code (≤ 0xA5) stored across low nibble of byte 2 and high nibble of byte 3
**Status:** validated
**Dependencies:** —
**Description:** `SetRollingCode()` writes the high nibble of the rolling code byte into the low nibble of byte 2 and the low nibble of the rolling code byte into the high nibble of byte 3. Values greater than `0xA5` are accepted without validation.

### FUNC-OREGON-011
**Title:** Battery-low flag is bit 2 of byte 3; cleared when battery is OK
**Status:** validated
**Dependencies:** —
**Description:** `SetBatteryLow(true)` sets bit 2 of byte 3 (`message[3] |= 0x04`). `SetBatteryLow(false)` clears bit 2 (`message[3] &= 0xFB`). All other bits of byte 3 are unaffected.

### FUNC-OREGON-012
**Title:** Device ID auto-selected at Send() based on which fields are populated
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** `FinalizeMessage()` writes the Oregon Scientific device ID into bytes 0–1 based on `messageStatus`: temperature-only sets `0xEC40` (THN132N); temperature and humidity sets `0x1D20` (THGN123N); temperature, humidity, and pressure sets `0x5A5D` (BTHR918). The checksum is computed and appended accordingly.

### FUNC-OREGON-013
**Title:** messageStatus bit-field tracks which fields are set (bit 0=temp, bit 1=humi, bit 2=press)
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** The private `messageStatus` byte is initialised to 0. `SetTemperature()` sets bit 0, `SetHumidity()` sets bit 1, and `SetPressure()` sets bit 2. The field is readable via `GetMessageStatus()` and governs device-ID selection in `FinalizeMessage()`.

### FUNC-OREGON-014
**Title:** Full frame byte sequence matches Oregon Scientific v3 reference decoding samples
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002, FUNC-OREGON-003, FUNC-OREGON-004, FUNC-OREGON-005, FUNC-OREGON-006, FUNC-OREGON-007, FUNC-OREGON-008, FUNC-OREGON-009, FUNC-OREGON-010, FUNC-OREGON-011, FUNC-OREGON-012, FUNC-OREGON-013
**Description:** The 11-byte payload produced by `FinalizeMessage()` for known channel, rolling code, battery, temperature, humidity, and pressure inputs must match the reference hex strings from the Oregon Scientific v3 protocol document (e.g., `5A5D423448088209603054`, `1D20485448088200390000`, `1D2016B0091073002C0000`).
