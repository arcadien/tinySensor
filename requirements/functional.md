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

### FUNC-LACROSSE-001
**Title:** Preamble is exactly 10 consecutive zero bits
**Status:** validated
**Dependencies:** —
**Description:** `SendPreamble()` transmits exactly 10 zero bits by calling `SendZero()` ten times. No other bits precede the preamble. The preamble is always followed by a single one-bit start marker emitted by `Send()` before the first nibble.

### FUNC-LACROSSE-002
**Title:** Sensor-type nibble encodes available data combination; unknown combination emits 0xF
**Status:** validated
**Dependencies:** —
**Description:** `SendType()` maps the `availableData` bitmask to a type nibble: temperature-only (0x01) → 0x0, temperature and humidity (0x03) → 0x1, temperature, humidity, and pressure (0x07) → 0x4, luminosity-only (0x08) → 0x5. Any other combination emits 0xF.

### FUNC-LACROSSE-003
**Title:** Address (0–7) in bits 0–2 of second nibble; negative temperature sign in bit 3
**Status:** validated
**Dependencies:** —
**Description:** `SendAddressAndTemperatureSign()` encodes the 3-bit sensor address in bits 0–2. If temperature is available and negative, bit 3 is set to 1. The resulting nibble is sent via `SendNibble()`.

### FUNC-LACROSSE-004
**Title:** Temperature clamped to ±99 °C and encoded as three nibbles: decimal, unit, dozen
**Status:** validated
**Dependencies:** —
**Description:** `SetTemperature()` clamps the input to [−99, +99] °C. `SendTemperature()` then transmits three nibbles in order: tenths digit, units digit, dozens digit. The sign is communicated separately via FUNC-LACROSSE-003.

### FUNC-LACROSSE-005
**Title:** Humidity clamped to 99.9 % maximum and encoded as three nibbles: decimal, unit, dozen
**Status:** validated
**Dependencies:** —
**Description:** `SetHumidity()` clamps any value ≥ 100 % to 99.9 %. `SendHumidity()` transmits three nibbles in order: tenths digit, units digit, dozens digit.

### FUNC-LACROSSE-006
**Title:** Pressure clamped to 850–1100 hPa, offset by −200, encoded as four nibbles: units, dozens, hundreds, decimal
**Status:** validated
**Dependencies:** —
**Description:** `SetPressure()` clamps the input to [850, 1100] hPa then subtracts 200. `SendPressure()` transmits four nibbles in order: units digit, dozens digit, hundreds digit, tenths digit of the offset value.

### FUNC-LACROSSE-007
**Title:** Luminosity value encoded as 7 nibbles in a type-5 (light sensor) frame
**Status:** validated
**Dependencies:** —
**Description:** `SendLuminosity()` transmits the units, dozens, and hundreds nibbles of the lux value followed by four zero nibbles, giving a fixed 7-nibble payload. The type nibble for luminosity frames is 0x5 (see FUNC-LACROSSE-002).

### FUNC-LACROSSE-008
**Title:** Frame ends with XOR checksum nibble followed by running-sum nibble initialised to 5
**Status:** validated
**Dependencies:** FUNC-LACROSSE-001, FUNC-LACROSSE-002, FUNC-LACROSSE-003, FUNC-LACROSSE-004, FUNC-LACROSSE-005, FUNC-LACROSSE-006, FUNC-LACROSSE-007
**Description:** `Send()` maintains a running XOR (`checkXor`, init 0) and a running sum (`checkSum`, init 5) over every transmitted nibble. After all data nibbles, `checkXor` is masked to 4 bits, added into `checkSum` (also masked to 4 bits), and both are transmitted as the final two nibbles.

### FUNC-LACROSSE-009
**Title:** Each nibble is followed by a mandatory trailing one-bit separator
**Status:** validated
**Dependencies:** —
**Description:** `SendNibble()` always calls `SendOne()` after transmitting its four data bits. This trailing one-bit separator is unconditional and applies to every nibble in the frame, including type, address, data, and checksum nibbles.

### FUNC-X10-001
**Title:** RFXmeter builds a 6-byte frame with address, partial complement, 24-bit value, packet-type nibble, and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** `RFXmeter()` fills a 6-byte buffer: byte[0]=address, byte[1]=(upper nibble of ~address)+(lower nibble of address), bytes[2–4]=24-bit value (clamped to 0 on overflow), byte[5] upper nibble=packet type, byte[5] lower nibble=bitwise NOT of sum of all byte nibble-pairs. The buffer is transmitted via `SendCommand`.

### FUNC-X10-002
**Title:** RFXsensor builds a 4-byte frame with address-type encoding, value, packet-type status, and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** `RFXsensor()` fills a 4-byte buffer: byte[0]=(address<<2)|type_bits (t=0,v=2,a=1,m=3), byte[1]=(upper nibble of ~byte[0])+(lower nibble of byte[0]), byte[2]=value, byte[3] upper nibble=packet-type status byte, byte[3] lower nibble=bitwise NOT of nibble sum of bytes[0–2] and byte[3] upper nibble. The buffer is transmitted via `SendCommand`.

### FUNC-X10-003
**Title:** x10Switch builds a 4-byte frame with house-code nibble lookup, bit-scattered unit encoding, and full-byte complements
**Status:** validated
**Dependencies:** —
**Description:** `x10Switch()` maps house-code characters 'a'–'p' to a 4-bit lookup table value placed in byte[0] upper nibble. Unit code bits are scattered across byte[0] bit 2 and byte[2] bits 3, 4, 6. Byte[1]=~byte[0] and byte[3]=~byte[2]. The 4-byte buffer is transmitted via `SendCommand`.

### FUNC-X10-004
**Title:** x10Security builds a 6-byte frame with XOR-fold parity reduced to a single bit
**Status:** validated
**Dependencies:** —
**Description:** `x10Security()` fills a 6-byte buffer: byte[0]=address, byte[1]=(lower nibble of ~byte[0])+(upper nibble of byte[0]), byte[2]=command, byte[3]=~command, byte[4]=id. Byte[5] is the XOR of all six bytes (using byte[5] bit 7 only), folded by successive XOR-halvings to a single bit. The buffer is transmitted via `SendCommand`.

### FUNC-X10-005
**Title:** Each transmission is repeated rf_repeats times with a 40 ms inter-repeat cooldown on AVR
**Status:** validated
**Dependencies:** —
**Description:** `SendCommand()` loops `_rf_repeats` times. Each iteration sends the preamble burst, all frame bits MSB-first, and a trailing stop bit, then calls `x10_cooldown_gap()` (40 000 µs delay on AVR, no-op on host). `LedOn`/`LedOff` bracket the full repeated sequence.

### FUNC-X10-006
**Title:** Battery voltage in mV is BCD-encoded and transmitted via RFXmeter when battery is not low
**Status:** validated
**Dependencies:** FUNC-X10-001, FUNC-BATTERY-001, FUNC-BATTERY-002
**Description:** When `BATTERY_VOLTAGE_X10_ID` is defined and `lowBattery` is false, `main()` reads VCC in mV, converts it to BCD via `ConversionTools::dec16ToHex`, and calls `x10encoder.RFXmeter(BATTERY_VOLTAGE_X10_ID, 0x00, value)`. The transmission is suppressed entirely when `lowBattery` is true.

### FUNC-X10-007
**Title:** Auxiliary analog sensor voltage in mV is BCD-encoded and transmitted via RFXmeter when battery is not low
**Status:** validated
**Dependencies:** FUNC-X10-001, FUNC-ANALOG-001
**Description:** When `ANALOG1_X10_ID` is defined and `lowBattery` is false, `main()` reads the raw analog sensor value, converts it to mV via `hal.ConvertAnalogValueToMv`, BCD-encodes via `ConversionTools::dec16ToHex`, and calls `x10encoder.RFXmeter(ANALOG1_X10_ID, 0x00, value)`. The transmission is suppressed when `lowBattery` is true.

### FUNC-BATTERY-001
**Title:** VCC computed from ATtiny84a internal 1.1 V reference
**Status:** validated
**Dependencies:** —
**Description:** `Hal::ComputeVccMv(known11refValue)` reads the raw ADC value of the internal 1.1 V reference via `GetRawInternal11Ref()` and returns `(uint16_t)(known11refValue * 1024.0 / rawAnalog11ref)`. The calibrated reference value is supplied as `INTERNAL_1v1` at build time.

### FUNC-BATTERY-002
**Title:** Battery voltage derived from VCC directly or resistor-divider on PA1
**Status:** validated
**Dependencies:** FUNC-BATTERY-001
**Description:** When `BATTERY_IS_VCC` is defined, `batteryVoltageInMv` equals `vccMv`. Otherwise, `main()` reads PA1 via `hal.GetRawBattery()` and calls `hal.ConvertAnalogValueToMv(rawBattery, vccMv)` to obtain the battery voltage in millivolts from the external resistor divider.

### FUNC-BATTERY-003
**Title:** Low-battery flag suppresses X10 meter transmissions and sets Oregon battery-low bit
**Status:** validated
**Dependencies:** FUNC-BATTERY-002, FUNC-OREGON-011, FUNC-X10-006, FUNC-X10-007
**Description:** When `batteryVoltageInMv < LOW_BATTERY_VOLTAGE`, `main()` sets `lowBattery = true`. Oregon frames receive `SetBatteryLow(true)`. All X10 RFXmeter transmissions guarded by `!lowBattery` (battery voltage and analog sensor) are suppressed entirely.

### FUNC-SENSOR-001
**Title:** BMx280 wraps BMP280 and BME280 behind a common interface at I2C address 0x76
**Status:** validated
**Dependencies:** —
**Description:** The `BMx280` class initialises the sensor at I2C address 0x76 and exposes `Begin()`, `GetTemperature()` (°C), `GetPressure()` (Pa), `GetHumidity()` (%), and `Shutdown()` (sleep mode). On non-AVR targets the methods are stubs returning 0.

### FUNC-SENSOR-002
**Title:** DS18B20 temperature read on 1-Wire PA3; raw value divided by 16 gives degrees Celsius
**Status:** validated
**Dependencies:** —
**Description:** When `USE_DS18B20` is defined on AVR, `main()` calls `ds18b20convert()` on PORTA bit 3, waits 1 s, then calls `ds18b20read()`. On `DS18B20_ERROR_OK`, the raw 16-bit fixed-point value is right-shifted by dividing by 16 to obtain temperature in °C as an integer.

### FUNC-SENSOR-003
**Title:** BH1750 light sensor read in ONE_TIME_HIGH_RES_MODE and reported as a Lacrosse type-5 luminosity frame transmitted twice
**Status:** validated
**Dependencies:** FUNC-LACROSSE-007
**Description:** When `USE_BH1750` is defined and `lowBattery` is false, `main()` initialises a `BH1750` at I2C address 0x23 in `ONE_TIME_HIGH_RES_MODE`, waits 150 ms, reads the lux value, sets it on a `LacrosseWS7000` encoder, and calls `Send()` twice to maximise reception reliability.

### FUNC-ANALOG-001
**Title:** Auxiliary analog input on PA0 read and converted to millivolts
**Status:** validated
**Dependencies:** FUNC-BATTERY-001
**Description:** When `ANALOG1_X10_ID` is defined and `lowBattery` is false, `main()` calls `hal.GetRawAnalogSensor()` (PA0 ADC reading) and converts it to millivolts via `hal.ConvertAnalogValueToMv(rawAdc, vccMv)`, where `vccMv` was computed by `ComputeVccMv(INTERNAL_1v1)`.
