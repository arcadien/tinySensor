# Functional Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->
<!-- Functional requirements state observable behaviour (radio frames, values, screen, serial lines); class names, signatures and memory layout belong in technical.md. -->

### FUNC-OREGON-001
**Title:** Oregon zero bit is radio high 512 µs, low 1024 µs, high 512 µs
**Status:** validated
**Dependencies:** —
**Description:** The Oregon encoder sends a zero bit as the radio output high for 512 µs, low for 1024 µs, then high for 512 µs. A zero bit has no other form on air.

### FUNC-OREGON-002
**Title:** Oregon one bit is radio low 512 µs, high 1024 µs, low 512 µs
**Status:** validated
**Dependencies:** —
**Description:** The Oregon encoder sends a one bit as the radio output low for 512 µs, high for 1024 µs, then low for 512 µs. A one bit has no other form on air.

### FUNC-OREGON-003
**Title:** Oregon bytes go out low nibble first
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** Every byte of an Oregon frame (preamble, payload and postamble) goes out low nibble (bits 0–3) first, then high nibble (bits 4–7), each bit as in FUNC-OREGON-001 and FUNC-OREGON-002.

### FUNC-OREGON-004
**Title:** Oregon frame is 24 one bits, the 11-byte payload, then 8 zero bits
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** Every Oregon frame starts with a 24 one-bit preamble (three `0xFF` bytes), then carries the 11-byte payload, and ends with an 8 zero-bit postamble (one `0x00` byte).

### FUNC-OREGON-005
**Title:** Positive temperature sent as tenths, units and tens digits in payload bytes 4–5
**Status:** validated
**Dependencies:** —
**Description:** A non-negative temperature is sent as three decimal digits: tenths in the high nibble of byte 4, units in the low nibble of byte 5, tens in the high nibble of byte 5 (frame nibbles 9, 10, 11, the sync nibble being nibble 0). This is the Oregon v2.1 layout read by RFLink (`Plugins/Plugin_048.c`, couin3, lines 733-734). The low nibble of byte 4 holds the flags (FUNC-OREGON-011).

### FUNC-OREGON-006
**Title:** Negative temperature sets the sign bit, bit 3 of byte 6, and sends the absolute value
**Status:** validated
**Dependencies:** FUNC-OREGON-005
**Description:** A negative temperature sets bit 3 of byte 6 (`0x08`, frame nibble 12, RFLink line 734) and sends its absolute value with the FUNC-OREGON-005 layout.

### FUNC-OREGON-007
**Title:** Humidity units in the high nibble of byte 6, tens in the low nibble of byte 7
**Status:** validated
**Dependencies:** —
**Description:** A humidity of 0–99 % is sent as two decimal digits: units in the high nibble of byte 6, tens in the low nibble of byte 7 (frame nibbles 13 and 14, RFLink line 737). The high nibble of byte 7 (comfort) stays 0.

### FUNC-OREGON-008
**Title:** Pressure sent as hPa − 795 in byte 8; weather forecast in byte 9
**Status:** validated
**Dependencies:** —
**Description:** Byte 8 (frame nibbles 16–17) holds the pressure in hPa minus 795, clamped to 0–255 (795–1050 hPa). Offset source: the user's RFLink gateway shows an early tinySensor (old firmware, this same layout) as Home Assistant `sensor.oregonbth_5a23` at 975 hPa while the receiver captured byte 8 = `0xB4` (180 + 795) in the same frame (2026-11-11); this restores that firmware's layout. Byte 9 holds a forecast code: 0x30 (rain, < 1000 hPa), 0x20 (cloudy, < 1010), 0x60 (partly cloudy, < 1025) or 0xC0 (sunny, ≥ 1025).

### FUNC-OREGON-009
**Title:** Channel 1–3 sent as a one-bit mask in the high nibble of byte 2; another value is ignored
**Status:** validated
**Dependencies:** —
**Description:** Channel 1, 2 or 3 sets the high nibble of byte 2 to 0x1, 0x2 or 0x4 (byte 2 = 0x10, 0x20 or 0x40). Any other channel value is ignored: byte 2 keeps its previous value.

### FUNC-OREGON-010
**Title:** Rolling code sent as byte 3
**Status:** validated
**Dependencies:** —
**Description:** The rolling code, any value 0x00–0xFF, is byte 3 (frame nibbles 6–7, RFLink `osdata[3]`, line 711). The low nibble of byte 2 (last device ID nibble) stays 0.

### FUNC-OREGON-011
**Title:** Battery-low flag is bit 2 of byte 4
**Status:** validated
**Dependencies:** —
**Description:** A low battery sets bit 2 of byte 4 (flags nibble 8, RFLink line 714); a good battery clears it. The other bits of byte 4 are unchanged.

### FUNC-OREGON-012
**Title:** Emulated Oregon device and checksum follow the measurements the frame carries
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** Payload bytes 0–1 are the device ID, the low nibble of byte 0 being the sync nibble 0xA. Temperature only gives `0xEA 0x4C` (THN132N); temperature and humidity give `0x1A 0x2D` (THGR228N); temperature, humidity and pressure give `0x5A 0x5D` (BTHR918) (RFLink lines 650, 696, 730). The checksum is (sum of all preceding nibbles − 0xA) & 0xFF, low nibble first: nibbles 13–14 (high nibble of byte 6, low nibble of byte 7) for THN132N, byte 8 for THGR228N, byte 10 for BTHR918 (RFLink lines 465, 654, 698).

### FUNC-OREGON-013
**Title:** An Oregon frame carries every measurement loaded since the encoder was created
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002
**Description:** The encoder starts with no measurement. Loading a temperature, a humidity or a pressure adds it to the measurements the encoder reports and sends; nothing removes one. This set chooses the device ID (FUNC-OREGON-012).

### FUNC-OREGON-014
**Title:** Full payload matches the Oregon v2.1 reference payloads decoded by RFLink
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002, FUNC-OREGON-003, FUNC-OREGON-004, FUNC-OREGON-005, FUNC-OREGON-006, FUNC-OREGON-007, FUNC-OREGON-008, FUNC-OREGON-009, FUNC-OREGON-010, FUNC-OREGON-011, FUNC-OREGON-012, FUNC-OREGON-013
**Description:** The 11-byte payload is: THN132N channel 1, rolling code 0x3C, battery low, −10.2 °C gives `EA4C103C2410D803000000`; THGR228N channel 3, rolling code 0xC4, 21.5 °C, 67 % gives `1A2D40C450217006390000`; BTHR918 channel 3, rolling code 0x7E, battery low, 21.5 °C, 60 %, 1013 hPa gives `5A5D407E54210006DA605F`. Read with the RFLink `Plugins/Plugin_048.c` (couin3) field formulas and the 795 hPa offset (FUNC-OREGON-008), these payloads give back their inputs.

### FUNC-OREGON-015
**Title:** A reused Oregon encoder carries no field value over from a previous frame
**Status:** validated
**Dependencies:** FUNC-OREGON-006, FUNC-OREGON-009, FUNC-OREGON-012, FUNC-OREGON-013, FUNC-OREGON-014
**Description:** When one encoder sends several frames, each new value fully replaces the previous one: a non-negative temperature clears the sign bit; a new channel replaces the previous channel; the checksum is computed again. The set of measurements stays cumulative (FUNC-OREGON-013). Test: one encoder sends the FUNC-OREGON-014 THN132N frame, then the THGR228N inputs, then the BTHR918 inputs; each payload equals its FUNC-OREGON-014 reference.

### FUNC-LACROSSE-001
**Title:** Lacrosse frame starts with 10 zero bits then a one start bit
**Status:** validated
**Dependencies:** —
**Description:** Every Lacrosse frame starts with exactly 10 zero bits, with nothing before them, followed by a single one bit before the first nibble.

### FUNC-LACROSSE-002
**Title:** Type nibble follows the measurements the frame carries; an unknown combination sends 0xF
**Status:** validated
**Dependencies:** —
**Description:** The first nibble is the sensor type: temperature only gives 0x0, temperature and humidity 0x1, temperature, humidity and pressure 0x4, luminosity only 0x5. Any other combination gives 0xF.

### FUNC-LACROSSE-003
**Title:** Address 0–7 in bits 0–2 of the second nibble; negative temperature sign in bit 3
**Status:** validated
**Dependencies:** —
**Description:** The second nibble holds the 3-bit sensor address in bits 0–2. Bit 3 is 1 when the frame carries a negative temperature, else 0.

### FUNC-LACROSSE-004
**Title:** Temperature clamped to ±99 °C and sent as tenths, units and tens nibbles
**Status:** validated
**Dependencies:** —
**Description:** The temperature is clamped to [−99, +99] °C and its absolute value sent as three nibbles in order: tenths, units, tens. The sign is sent per FUNC-LACROSSE-003.

### FUNC-LACROSSE-005
**Title:** Humidity clamped to 99.9 % and sent as tenths, units and tens nibbles
**Status:** validated
**Dependencies:** —
**Description:** A humidity of 100 % or more is sent as 99.9 %. The value is sent as three nibbles in order: tenths, units, tens.

### FUNC-LACROSSE-006
**Title:** Pressure clamped to 850–1100 hPa, minus 200, sent as units, tens, hundreds and tenths nibbles
**Status:** validated
**Dependencies:** —
**Description:** The pressure is clamped to [850, 1100] hPa, then 200 is subtracted. The result is sent as four nibbles in order: units, tens, hundreds, tenths.

### FUNC-LACROSSE-007
**Title:** Luminosity sent as 7 nibbles in a type 5 frame
**Status:** validated
**Dependencies:** —
**Description:** A luminosity frame (type 0x5, FUNC-LACROSSE-002) carries the units, tens and hundreds digits of the lux value followed by four zero nibbles: 7 data nibbles.

### FUNC-LACROSSE-008
**Title:** Frame ends with an XOR check nibble then a sum check nibble starting at 5
**Status:** validated
**Dependencies:** FUNC-LACROSSE-001, FUNC-LACROSSE-002, FUNC-LACROSSE-003, FUNC-LACROSSE-004, FUNC-LACROSSE-005, FUNC-LACROSSE-006, FUNC-LACROSSE-007
**Description:** After the data nibbles come two check nibbles. The first is the XOR of every nibble sent before it, on 4 bits. The second is 5 plus every nibble sent before it, the XOR nibble included, on 4 bits.

### FUNC-LACROSSE-009
**Title:** Every nibble is followed by a one bit separator
**Status:** validated
**Dependencies:** —
**Description:** Every nibble of the frame (type, address, data and check nibbles) is sent as its four bits followed by a one bit.

### FUNC-X10-001
**Title:** RFXmeter frame: address, address check, 24-bit value, packet type and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** An RFXmeter frame is 6 bytes: byte 0 is the address; byte 1 is the address with its high nibble inverted; bytes 2–4 carry the 24-bit value (0 when the value does not fit in 24 bits); the high nibble of byte 5 is the packet type; the low nibble of byte 5 is the inverted sum of all the nibbles before it. It is sent per FUNC-X10-005.

### FUNC-X10-002
**Title:** RFXsensor frame: address and type, check, value, status and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** An RFXsensor frame is 4 bytes: byte 0 is the address shifted left by 2 with the sensor type in bits 0–1 (temperature 0, voltage 2, A/D 1, message 3); byte 1 is byte 0 with its high nibble inverted; byte 2 is the value; the high nibble of byte 3 is the packet-type status; the low nibble of byte 3 is the inverted sum of all the nibbles before it. It is sent per FUNC-X10-005.

### FUNC-X10-003
**Title:** X10 switch frame: house code, scattered unit code and byte complements
**Status:** validated
**Dependencies:** —
**Description:** An X10 switch frame is 4 bytes. House code `a`–`p` gives a 4-bit value from the X10 lookup table in the high nibble of byte 0. The unit code bits are spread over bit 2 of byte 0 and bits 3, 4 and 6 of byte 2. Byte 1 is the complement of byte 0 and byte 3 the complement of byte 2. It is sent per FUNC-X10-005.

### FUNC-X10-004
**Title:** X10 security frame with a single parity bit
**Status:** validated
**Dependencies:** —
**Description:** An X10 security frame is 6 bytes: byte 0 is the address; byte 1 is the low nibble of the inverted address plus the high nibble of the address; byte 2 is the command and byte 3 its complement; byte 4 is the id; bit 7 of byte 5 is the parity of the frame, the XOR of all six bytes folded down to one bit. It is sent per FUNC-X10-005.

### FUNC-X10-005
**Title:** Each X10 frame is repeated with a 40 ms gap, the LED on throughout
**Status:** validated
**Dependencies:** —
**Description:** An X10 frame is sent the configured number of times. Each copy is the preamble burst, the frame bits most significant first and a stop bit, followed by a 40 ms silence. The LED is lit from before the first copy until after the last.

### FUNC-X10-006
**Title:** Battery voltage in mV is sent as BCD in an RFXmeter frame while the battery is not low
**Status:** validated
**Dependencies:** FUNC-X10-001, FUNC-BATTERY-001, FUNC-BATTERY-002
**Description:** When the build sets a battery meter address (CONF-BUILD-008) and the battery is not low, every cycle sends the battery voltage in mV, one decimal digit per nibble (3960 mV gives 0x3960), in an RFXmeter frame with that address and packet type 0. Nothing is sent while the battery is low.

### FUNC-X10-007
**Title:** Auxiliary analog voltage in mV is sent as BCD in an RFXmeter frame while the battery is not low
**Status:** validated
**Dependencies:** FUNC-X10-001, FUNC-ANALOG-001
**Description:** When the build sets an analog meter address (CONF-BUILD-009) and the battery is not low, every cycle sends the auxiliary analog voltage in mV (FUNC-ANALOG-001), one decimal digit per nibble, in an RFXmeter frame with that address and packet type 0. Nothing is sent while the battery is low.

### FUNC-BATTERY-001
**Title:** VCC measured against the internal 1.1 V reference
**Status:** validated
**Dependencies:** —
**Description:** VCC in mV is the board's calibrated 1.1 V reference value (mV, CONF-BUILD-005) × 1024 divided by the ADC reading of the internal 1.1 V reference, truncated to an integer.

### FUNC-BATTERY-002
**Title:** Battery voltage is VCC, or the PA1 divider reading multiplied by the board's divider ratio
**Status:** validated
**Dependencies:** FUNC-BATTERY-001, CONF-BUILD-012
**Description:** On boards where the battery powers the chip directly, the battery voltage is the measured VCC. On other boards, it is the PA1 voltage multiplied by the divider ratio set at build time, which is 1 when not set. This scaled voltage drives the low-battery decision and is the value the X10 battery meter reports. Acceptance: with a ratio of 2, the battery meter reports twice the PA1 voltage; without a ratio, it reports the PA1 voltage unchanged.

### FUNC-BATTERY-003
**Title:** Low battery mutes the X10 meters and sets the Oregon battery-low flag
**Status:** validated
**Dependencies:** FUNC-BATTERY-002, FUNC-OREGON-011, FUNC-X10-006, FUNC-X10-007
**Description:** The battery is low when its voltage is below the build's threshold (CONF-BUILD-007). Then Oregon frames carry the battery-low flag (FUNC-OREGON-011), and the battery and analog X10 meter frames are not sent.

### FUNC-SENSOR-001
**Title:** Temperature, humidity and pressure read from a BME280 or BMP280 at I2C address 0x76
**Status:** validated
**Dependencies:** —
**Description:** On a build with a BME280 or BMP280 (CONF-BUILD-002), each cycle reads the temperature (°C), the pressure and, on a BME280, the humidity (%) from the sensor at I2C address 0x76, then puts the sensor to sleep. Wrapper details: TECH-SENSOR-002.

### FUNC-SENSOR-002
**Title:** DS18B20 temperature read on the 1-Wire bus on PA3, in whole °C
**Status:** validated
**Dependencies:** —
**Description:** On a build with a DS18B20, each cycle starts a conversion on the 1-Wire bus on PA3, waits 1 s and reads the result. On a successful read, the temperature in whole °C is the raw 1/16 °C value divided by 16. Wrapper details: TECH-SENSOR-001.

### FUNC-SENSOR-003
**Title:** BH1750 luminosity sent twice as a Lacrosse luminosity frame
**Status:** validated
**Dependencies:** FUNC-LACROSSE-007
**Description:** On a build with a BH1750 and while the battery is not low, each cycle starts a one-time high-resolution measurement on the sensor at I2C address 0x23, waits 150 ms, reads the lux value and sends it twice in a Lacrosse luminosity frame (FUNC-LACROSSE-007), for better reception.

### FUNC-ANALOG-001
**Title:** Auxiliary analog input on PA0 measured in millivolts
**Status:** validated
**Dependencies:** FUNC-BATTERY-001
**Description:** When the build sets an analog meter address (CONF-BUILD-009) and the battery is not low, each cycle reads PA0 with the ADC and converts the reading to mV using the VCC measured per FUNC-BATTERY-001.

### FUNC-MAIN-001
**Title:** Environment frame sent twice per cycle, separated by a silence at least one frame long
**Status:** validated
**Dependencies:** FUNC-OREGON-014, FUNC-LACROSSE-002, PLAT-POWER-004, TECH-MAIN-001
**Description:** In every cycle where the sensor sends an environment frame (Oregon or Lacrosse, whichever the build uses), it sends that frame twice, and both copies are identical. Between the end of the first copy and the start of the second, the radio stays low for at least the length of one frame: 246 ms for Oregon and 98 ms for Lacrosse. This matches commercial Oregon sensors and makes up for single frames that RFLink missed. The X10 meter frames, the luminosity frame, and cycles that send no environment frame stay as they are. Acceptance: in one cycle on an Oregon build and one on a Lacrosse build, the radio output holds two identical environment frames. Between them there is a silence of at least 246 ms (Oregon) or 98 ms (Lacrosse), and the radio is never high during it. A build with no environment protocol sends no extra frame.

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
**Title:** OregonDecoder rebuilds Oregon v2.1 frames (device ID, channel, rolling code, battery) from level/duration pulses
**Status:** validated
**Dependencies:** FUNC-OREGON-001, FUNC-OREGON-002, FUNC-OREGON-003, FUNC-OREGON-004, FUNC-OREGON-009, FUNC-OREGON-010, FUNC-OREGON-011, FUNC-OREGON-012, TECH-DECODE-001
**Description:** Class `OregonDecoder` (`tinyDecoders/lib/OregonDecoder/OregonDecoder.h`, default constructor, `<stdint.h>` only) exposes `bool Feed(bool high, uint16_t durationUs)`, `uint16_t DeviceId() const`, `uint8_t Channel() const`, `uint8_t RollingCode() const`, `bool BatteryLow() const`. Windows: short 150–731 µs, long 732–1199 µs (origin: RFLink `OregonDecoderV2`, `Plugins/Plugin_048.c` (couin3) lines 322-324, threshold moved from 600 µs). 732 µs is the midpoint of the nominal half-bit (488 µs) and bit (976 µs) at 1024 Hz: a weak commercial THGN132N on the RXB6 (bench, 2026-11-12) gives highs ~80 µs short and lows ~80 µs long (short lows up to 649 µs, long highs from 800 µs), which the 600 µs threshold rejected (5 frames decoded vs 203 by an RFLink gateway over 2 h). Each bit is a long pulse whose level is the bit value; between two bits comes either one long pulse of the opposite level (next bit equal) or two short pulses, opposite then same level (next bit inverted). Sync: a high long pulse starts a run of at least 12 one bits (RFLink's 24 long pulses, line 333), ended by a zero bit where `b0` starts (its low nibble is the sync nibble 0xA). Bytes `b0..b10` are LSB-first; nibble `2i` is the low nibble of `b(i)`. `b0 b1` must be `0xEA 0x4C` (c = 13), `0x1A 0x2D` or `0x1A 0x3D` (c = 16), or `0x5A 0x5D` (c = 20); nibbles `c` and `c+1` are the low and high nibbles of (sum of nibbles `0..c-1` − 0xA) & 0xFF (lines 465, 654, 698); later nibbles are ignored. `Feed` returns true on the long pulse of the last checksum bit (frame bit 60, 72 or 88); then, until the next true, `DeviceId()` is `(b0 << 8) | b1`, `Channel()` is 1, 2 or 3 when `b2 >> 4` is 1, 2 or 4 (else 0), `RollingCode()` is `b3`, `BatteryLow()` is `(b4 & 0x0C) != 0` (lines 711-714). A pulse of unexpected level or duration, an unknown ID or a failed checksum resets the decoder; that pulse is re-checked as a sync high. Test: `OrdersToPulses()` of `OregonV3` sending each FUNC-OREGON-014 device twice returns true exactly twice with its ID, channel, code and battery flag; bit pulses of the RFLink captures `EA4C20725C21D083`, `EA4C101360193023`, `1A2D40C4512170463E`, `1A2D1072512080E73F` (lines 644-645, 685-686) give channel/code/battery 2/0x72/low, 1/0x13/ok, 3/0xC4/ok, 1/0x72/ok; a `0x1A3D` copy with recomputed checksum is accepted; 11 sync ones or one flipped data bit are rejected; a valid frame with short lows 620 µs, short highs 400 µs, long highs 880 µs, long lows 1060 µs decodes; window bounds 149/150, 731/732, 1199/1200 µs.

### FUNC-DECODE-005
**Title:** OregonDecoder exposes the measurements its device ID carries in integer tenths, % and hPa
**Status:** validated
**Dependencies:** FUNC-DECODE-004, FUNC-OREGON-005, FUNC-OREGON-006, FUNC-OREGON-007, FUNC-OREGON-008, FUNC-OREGON-012
**Description:** After `OregonDecoder::Feed` returns true, and until its next true, with `b0..b10` the frame bytes: `int16_t TemperatureTenths() const` is `(b5 >> 4)*100 + (b5 & 0x0F)*10 + (b4 >> 4)`, negated when bit 3 of `b6` is set (all IDs); `uint8_t HumidityPercent() const` is `(b7 & 0x0F)*10 + (b6 >> 4)` (IDs `0x1A2D`, `0x1A3D`, `0x5A5D`); `uint16_t PressureHpa() const` is `b8 + 795` (ID `0x5A5D`) (RFLink `Plugins/Plugin_048.c` (couin3) lines 733-751; offset 795 per FUNC-OREGON-008). Forecast and comfort nibbles are ignored. A value the device ID does not carry reads 0. Test: loopback of `OregonV3` reads THN132N −10.2 °C as −102 (humidity and pressure 0); THGR228N 21.5 °C, 67 % as 215, 67 (pressure 0); BTHR918 21.5 °C, 60 %, 1013 hPa as 215, 60, 1013; the FUNC-DECODE-004 RFLink captures read 215; 196; 215 and 67; 205 and 78.

### FUNC-DECODE-006
**Title:** SourceTable keeps the last decoded record of each radio source, one page per source
**Status:** validated
**Dependencies:** FUNC-DECODE-001, FUNC-DECODE-003, FUNC-DECODE-005
**Description:** `tinyDecoders/lib/SourceTable/SourceTable.h` (`<stdint.h>` and the three decoder headers only, no dynamic allocation) defines `struct SourceRecord { Kind kind; uint8_t id; uint8_t code; uint8_t fields; int16_t temperatureTenths; uint16_t humidityTenths; uint16_t pressureTenths; uint16_t lux; uint16_t millivolts; uint32_t lastSeenSeconds; }` (18 B) with `enum Kind : uint8_t { Oregon, Lacrosse, LacrosseLux, X10 }` and `fields` a bitmask of `Temperature=1, Humidity=2, Pressure=4, Lux=8, Millivolts=16`; values whose bit is clear are 0. Class `SourceTable` (default constructor, empty, `kCapacity` = 16 when `AVR` is defined, else 8) exposes `void Update(const OregonDecoder&, uint32_t nowSeconds)`, `void Update(const LacrosseDecoder&, uint32_t nowSeconds)`, `void Update(const X10Decoder&, uint32_t nowSeconds)`, called after that decoder's `Feed` returned true, `uint8_t Count() const` and `const SourceRecord& At(uint8_t index) const`. A source is the triple `(kind, id, code)`. Mapping: Oregon gives `id = Channel()`, `code = RollingCode()`, Temperature always, Humidity (`HumidityPercent()*10`) for IDs `0x1A2D`, `0x1A3D`, `0x5A5D`, Pressure (`PressureHpa()*10`) for `0x5A5D`. Lacrosse and X10 give `code = 0`. Lacrosse type 5 gives `LacrosseLux`, `id = Address()`, Lux; types 0, 1, 4 give `Lacrosse`, `id = Address()`, Temperature, plus Humidity (types 1, 4), plus Pressure (type 4), in tenths as decoded. X10 gives `id = Address()`, Millivolts as the BCD `Value()` read in decimal when `Value() <= 0x9999` and every nibble is ≤ 9, else no field. `Update` overwrites the whole record of a known source in place (`lastSeenSeconds = nowSeconds`), else appends it; when full, the record with the smallest `lastSeenSeconds` is replaced in place. `At` follows first-seen order. Table RAM on the Pro Mini and the Pro Micro: 16 × 18 = 288 B. Test (native, capacity 8): loopback frames from each encoder update or append the expected records; two Oregon frames on channel 2 with codes 0xB8 and 0x36 give two records; a 9th source replaces the oldest; X10 value 0x2950 gives 2950 mV, 0x2A50 gives no field.

### FUNC-DECODE-007
**Title:** Receiver feeds pulses to the decoders, cycles one page per source every 10 s and redraws only changed cells
**Status:** validated
**Dependencies:** FUNC-DECODE-006, FUNC-DECODE-010, UI-DECODE-001, TECH-DECODE-002
**Description:** Class `Receiver` (`tinyDecoders/lib/Receiver/Receiver.h`, `explicit Receiver(ReceiverHal* hal)`, owns the three decoders and a `SourceTable`) exposes `void Poll()`. `Poll` pops every available pulse and feeds it to `X10Decoder`, `LacrosseDecoder`, `OregonDecoder`; each `Feed` returning true calls `SourceTable::Update` with `NowSeconds()`. The page is recomputed only when `now = NowSeconds()` changed or a frame was decoded; recomputing first calls `SourceTable::Expire(now)`. Only while the table is empty (at start, or once `Expire` removed the last source), the waiting page is shown: `Waiting for data`, vertically centred (`DrawChar` page 1, cols 0–15, pixel rows 8–23); it is never one of the rotated pages. Else `FormatPage(At(i), (now / 2) % 2 == 1, big, top, bottom)` with `i = (now / 10) % Count()`. A source page is 6 big cells (`DrawBigChar` col 0–5) and a side column of 2 × 4 characters (`DrawChar` col 12–15, `top` at page 0, `bottom` at page 2); its 14 cells cover the whole screen. The screen is blank at start, so the first waiting page draws only its 16 text cells. Entering a source page draws all 14 of its cells, spaces included. Entering the waiting page from a source page first draws the 14 source cells as spaces (blanking pages 0–3), then the 16 text cells. Each `Poll` draws at most one pending cell (differing from what was drawn, or forced by a switch), the first in the order big 0–5, `top` 12–15, `bottom` 12–15, then waiting text 0–15. Test: waiting page at 12 s (`Row(1)` is `Waiting for data`, 16 draws); the first decoded frame then draws all 14 source cells; loopback Oregon and Lacrosse frames give their pages alternating every 10 s; a BTHR918 alone shows its humidity at 0–1 s and its pressure at 2–3 s in `bottom`; no draw when nothing changed; a single source last seen 1800 s ago gives the waiting page, after 14 blank source cells (`Big()` and `Row(0)`, `Row(2)` cols 12–15 are spaces).

### FUNC-DECODE-008
**Title:** RflinkFormatter renders one decoded frame as an RFLink text line
**Status:** validated
**Dependencies:** FUNC-DECODE-001, FUNC-DECODE-003, FUNC-DECODE-005
**Description:** `tinyDecoders/lib/RflinkFormatter/RflinkFormatter.h` (`<stdint.h>` and the decoder headers only, no `printf`, no dynamic allocation) defines `const uint8_t RflinkLineSize = 64` and three overloads `uint8_t FormatRflink(const X10Decoder&|const LacrosseDecoder&|const OregonDecoder&, uint8_t counter, char line[RflinkLineSize])`, called after that decoder's `Feed` returned true; each writes the line plus `'\0'` (no `\r\n`) and returns its length. Line: `20;`, `counter` as 2 uppercase hex digits, `;`, name, `;`, then `KEY=value;` fields. `ID` is 4 uppercase hex digits; hex values are lowercase. `TEMP` is the absolute temperature in tenths as 4 hex digits, `| 0x8000` when negative. Oregon: name `Oregon Temp` (`0xEA4C`), `Oregon TempHygro` (`0x1A2D`, `0x1A3D`), `Oregon BTHR` (`0x5A5D`); `ID = (RollingCode() << 8) | Channel()`; `TEMP`; `HUM` decimal (`0x1A2D`, `0x1A3D`, `0x5A5D`); `BARO` hPa as 4 hex digits (`0x5A5D`); `BAT=OK` or `BAT=LOW`. Lacrosse: name `LaCrosse WS7000`; `ID = (Type() << 8) | Address()`; `TEMP` (types 0, 1, 4); `HUM = (h+5)/10` decimal (types 1, 4); `BARO = (p+5)/10` as 4 hex digits (type 4); `LUX` as 4 hex digits (type 5). X10: name `X10Meter`; `ID = Address()`; `METER` = millivolts in decimal when `Value()` is valid BCD (FUNC-DECODE-006 rule), else the line ends after `ID`. Test: THGR228N ch 3, code 0xC4, 21.5 °C, 67 %, counter 0x0A gives `20;0A;Oregon TempHygro;ID=C403;TEMP=00d7;HUM=67;BAT=OK;`; THN132N ch 1, code 0x3C, −10.2 °C, battery low, counter 0 gives `20;00;Oregon Temp;ID=3C01;TEMP=8066;BAT=LOW;`; BTHR918 ch 3, code 0x7E, 21.5 °C, 60 %, 1013 hPa, battery low, counter 0xFF gives `20;FF;Oregon BTHR;ID=7E03;TEMP=00d7;HUM=60;BARO=03f5;BAT=LOW;`; Lacrosse type 4 address 3, −10.2 °C, 42.5 %, 1013.4 hPa, counter 1 gives `20;01;LaCrosse WS7000;ID=0403;TEMP=8066;HUM=43;BARO=03f5;`; Lacrosse type 5 address 2, 1000 lux, counter 1 gives `20;01;LaCrosse WS7000;ID=0502;LUX=03e8;`; X10 address 10, value 0x2950, counter 2 gives `20;02;X10Meter;ID=000A;METER=2950;`; value 0x2A50, counter 3 gives `20;03;X10Meter;ID=000A;`.

### FUNC-DECODE-009
**Title:** Receiver writes one RFLink line per decoded frame through ReceiverHal
**Status:** validated
**Dependencies:** FUNC-DECODE-007, FUNC-DECODE-008, TECH-DECODE-002
**Description:** When `ReceiverHal::HasLineOutput` is true (compile-time constant, so the formatter is not linked when false), each decoder `Feed` returning true in `Receiver::Poll` makes `Receiver`, after `SourceTable::Update`, call `FormatRflink` on that decoder and pass the line to `hal->WriteLine(line)`. `Receiver` keeps a `uint8_t` line counter starting at 0, passed to `FormatRflink` and incremented (wrapping) after each line. No deduplication: a frame sent twice gives two lines. The OLED behaviour of FUNC-DECODE-007 is unchanged. Test: loopback Oregon then Lacrosse frames, each sent twice, give 4 lines in reception order with counters `00` to `03` in `TestReceiverHal::Lines()`.

### FUNC-DECODE-010
**Title:** SourceTable removes a source not received for 30 minutes
**Status:** validated
**Dependencies:** FUNC-DECODE-006
**Description:** `SourceTable` (`tinyDecoders/lib/SourceTable/SourceTable.h`) exposes `void Expire(uint32_t nowSeconds)`, which removes every record with `nowSeconds - lastSeenSeconds >= 1800`. The remaining records keep their first-seen order in `At` and `Count()` drops by the number removed; nothing else removes a record (the FUNC-DECODE-006 replacement when full is unchanged). A removed source received again is appended as a new source. `Receiver` calls it before each page computation (FUNC-DECODE-007), so an expired source leaves the rotation and an empty table shows the waiting page. Test (native): a source seen at 100 is kept by `Expire(1899)` and removed by `Expire(1900)`; sources seen at 0, 500 and 1000 leave, after `Expire(1800)`, the second then the third; a source first seen at 0 and updated at 1000 survives `Expire(1800)`; `Expire` on an empty table keeps `Count()` at 0.
