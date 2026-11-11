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

---

### FUNC-DECODE-001
**Title:** X10Decoder rebuilds RFXmeter frames (address, raw value) from level/duration pulses
**Status:** validated
**Dependencies:** FUNC-X10-001, TECH-DECODE-001
**Description:** Class `X10Decoder` (`tinyDecoders/lib/X10Decoder/X10Decoder.h`, default constructor, depends only on `<stdint.h>`: no `Hal`, no AVR register) exposes `bool Feed(bool high, uint16_t durationUs)`, `uint8_t Address() const`, `uint32_t Value() const`. Each window is nominal ±30 %: preamble high 8960 then low 4500 µs, then 48 bits MSB-first, each high 560 then low 560 (`0`) or 1680 (`1`). `Feed` returns true on the low pulse of bit 48 if the 6 bytes `b0..b5` satisfy: `b1 == (~b0 & 0xF0) | (b0 & 0x0F)`, `b5 >> 4 == 0`, `b5 & 0x0F` equals the low nibble of the bitwise complement of the sum of all nibbles of `b0..b4` plus `b5 >> 4`. Then `Address()` is `b0` and `Value()` is `(b4 << 16) | (b2 << 8) | b3` until the next true. A failed check or a pulse of unexpected level or duration resets the decoder and that pulse is re-checked as a preamble high. Test: pulses from `OrdersToPulses()` of `x10rf` with 2 repeats sending `RFXmeter(address, 0, value)` make `Feed` return true exactly twice, each time with that address and value.

### FUNC-DECODE-002
**Title:** LacrosseDecoder rebuilds WS7000 frames (type, address) from level/duration pulses
**Status:** validated
**Dependencies:** FUNC-LACROSSE-001, FUNC-LACROSSE-002, FUNC-LACROSSE-003, FUNC-LACROSSE-008, FUNC-LACROSSE-009, TECH-DECODE-001
**Description:** Class `LacrosseDecoder` (`tinyDecoders/lib/LacrosseDecoder/LacrosseDecoder.h`, default constructor, `<stdint.h>` only) exposes `bool Feed(bool high, uint16_t durationUs)`, `uint8_t Type() const`, `uint8_t Address() const`. Windows are nominal ±30 %: bit `1` is high 400 then low 800 µs, `0` is high 800 then low 400. Sync is at least 6 zero bits then a one. Then nibbles, each 4 bits LSB-first plus a `1` separator. Nibble 1 is the type: 0, 1, 4, 5 give 3, 6, 10, 7 data nibbles after the address nibble, others fail. Then XOR and sum nibbles are checked per FUNC-LACROSSE-008. `Feed` returns true on the high pulse of the last separator; then `Type()` is the type and `Address()` is address-nibble bits 0–2. A pulse of unexpected level or duration, a `0` separator or a failed check resets the decoder; that pulse is re-checked as a sync high. Test: `OrdersToPulses()` of `LacrosseWS7000` sending each type twice returns true exactly twice, with that type and address.

### FUNC-DECODE-003
**Title:** LacrosseDecoder exposes decoded measurements in integer tenths and lux
**Status:** validated
**Dependencies:** FUNC-DECODE-002, FUNC-LACROSSE-003, FUNC-LACROSSE-004, FUNC-LACROSSE-005, FUNC-LACROSSE-006, FUNC-LACROSSE-007
**Description:** After `LacrosseDecoder::Feed` returns true, and until its next true, with `d1..dn` the data nibbles after the address nibble in transmission order: `int16_t TemperatureTenths() const` is `d3*100 + d2*10 + d1`, negated when address-nibble bit 3 is set (types 0, 1, 4); `uint16_t HumidityTenths() const` is `d6*100 + d5*10 + d4` (types 1, 4); `uint16_t PressureTenths() const` is hPa tenths `d9*1000 + d8*100 + d7*10 + d10 + 2000` (type 4); `uint16_t Lux() const` is `d3*100 + d2*10 + d1` (type 5, `d4..d7` ignored). A value the frame type does not carry reads 0. Test: loopback of `LacrosseWS7000` with −10.2 °C, 42.5 %, 1013.4 hPa and 1000 lux reads −102, 425, 10134 and 1000.

### FUNC-DECODE-004
**Title:** OregonDecoder rebuilds Oregon v3 frames (device ID, channel, rolling code, battery) from level/duration pulses
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002, FUNC-OREGON-003, FUNC-OREGON-004, FUNC-OREGON-009, FUNC-OREGON-010, FUNC-OREGON-011, FUNC-OREGON-012, TECH-DECODE-001
**Description:** Class `OregonDecoder` (`tinyDecoders/lib/OregonDecoder/OregonDecoder.h`, default constructor, `<stdint.h>` only) exposes `bool Feed(bool high, uint16_t durationUs)`, `uint16_t DeviceId() const`, `uint8_t Channel() const`, `uint8_t RollingCode() const`, `bool BatteryLow() const`. Windows: short 358–666 µs (512 ±30 %), long 717–1331 µs (1024 ±30 %). Each bit is a long pulse whose level is the bit value; between two bits comes either one long pulse of the opposite level (next bit equal) or two short pulses, opposite then same level (next bit inverted). Sync: a high long pulse starts a run of at least 16 one bits, ended by a zero bit. Bytes `b0..b10` are sent LSB-first; nibble `2i` is the low nibble of `b(i)`. `b0 b1` must be `0xEC 0x40`, `0x1D 0x20` or `0x5A 0x5D`; `b0` starts at the first zero bit, or at the last one bit of the run for `0x1D`. For these IDs, with `c` = 13, 16 or 20, nibbles `c` and `c+1` are the low and high nibbles of (sum of nibbles `0..c-1` − 0xA) & 0xFF. `Feed` returns true on the long pulse of the last checksum bit (frame bit 60, 72 or 88); then, until the next true, `DeviceId()` is `(b0 << 8) | b1`, `Channel()` is 1, 2 or 3 when `b2 >> 4` is 1, 2 or 4 (else 0), `RollingCode()` is `((b2 & 0x0F) << 4) | (b3 >> 4)`, `BatteryLow()` is bit 2 of `b3`. A pulse of unexpected level or duration, an unknown ID or a failed checksum resets the decoder; that pulse is re-checked as a sync high. Test: `OrdersToPulses()` of `OregonV3` sending each device twice (THN132N channel 1, code 0x3C, battery low; THGN123N channel 2, code 0xA5; BTHR918 channel 3, code 0x7E, battery low) returns true exactly twice, with that ID, channel, code and battery flag.

### FUNC-DECODE-005
**Title:** OregonDecoder exposes the measurements its device ID carries in integer tenths, % and hPa
**Status:** validated
**Dependencies:** FUNC-DECODE-004, FUNC-OREGON-005, FUNC-OREGON-006, FUNC-OREGON-007, FUNC-OREGON-008, FUNC-OREGON-012
**Description:** After `OregonDecoder::Feed` returns true, and until its next true, with `b0..b10` the frame bytes: `int16_t TemperatureTenths() const` is `(b5 >> 4)*100 + (b4 & 0x0F)*10 + (b4 >> 4)`, negated when bit 3 of `b5` is set (all three IDs); `uint8_t HumidityPercent() const` is `(b6 & 0x0F)*10 + (b6 >> 4)` (IDs `0x1D20`, `0x5A5D`); `uint16_t PressureHpa() const` is `(((b8 & 0x0F) << 8) | (b8 & 0xF0) | (b7 & 0x0F)) + 795` (ID `0x5A5D`). The forecast byte `b9` is ignored. A value the device ID does not carry reads 0. Test: loopback of `OregonV3` reads THN132N −10.2 °C as −102 (humidity and pressure 0); THGN123N 23.7 °C, 45 % as 237, 45 (pressure 0); BTHR918 21.5 °C, 60 %, 1013 hPa as 215, 60, 1013.

### FUNC-DECODE-006
**Title:** SourceTable keeps the last decoded record of each radio source, one page per source
**Status:** validated
**Dependencies:** FUNC-DECODE-001, FUNC-DECODE-003, FUNC-DECODE-005
**Description:** `tinyDecoders/lib/SourceTable/SourceTable.h` (`<stdint.h>` and the three decoder headers only, no dynamic allocation) defines `struct SourceRecord { Kind kind; uint8_t id; uint8_t fields; int16_t temperatureTenths; uint16_t humidityTenths; uint16_t pressureTenths; uint16_t lux; uint16_t millivolts; uint32_t lastSeenSeconds; }` with `enum Kind : uint8_t { Oregon, Lacrosse, LacrosseLux, X10 }` and `fields` a bitmask of `Temperature=1, Humidity=2, Pressure=4, Lux=8, Millivolts=16`; values whose bit is clear are 0. Class `SourceTable` (default constructor, empty, `kCapacity = 6`) exposes `void Update(const OregonDecoder&, uint32_t nowSeconds)`, `void Update(const LacrosseDecoder&, uint32_t nowSeconds)`, `void Update(const X10Decoder&, uint32_t nowSeconds)`, called after that decoder's `Feed` returned true, `uint8_t Count() const` and `const SourceRecord& At(uint8_t index) const`. A source is the pair `(kind, id)`. Mapping: Oregon gives `id = Channel()`, Temperature always, Humidity (`HumidityPercent()*10`) for IDs `0x1D20`, `0x5A5D`, Pressure (`PressureHpa()*10`) for `0x5A5D`. Lacrosse type 5 gives `LacrosseLux`, `id = Address()`, Lux; types 0, 1, 4 give `Lacrosse`, `id = Address()`, Temperature, plus Humidity (types 1, 4), plus Pressure (type 4), in tenths as decoded. X10 gives `id = Address()`, Millivolts as the BCD `Value()` read in decimal when `Value() <= 0x9999` and every nibble is ≤ 9, else no field. `Update` overwrites the whole record of a known source in place (`lastSeenSeconds = nowSeconds`), else appends it; when full, the record with the smallest `lastSeenSeconds` is replaced in place. `At` follows first-seen order. Test: loopback frames from each encoder update or append the expected records; a 7th source replaces the oldest; X10 value 0x2950 gives 2950 mV, 0x2A50 gives no field.

### FUNC-DECODE-007
**Title:** Receiver feeds pulses to the decoders, cycles one page per source every 4 s and redraws only changed characters
**Status:** validated
**Dependencies:** FUNC-DECODE-006, UI-DECODE-001, TECH-DECODE-002
**Description:** Class `Receiver` (`tinyDecoders/lib/Receiver/Receiver.h`, `explicit Receiver(ReceiverHal* hal)`, owns the three decoders and a `SourceTable`) exposes `void Poll()`. `Poll` pops every available pulse and feeds it to `X10Decoder`, `LacrosseDecoder`, `OregonDecoder`; each `Feed` returning true calls `SourceTable::Update` with `NowSeconds()`. Shown page, with `now = NowSeconds()`: empty table gives line 1 `Ore Lac X` and, on line 2, `now` in the UI-DECODE-001 age format ending at column 16; else `FormatPage(At(i), now - At(i).lastSeenSeconds)` with `i = (now / 4) % Count()`. Lines are recomputed only when `now` changed or a frame was decoded. Each `Poll` then calls `DrawChar` for at most one character differing from what was drawn (screen starts as spaces), the first in row-major order. Test: waiting page at 12 s; loopback Oregon and Lacrosse frames give their pages alternating every 4 s; no draw when nothing changed.

### FUNC-DECODE-008
**Title:** RflinkFormatter renders one decoded frame as an RFLink text line
**Status:** validated
**Dependencies:** FUNC-DECODE-001, FUNC-DECODE-003, FUNC-DECODE-005
**Description:** `tinyDecoders/lib/RflinkFormatter/RflinkFormatter.h` (`<stdint.h>` and the decoder headers only, no `printf`, no dynamic allocation) defines `const uint8_t RflinkLineSize = 64` and three overloads `uint8_t FormatRflink(const X10Decoder&|const LacrosseDecoder&|const OregonDecoder&, uint8_t counter, char line[RflinkLineSize])`, called after that decoder's `Feed` returned true; each writes the line plus `'\0'` (no `\r\n`) and returns its length. Line: `20;`, `counter` as 2 uppercase hex digits, `;`, name, `;`, then `KEY=value;` fields. `ID` is 4 uppercase hex digits; hex values are lowercase. `TEMP` is the absolute temperature in tenths as 4 hex digits, `| 0x8000` when negative. Oregon: name `Oregon Temp` (`0xEC40`), `Oregon TempHygro` (`0x1D20`), `Oregon BTHR` (`0x5A5D`); `ID = (RollingCode() << 8) | Channel()`; `TEMP`; `HUM` decimal (`0x1D20`, `0x5A5D`); `BARO` hPa as 4 hex digits (`0x5A5D`); `BAT=OK` or `BAT=LOW`. Lacrosse: name `LaCrosse WS7000`; `ID = (Type() << 8) | Address()`; `TEMP` (types 0, 1, 4); `HUM = (h+5)/10` decimal (types 1, 4); `BARO = (p+5)/10` as 4 hex digits (type 4); `LUX` as 4 hex digits (type 5). X10: name `X10Meter`; `ID = Address()`; `METER` = millivolts in decimal when `Value()` is valid BCD (FUNC-DECODE-006 rule), else the line ends after `ID`. Test: THGN123N ch 2, code 0xA5, 23.7 °C, 45 %, counter 0x0A gives `20;0A;Oregon TempHygro;ID=A502;TEMP=00ed;HUM=45;BAT=OK;`; THN132N ch 1, code 0x3C, −10.2 °C, battery low, counter 0 gives `20;00;Oregon Temp;ID=3C01;TEMP=8066;BAT=LOW;`; BTHR918 ch 3, code 0x7E, 21.5 °C, 60 %, 1013 hPa, battery low, counter 0xFF gives `20;FF;Oregon BTHR;ID=7E03;TEMP=00d7;HUM=60;BARO=03f5;BAT=LOW;`; Lacrosse type 4 address 3, −10.2 °C, 42.5 %, 1013.4 hPa, counter 1 gives `20;01;LaCrosse WS7000;ID=0403;TEMP=8066;HUM=43;BARO=03f5;`; Lacrosse type 5 address 2, 1000 lux, counter 1 gives `20;01;LaCrosse WS7000;ID=0502;LUX=03e8;`; X10 address 10, value 0x2950, counter 2 gives `20;02;X10Meter;ID=000A;METER=2950;`; value 0x2A50, counter 3 gives `20;03;X10Meter;ID=000A;`.

### FUNC-DECODE-009
**Title:** Receiver writes one RFLink line per decoded frame through ReceiverHal
**Status:** validated
**Dependencies:** FUNC-DECODE-007, FUNC-DECODE-008, TECH-DECODE-002
**Description:** When `ReceiverHal::HasLineOutput` is true (compile-time constant, so the formatter is not linked when false), each decoder `Feed` returning true in `Receiver::Poll` makes `Receiver`, after `SourceTable::Update`, call `FormatRflink` on that decoder and pass the line to `hal->WriteLine(line)`. `Receiver` keeps a `uint8_t` line counter starting at 0, passed to `FormatRflink` and incremented (wrapping) after each line. No deduplication: a frame sent twice gives two lines. The OLED behaviour of FUNC-DECODE-007 is unchanged. Test: loopback Oregon then Lacrosse frames, each sent twice, give 4 lines in reception order with counters `00` to `03` in `TestReceiverHal::Lines()`.
