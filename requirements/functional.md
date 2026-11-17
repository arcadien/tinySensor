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
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002)
**Description:** Every byte of an Oregon frame (preamble, payload and postamble) goes out low nibble (bits 0–3) first, then high nibble (bits 4–7), each bit as in [FUNC-OREGON-001](#func-oregon-001) and [FUNC-OREGON-002](#func-oregon-002).

### FUNC-OREGON-004
**Title:** Oregon frame is 24 one bits, the 11-byte payload, then 8 zero bits
**Status:** validated
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002)
**Description:** Every Oregon frame starts with a 24 one-bit preamble (three `0xFF` bytes), then carries the 11-byte payload, and ends with an 8 zero-bit postamble (one `0x00` byte).

### FUNC-OREGON-005
**Title:** Positive temperature rounded to the tenth, clamped to 99.9 °C, sent as tenths, units and tens digits in payload bytes 4–5
**Status:** validated
**Dependencies:** —
**Description:** A non-negative temperature is rounded to the nearest tenth (halves up), then clamped to 99.9 °C, and sent as three decimal digits: tenths in the high nibble of byte 4, units in the low nibble of byte 5, tens in the high nibble of byte 5 (frame nibbles 9, 10, 11, the sync nibble being nibble 0). Rounding comes before the split, so each digit is 0–9: 21.97 °C sends 22.0, 21.94 °C sends 21.9, 99.96 °C and 120 °C send 99.9. This is the Oregon v2.1 layout read by RFLink (`Plugins/Plugin_048.c`, couin3, lines 733-734). The low nibble of byte 4 holds the flags ([FUNC-OREGON-011](#func-oregon-011)).

### FUNC-OREGON-006
**Title:** Negative temperature sets the sign bit, bit 3 of byte 6, and sends the absolute value
**Status:** validated
**Dependencies:** [FUNC-OREGON-005](#func-oregon-005)
**Description:** A temperature that is negative after rounding to the nearest tenth sets bit 3 of byte 6 (`0x08`, frame nibble 12, RFLink line 734). It sends its absolute value with the [FUNC-OREGON-005](#func-oregon-005) rounding, clamp and layout: −10.25 °C sends −10.3, −120 °C sends −99.9, and −0.04 °C sends 0.0 with the sign bit clear.

### FUNC-OREGON-007
**Title:** Humidity rounded to whole %, clamped to 0–99 %, units in the high nibble of byte 6, tens in the low nibble of byte 7
**Status:** validated
**Dependencies:** —
**Description:** The humidity is rounded to the nearest whole percent (halves up), then clamped to 0–99 %, and sent as two decimal digits: units in the high nibble of byte 6, tens in the low nibble of byte 7 (frame nibbles 13 and 14, RFLink line 737). 67.4 % sends 67, 67.5 % sends 68, 99.6 % and 100 % send 99. The high nibble of byte 7 (comfort) stays 0.

### FUNC-OREGON-008
**Title:** Pressure rounded to whole hPa, sent as hPa − 795 in byte 8; weather forecast in byte 9
**Status:** validated
**Dependencies:** —
**Description:** Byte 8 (frame nibbles 16–17) holds the pressure rounded to the nearest whole hPa (halves up), minus 795, clamped to 0–255 (795–1050 hPa): 1013.4 hPa sends 218, 1013.5 hPa sends 219. Offset source: the user's RFLink gateway shows an early tinySensor (old firmware, this same layout) as Home Assistant `sensor.oregonbth_5a23` at 975 hPa while the receiver captured byte 8 = `0xB4` (180 + 795) in the same frame (2026-11-11); this restores that firmware's layout. Byte 9 holds a forecast code chosen from the rounded pressure: 0x30 (rain, < 1000 hPa), 0x20 (cloudy, < 1010), 0x60 (partly cloudy, < 1025) or 0xC0 (sunny, ≥ 1025).

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
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002)
**Description:** Payload bytes 0–1 are the device ID, the low nibble of byte 0 being the sync nibble 0xA. Temperature only gives `0xEA 0x4C` (THN132N); temperature and humidity give `0x1A 0x2D` (THGR228N); temperature, humidity and pressure give `0x5A 0x5D` (BTHR918) (RFLink lines 650, 696, 730). The checksum is (sum of all preceding nibbles − 0xA) & 0xFF, low nibble first: nibbles 13–14 (high nibble of byte 6, low nibble of byte 7) for THN132N, byte 8 for THGR228N, byte 10 for BTHR918 (RFLink lines 465, 654, 698).

### FUNC-OREGON-013
**Title:** An Oregon frame carries every measurement loaded since the encoder was created
**Status:** validated
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002)
**Description:** The encoder starts with no measurement. Loading a temperature, a humidity or a pressure adds it to the measurements the encoder reports and sends; nothing removes one. This set chooses the device ID ([FUNC-OREGON-012](#func-oregon-012)).

### FUNC-OREGON-014
**Title:** Full payload matches the Oregon v2.1 reference payloads decoded by RFLink
**Status:** validated
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002), [FUNC-OREGON-003](#func-oregon-003), [FUNC-OREGON-004](#func-oregon-004), [FUNC-OREGON-005](#func-oregon-005), [FUNC-OREGON-006](#func-oregon-006), [FUNC-OREGON-007](#func-oregon-007), [FUNC-OREGON-008](#func-oregon-008), [FUNC-OREGON-009](#func-oregon-009), [FUNC-OREGON-010](#func-oregon-010), [FUNC-OREGON-011](#func-oregon-011), [FUNC-OREGON-012](#func-oregon-012), [FUNC-OREGON-013](#func-oregon-013)
**Description:** The 11-byte payload is: THN132N channel 1, rolling code 0x3C, battery low, −10.2 °C gives `EA4C103C2410D803000000`; THGR228N channel 3, rolling code 0xC4, 21.5 °C, 67 % gives `1A2D40C450217006390000`; BTHR918 channel 3, rolling code 0x7E, battery low, 21.5 °C, 60 %, 1013 hPa gives `5A5D407E54210006DA605F`. Read with the RFLink `Plugins/Plugin_048.c` (couin3) field formulas and the 795 hPa offset ([FUNC-OREGON-008](#func-oregon-008)), these payloads give back their inputs.

### FUNC-OREGON-015
**Title:** A reused Oregon encoder carries no field value over from a previous frame
**Status:** validated
**Dependencies:** [FUNC-OREGON-006](#func-oregon-006), [FUNC-OREGON-009](#func-oregon-009), [FUNC-OREGON-012](#func-oregon-012), [FUNC-OREGON-013](#func-oregon-013), [FUNC-OREGON-014](#func-oregon-014)
**Description:** When one encoder sends several frames, each new value fully replaces the previous one: a non-negative temperature clears the sign bit; a new channel replaces the previous channel; the checksum is computed again. The set of measurements stays cumulative ([FUNC-OREGON-013](#func-oregon-013)). Test: one encoder sends the [FUNC-OREGON-014](#func-oregon-014) THN132N frame, then the THGR228N inputs, then the BTHR918 inputs; each payload equals its [FUNC-OREGON-014](#func-oregon-014) reference.

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
**Description:** The temperature is clamped to [−99, +99] °C and its absolute value sent as three nibbles in order: tenths, units, tens. The sign is sent per [FUNC-LACROSSE-003](#func-lacrosse-003).

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
**Description:** A luminosity frame (type 0x5, [FUNC-LACROSSE-002](#func-lacrosse-002)) carries the units, tens and hundreds digits of the lux value followed by four zero nibbles: 7 data nibbles.

### FUNC-LACROSSE-008
**Title:** Frame ends with an XOR check nibble then a sum check nibble starting at 5
**Status:** validated
**Dependencies:** [FUNC-LACROSSE-001](#func-lacrosse-001), [FUNC-LACROSSE-002](#func-lacrosse-002), [FUNC-LACROSSE-003](#func-lacrosse-003), [FUNC-LACROSSE-004](#func-lacrosse-004), [FUNC-LACROSSE-005](#func-lacrosse-005), [FUNC-LACROSSE-006](#func-lacrosse-006), [FUNC-LACROSSE-007](#func-lacrosse-007)
**Description:** After the data nibbles come two check nibbles. The first is the XOR of every nibble sent before it, on 4 bits. The second is 5 plus every nibble sent before it, the XOR nibble included, on 4 bits.

### FUNC-LACROSSE-009
**Title:** Lacrosse bits are 400/800 µs pulses, nibbles least significant bit first, each followed by a one bit separator
**Status:** validated
**Dependencies:** —
**Description:** A one bit is radio high 400 µs then low 800 µs; a zero bit is high 800 µs then low 400 µs. Every nibble of the frame (type, address, data and check nibbles) is sent as its four bits, least significant first, followed by a one bit.

### FUNC-X10-001
**Title:** RFXmeter frame: address, address check, 24-bit value, packet type and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** An RFXmeter frame is 6 bytes: byte 0 is the address; byte 1 is the address with its high nibble inverted; bytes 2–4 carry the 24-bit value: byte 4 its high byte, byte 2 its middle byte, byte 3 its low byte (the value is 0 when it does not fit in 24 bits); the high nibble of byte 5 is the packet type; the low nibble of byte 5 is the inverted sum of all the nibbles before it. It is sent per [FUNC-X10-005](#func-x10-005).

### FUNC-X10-002
**Title:** RFXsensor frame: address and type, check, value, status and nibble-sum parity
**Status:** validated
**Dependencies:** —
**Description:** An RFXsensor frame is 4 bytes: byte 0 is the address shifted left by 2 with the sensor type in bits 0–1 (temperature 0, voltage 2, A/D 1, message 3); byte 1 is byte 0 with its high nibble inverted; byte 2 is the value; the high nibble of byte 3 is the packet-type status; the low nibble of byte 3 is the inverted sum of all the nibbles before it. It is sent per [FUNC-X10-005](#func-x10-005).

### FUNC-X10-003
**Title:** X10 switch frame: house code, scattered unit code and byte complements
**Status:** validated
**Dependencies:** —
**Description:** An X10 switch frame is 4 bytes. House code `a`–`p` (either case) puts in the high nibble of byte 0 the X10 code 6, 7, 4, 5, 8, 9, 10, 11, 14, 15, 12, 13, 0, 1, 2, 3 respectively; another character gives 0. With `u` = unit − 1, bit 3 of `u` goes to bit 2 of byte 0, and bits 2, 1, 0 of `u` go to bits 6, 3, 4 of byte 2. The command (on, off, bright, dim) is the rest of byte 2. Byte 1 is the complement of byte 0 and byte 3 the complement of byte 2. It is sent per [FUNC-X10-005](#func-x10-005).

### FUNC-X10-004
**Title:** X10 security frame with a single parity bit
**Status:** validated
**Dependencies:** —
**Description:** An X10 security frame is 6 bytes: byte 0 is the address; byte 1 is the address with its low nibble inverted; byte 2 is the command and byte 3 its complement; byte 4 is the id; byte 5 is the parity of the frame, 0x00 or 0x01: the XOR of bytes 0–4 folded down to one bit. It is sent per [FUNC-X10-005](#func-x10-005).

### FUNC-X10-005
**Title:** Each X10 frame is repeated with a 40 ms gap, the LED on throughout
**Status:** validated
**Dependencies:** —
**Description:** An X10 frame is sent twice in each sensor cycle (the encoder takes the copy count at construction). Each copy is a preamble, radio high 8960 µs then low 4500 µs, then the frame bits most significant first and a one stop bit, each bit being high 560 µs then low 560 µs (`0`) or 1680 µs (`1`), followed by a 40 ms silence. The LED is lit from before the first copy until after the last.

### FUNC-X10-006
**Title:** Battery voltage in mV is sent as BCD in an RFXmeter frame while the battery is not low
**Status:** validated
**Dependencies:** [FUNC-X10-001](#func-x10-001), [FUNC-BATTERY-001](#func-battery-001), [FUNC-BATTERY-002](#func-battery-002)
**Description:** When the build sets a battery meter address ([CONF-BUILD-008](configuration.md#conf-build-008)) and the battery is not low, every cycle sends the battery voltage in mV, one decimal digit per nibble (3960 mV gives 0x3960), in an RFXmeter frame with that address and packet type 0. Nothing is sent while the battery is low.

### FUNC-X10-007
**Title:** Auxiliary analog voltage in mV is sent as BCD in an RFXmeter frame while the battery is not low
**Status:** validated
**Dependencies:** [FUNC-X10-001](#func-x10-001), [FUNC-ANALOG-001](#func-analog-001)
**Description:** When the build sets an analog meter address ([CONF-BUILD-009](configuration.md#conf-build-009)) and the battery is not low, every cycle sends the auxiliary analog voltage in mV ([FUNC-ANALOG-001](#func-analog-001)), one decimal digit per nibble, in an RFXmeter frame with that address and packet type 0. Nothing is sent while the battery is low.

### FUNC-BATTERY-001
**Title:** VCC measured against the internal 1.1 V reference; pin voltages scaled from VCC
**Status:** validated
**Dependencies:** —
**Description:** VCC in mV is the board's calibrated 1.1 V reference value (mV, [CONF-BUILD-005](configuration.md#conf-build-005)) × (1024 / the ADC reading of the internal 1.1 V reference), truncated to an integer. A pin voltage in mV is VCC / 1024 × its 10-bit ADC reading, truncated to an integer; [FUNC-BATTERY-002](#func-battery-002) and [FUNC-ANALOG-001](#func-analog-001) use it.

### FUNC-BATTERY-002
**Title:** Battery voltage is VCC, or the PA1 divider reading multiplied by the board's divider ratio
**Status:** validated
**Dependencies:** [FUNC-BATTERY-001](#func-battery-001)
**Description:** On boards where the battery powers the chip directly, the battery voltage is the measured VCC. On other boards, it is the PA1 voltage ([FUNC-BATTERY-001](#func-battery-001) pin voltage formula) multiplied by the divider ratio set at build time, which is 1 when not set. This scaled voltage drives the low-battery decision and is the value the X10 battery meter reports. Acceptance: with a ratio of 2, the battery meter reports twice the PA1 voltage; without a ratio, it reports the PA1 voltage unchanged.

### FUNC-BATTERY-003
**Title:** Low battery mutes the X10 meters and the luminosity frames, and sets the Oregon battery-low flag
**Status:** validated
**Dependencies:** [FUNC-BATTERY-002](#func-battery-002), [FUNC-OREGON-011](#func-oregon-011), [FUNC-X10-006](#func-x10-006), [FUNC-X10-007](#func-x10-007), [FUNC-SENSOR-003](#func-sensor-003)
**Description:** The battery is low when its voltage is below the build's threshold ([CONF-BUILD-007](configuration.md#conf-build-007)). Then the battery and analog X10 meter frames and the luminosity frames ([FUNC-SENSOR-003](#func-sensor-003)) are not sent, and Oregon frames carry the battery-low flag ([FUNC-OREGON-011](#func-oregon-011)). The environment frame is still sent; a Lacrosse frame has no battery-low flag, so it is unchanged.

### FUNC-SENSOR-001
**Title:** Temperature, humidity and pressure read from a BME280 or BMP280 at I2C address 0x76
**Status:** validated
**Dependencies:** —
**Description:** On a build with a BME280 or BMP280 ([CONF-BUILD-002](configuration.md#conf-build-002)), each cycle reads the temperature (°C), the pressure and, on a BME280, the humidity (%) from the sensor at I2C address 0x76, then puts the sensor to sleep. Wrapper details: [TECH-SENSOR-002](technical.md#tech-sensor-002). Verification: inspection and the AVR builds; native builds stub the sensor ([TECH-SENSOR-002](technical.md#tech-sensor-002)). Bench check pending: BME280 readings over several cycles on the mushroom board.

### FUNC-SENSOR-002
**Title:** DS18B20 temperature read on the 1-Wire bus on PA3, in whole °C
**Status:** draft
**Dependencies:** —
**Description:** On a build with a DS18B20, each cycle starts a conversion on the 1-Wire bus on PA3, waits 1 s and reads the result. The temperature in whole °C is the raw 1/16 °C value divided by 16. A result of 0 °C counts as a failed read, so a real 0 °C is lost too: the cycle then has no temperature and, with no other temperature source, sends no environment frame ([CONF-BUILD-002](configuration.md#conf-build-002)). Wrapper details: [TECH-SENSOR-001](technical.md#tech-sensor-001). Verification: inspection and the AVR builds only; no sensor unit in service uses a DS18B20, so it is not bench-tested.

### FUNC-SENSOR-003
**Title:** BH1750 luminosity sent twice as a Lacrosse luminosity frame
**Status:** validated
**Dependencies:** [FUNC-LACROSSE-007](#func-lacrosse-007)
**Description:** On a build with a BH1750 and while the battery is not low, each cycle starts a one-time high-resolution measurement on the sensor at I2C address 0x23, waits 150 ms, reads the lux value and sends it twice in a Lacrosse luminosity frame ([FUNC-LACROSSE-007](#func-lacrosse-007)), for better reception.

### FUNC-ANALOG-001
**Title:** Auxiliary analog input on PA0 measured in millivolts
**Status:** validated
**Dependencies:** [FUNC-BATTERY-001](#func-battery-001)
**Description:** When the build sets an analog meter address ([CONF-BUILD-009](configuration.md#conf-build-009)) and the battery is not low, each cycle reads PA0 with the ADC and converts the reading to mV with the [FUNC-BATTERY-001](#func-battery-001) pin voltage formula.

### FUNC-MAIN-001
**Title:** Environment frame sent twice per cycle, separated by a silence at least one frame long
**Status:** validated
**Dependencies:** [FUNC-OREGON-014](#func-oregon-014), [FUNC-LACROSSE-002](#func-lacrosse-002), [PLAT-POWER-004](platform.md#plat-power-004), [TECH-MAIN-001](technical.md#tech-main-001)
**Description:** In every cycle where the sensor sends an environment frame (Oregon or Lacrosse, whichever the build uses), it sends that frame twice, and both copies are identical. Between the end of the first copy and the start of the second, the radio stays low for at least the length of one frame: 246 ms for Oregon and 98 ms for Lacrosse. This matches commercial Oregon sensors and makes up for single frames that RFLink missed. The X10 meter frames, the luminosity frame, and cycles that send no environment frame stay as they are. Acceptance: in one cycle on an Oregon build and one on a Lacrosse build, the radio output holds two identical environment frames. Between them there is a silence of at least 246 ms (Oregon) or 98 ms (Lacrosse), and the radio is never high during it. A build with no environment protocol sends no extra frame.

---

### FUNC-DECODE-001
**Title:** Receiver decodes X10 RFXmeter frames (address, value) from radio pulses
**Status:** validated
**Dependencies:** [FUNC-X10-001](#func-x10-001), [TECH-DECODE-001](technical.md#tech-decode-001), [TECH-DECODE-003](technical.md#tech-decode-003)
**Description:** The receiver decodes RFXmeter frames from the radio pulses (level and duration). Each timing window is nominal ±30 %: preamble high 8960 µs then low 4500 µs, then 48 bits most significant first, each high 560 µs then low 560 µs (`0`) or 1680 µs (`1`). A frame of 6 bytes `b0..b5` is decoded at the low pulse of bit 48 when `b1` is `b0` with its high nibble inverted, the high nibble of `b5` is 0, and the low nibble of `b5` is the inverted sum of the nibbles before it. Its address is `b0` and its value is the 24-bit number `b4 b2 b3` (`b4` most significant), until the next decoded frame. A failed check or a pulse of unexpected level or duration drops the frame in progress, and that pulse is checked again as a preamble start. Test: the pulses of the X10 encoder sending one RFXmeter frame with 2 repeats give exactly two decoded frames, each with that address and value.

### FUNC-DECODE-002
**Title:** Receiver decodes Lacrosse WS7000 frames (type, address) from radio pulses
**Status:** validated
**Dependencies:** [FUNC-LACROSSE-001](#func-lacrosse-001), [FUNC-LACROSSE-002](#func-lacrosse-002), [FUNC-LACROSSE-003](#func-lacrosse-003), [FUNC-LACROSSE-008](#func-lacrosse-008), [FUNC-LACROSSE-009](#func-lacrosse-009), [TECH-DECODE-001](technical.md#tech-decode-001), [TECH-DECODE-003](technical.md#tech-decode-003)
**Description:** The receiver decodes Lacrosse WS7000 frames from the radio pulses. Windows are nominal ±30 %: bit `1` is high 400 µs then low 800 µs, bit `0` high 800 µs then low 400 µs. Sync is at least 6 zero bits then a one. Then come nibbles, each 4 bits least significant first plus a `1` separator. The first nibble is the type: types 0, 1, 4 and 5 have 3, 6, 10 and 7 data nibbles after the address nibble; another type is rejected. The two check nibbles are verified per [FUNC-LACROSSE-008](#func-lacrosse-008). A frame is decoded at the high pulse of its last separator, with its type and its address (bits 0–2 of the address nibble). A pulse of unexpected level or duration, a `0` separator or a failed check drops the frame in progress, and that pulse is checked again as a sync start. Test: the Lacrosse encoder sending each type twice gives exactly two decoded frames, with that type and address.

### FUNC-DECODE-003
**Title:** Receiver reads Lacrosse measurements in tenths and lux
**Status:** validated
**Dependencies:** [FUNC-DECODE-002](#func-decode-002), [FUNC-LACROSSE-003](#func-lacrosse-003), [FUNC-LACROSSE-004](#func-lacrosse-004), [FUNC-LACROSSE-005](#func-lacrosse-005), [FUNC-LACROSSE-006](#func-lacrosse-006), [FUNC-LACROSSE-007](#func-lacrosse-007)
**Description:** With `d1..dn` the data nibbles after the address nibble in transmission order, a decoded Lacrosse frame gives, until the next one: temperature in tenths `d3*100 + d2*10 + d1`, negative when bit 3 of the address nibble is set (types 0, 1, 4); humidity in tenths `d6*100 + d5*10 + d4` (types 1, 4); pressure in hPa tenths `d9*1000 + d8*100 + d7*10 + d10 + 2000` (type 4); lux `d3*100 + d2*10 + d1` (type 5, `d4..d7` ignored). A value the frame type does not carry reads 0. Test: the Lacrosse encoder sending −10.2 °C, 42.5 %, 1013.4 hPa and 1000 lux reads −102, 425, 10134 and 1000.

### FUNC-DECODE-004
**Title:** Receiver decodes Oregon v2.1 frames (device ID, channel, rolling code, battery) from radio pulses
**Status:** validated
**Dependencies:** [FUNC-OREGON-001](#func-oregon-001), [FUNC-OREGON-002](#func-oregon-002), [FUNC-OREGON-003](#func-oregon-003), [FUNC-OREGON-004](#func-oregon-004), [FUNC-OREGON-009](#func-oregon-009), [FUNC-OREGON-010](#func-oregon-010), [FUNC-OREGON-011](#func-oregon-011), [FUNC-OREGON-012](#func-oregon-012), [TECH-DECODE-001](technical.md#tech-decode-001), [TECH-DECODE-003](technical.md#tech-decode-003)
**Description:** The receiver decodes Oregon v2.1 frames from the radio pulses. Windows: short 150–731 µs, long 732–1199 µs (origin: RFLink `OregonDecoderV2`, `Plugins/Plugin_048.c` (couin3) lines 322-324, threshold moved from 600 µs). 732 µs is the midpoint of the nominal half-bit (488 µs) and bit (976 µs) at 1024 Hz: a weak commercial THGN132N on the RXB6 (bench, 2026-11-12) gives highs ~80 µs short and lows ~80 µs long (short lows up to 649 µs, long highs from 800 µs), which the 600 µs threshold rejected (5 frames decoded vs 203 by an RFLink gateway over 2 h). Each bit is a long pulse whose level is the bit value; between two bits comes either one long pulse of the opposite level (next bit equal) or two short pulses, opposite then same level (next bit inverted). Sync: a long high pulse starts a run of at least 12 one bits (RFLink's 24 long pulses, line 333), ended by a zero bit where `b0` starts (its low nibble is the sync nibble 0xA). Bytes `b0..b10` are least significant bit first; nibble `2i` is the low nibble of `b(i)`. `b0 b1` must be `0xEA 0x4C` (c = 13), `0x1A 0x2D` or `0x1A 0x3D` (c = 16), or `0x5A 0x5D` (c = 20); nibbles `c` and `c+1` are the low and high nibbles of (sum of nibbles `0..c-1` − 0xA) & 0xFF (lines 465, 654, 698); later nibbles are ignored. A frame is decoded on the long pulse of its last checksum bit (frame bit 60, 72 or 88). Until the next one, its device ID is `b0 b1`, its channel is 1, 2 or 3 when the high nibble of `b2` is 1, 2 or 4 (else 0), its rolling code is `b3`, and its battery is low when bit 2 or 3 of `b4` is set (lines 711-714). A pulse of unexpected level or duration, an unknown ID or a failed checksum drops the frame in progress, and that pulse is checked again as a sync start. Test: the Oregon encoder sending each [FUNC-OREGON-014](#func-oregon-014) device twice gives exactly two decoded frames with its ID, channel, code and battery flag; bit pulses of the RFLink captures `EA4C20725C21D083`, `EA4C101360193023`, `1A2D40C4512170463E`, `1A2D1072512080E73F` (lines 644-645, 685-686) give channel/code/battery 2/0x72/low, 1/0x13/ok, 3/0xC4/ok, 1/0x72/ok; a `0x1A3D` copy with recomputed checksum is accepted; 11 sync ones or one flipped data bit are rejected; a valid frame with short lows 620 µs, short highs 400 µs, long highs 880 µs, long lows 1060 µs decodes; window bounds 149/150, 731/732, 1199/1200 µs.

### FUNC-DECODE-005
**Title:** Receiver reads the Oregon measurements its device ID carries in tenths, % and hPa
**Status:** validated
**Dependencies:** [FUNC-DECODE-004](#func-decode-004), [FUNC-OREGON-005](#func-oregon-005), [FUNC-OREGON-006](#func-oregon-006), [FUNC-OREGON-007](#func-oregon-007), [FUNC-OREGON-008](#func-oregon-008), [FUNC-OREGON-012](#func-oregon-012)
**Description:** With `b0..b10` the frame bytes, a decoded Oregon frame gives, until the next one: temperature in tenths, the digits being the high nibble of `b5`, the low nibble of `b5` and the high nibble of `b4`, negative when bit 3 of `b6` is set (all IDs); humidity in %, the digits being the low nibble of `b7` and the high nibble of `b6` (IDs `0x1A2D`, `0x1A3D`, `0x5A5D`); pressure in hPa `b8 + 795` (ID `0x5A5D`) (RFLink `Plugins/Plugin_048.c` (couin3) lines 733-751; offset 795 per [FUNC-OREGON-008](#func-oregon-008)). Forecast and comfort nibbles are ignored. A value the device ID does not carry reads 0. Test: the Oregon encoder sending THN132N −10.2 °C reads −102 (humidity and pressure 0); THGR228N 21.5 °C, 67 % reads 215, 67 (pressure 0); BTHR918 21.5 °C, 60 %, 1013 hPa reads 215, 60, 1013; the [FUNC-DECODE-004](#func-decode-004) RFLink captures read 215; 196; 215 and 67; 205 and 78.

### FUNC-DECODE-006
**Title:** Receiver keeps the last decoded values of each radio source, 16 sources at most
**Status:** validated
**Dependencies:** [FUNC-DECODE-001](#func-decode-001), [FUNC-DECODE-003](#func-decode-003), [FUNC-DECODE-005](#func-decode-005)
**Description:** The receiver keeps, per radio source, the values of its last decoded frame and the second it was heard. A source is identified by its protocol (Oregon, Lacrosse, Lacrosse luminosity, X10), its id and its code. The values are temperature, humidity and pressure in tenths, lux and millivolts; a value the frame does not carry is absent and reads 0. Oregon: id is the channel, code the rolling code; temperature always, humidity (% × 10) for IDs `0x1A2D`, `0x1A3D`, `0x5A5D`, pressure (hPa × 10) for `0x5A5D`. Lacrosse and X10 have code 0. Lacrosse type 5 is a Lacrosse luminosity source with id the address and the lux value; types 0, 1 and 4 are Lacrosse sources with id the address, temperature, plus humidity (types 1, 4), plus pressure (type 4), in tenths as decoded. X10: id is the address; millivolts is the value read as BCD when every nibble is a decimal digit and the value is at most 0x9999, else the source has no value. A frame from a known source replaces all its values and its last-heard second; a new source is added. The Pro Mini and Pro Micro receivers keep 16 sources (native test builds keep 8, [TECH-DECODE-003](technical.md#tech-decode-003)); when full, the source heard longest ago is replaced in its place. Sources are listed in first-heard order. Test (native, 8 sources): frames from each encoder update or add the expected sources; two Oregon frames on channel 2 with codes 0xB8 and 0x36 give two sources; a 9th source replaces the oldest; X10 value 0x2950 gives 2950 mV, 0x2A50 gives no value.

### FUNC-DECODE-007
**Title:** Receiver shows one page per source every 10 s on the OLED and redraws only changed cells
**Status:** validated
**Dependencies:** [FUNC-DECODE-006](#func-decode-006), [FUNC-DECODE-010](#func-decode-010), [UI-DECODE-001](ui.md#ui-decode-001), [TECH-DECODE-002](technical.md#tech-decode-002), [TECH-DECODE-003](technical.md#tech-decode-003)
**Description:** The receiver feeds every captured radio pulse to its X10, Lacrosse and Oregon decoders and records each decoded frame with the current second ([FUNC-DECODE-006](#func-decode-006)). The page is computed again only when the second changes or a frame is decoded, and each computation first removes expired sources ([FUNC-DECODE-010](#func-decode-010)). While no source is known (at start, or once the last one expired), the screen shows the waiting page: `Waiting for data` on the middle text row (8x16 row 1, pixel rows 8–23, columns 0–15); it is never part of the rotation. Otherwise it shows source `(second / 10) % count`, laid out per [UI-DECODE-001](ui.md#ui-decode-001), with the alternate value when `(second / 2)` is odd. A source page is 6 big 16x32 cells and a side column of 2 lines of 4 characters (8x16 columns 12–15, identifier on row 0, humidity or pressure on row 2); its 14 cells cover the whole screen. The screen is blank at start, so the first waiting page draws only its 16 text cells. Entering a source page draws all 14 of its cells, spaces included. Entering the waiting page from a source page first draws the 14 source cells as spaces, then the 16 text cells. Each turn of the receiver's main loop, which also empties the pulse buffer, draws at most one pending cell, so a page change never delays the decoding by more than one cell (one that differs from what is shown, or forced by a page switch), the first in the order big 0–5, identifier 12–15, humidity or pressure 12–15, then waiting text 0–15. Test: waiting page at 12 s (row 1 reads `Waiting for data`, 16 draws); the first decoded frame then draws all 14 source cells; Oregon and Lacrosse frames give their pages alternating every 10 s; a BTHR918 alone shows its humidity at 0–1 s and its pressure at 2–3 s on row 2; no draw when nothing changed; a single source last heard 1800 s ago gives the waiting page after 14 blank source cells (big cells and rows 0 and 2, columns 12–15, are spaces).

### FUNC-DECODE-008
**Title:** Receiver renders each decoded frame as an RFLink text line
**Status:** validated
**Dependencies:** [FUNC-DECODE-001](#func-decode-001), [FUNC-DECODE-003](#func-decode-003), [FUNC-DECODE-005](#func-decode-005), [TECH-DECODE-003](technical.md#tech-decode-003)
**Description:** A decoded frame renders as one RFLink line of at most 63 characters, without line terminator: `20;`, the line counter as 2 uppercase hex digits, `;`, the name, `;`, then `KEY=value;` fields. `ID` is 4 uppercase hex digits; hex values are lowercase. `TEMP` is the absolute temperature in tenths as 4 hex digits, plus 0x8000 when negative. Oregon: name `Oregon Temp` (`0xEA4C`), `Oregon TempHygro` (`0x1A2D`, `0x1A3D`), `Oregon BTHR` (`0x5A5D`); `ID` is rolling code × 256 + channel; `TEMP`; `HUM` in decimal (`0x1A2D`, `0x1A3D`, `0x5A5D`); `BARO` in hPa as 4 hex digits (`0x5A5D`); `BAT=OK` or `BAT=LOW`. Lacrosse: name `LaCrosse WS7000`; `ID` is type × 256 + address; `TEMP` (types 0, 1, 4); `HUM` is (humidity tenths + 5) / 10 in decimal (types 1, 4); `BARO` is (pressure tenths + 5) / 10 as 4 hex digits (type 4); `LUX` as 4 hex digits (type 5). X10: name `X10Meter`; `ID` is the address; `METER` is the millivolts in decimal when the value is valid BCD ([FUNC-DECODE-006](#func-decode-006) rule), else the line ends after `ID`. Test: THGR228N ch 3, code 0xC4, 21.5 °C, 67 %, counter 0x0A gives `20;0A;Oregon TempHygro;ID=C403;TEMP=00d7;HUM=67;BAT=OK;`; THN132N ch 1, code 0x3C, −10.2 °C, battery low, counter 0 gives `20;00;Oregon Temp;ID=3C01;TEMP=8066;BAT=LOW;`; BTHR918 ch 3, code 0x7E, 21.5 °C, 60 %, 1013 hPa, battery low, counter 0xFF gives `20;FF;Oregon BTHR;ID=7E03;TEMP=00d7;HUM=60;BARO=03f5;BAT=LOW;`; Lacrosse type 4 address 3, −10.2 °C, 42.5 %, 1013.4 hPa, counter 1 gives `20;01;LaCrosse WS7000;ID=0403;TEMP=8066;HUM=43;BARO=03f5;`; Lacrosse type 5 address 2, 1000 lux, counter 1 gives `20;01;LaCrosse WS7000;ID=0502;LUX=03e8;`; X10 address 10, value 0x2950, counter 2 gives `20;02;X10Meter;ID=000A;METER=2950;`; value 0x2A50, counter 3 gives `20;03;X10Meter;ID=000A;`.

### FUNC-DECODE-009
**Title:** Receiver writes one RFLink line per decoded frame on its serial output
**Status:** validated
**Dependencies:** [FUNC-DECODE-007](#func-decode-007), [FUNC-DECODE-008](#func-decode-008), [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** On a receiver with a line output, every decoded frame, once recorded ([FUNC-DECODE-006](#func-decode-006)), is written as one RFLink line ([FUNC-DECODE-008](#func-decode-008)). The line counter starts at 0 and goes up by one after each line, wrapping after 0xFF. No deduplication: a frame sent twice gives two lines. The display ([FUNC-DECODE-007](#func-decode-007)) is unchanged. Test: Oregon then Lacrosse frames, each sent twice, give 4 lines in reception order with counters `00` to `03`.

### FUNC-DECODE-010
**Title:** Receiver forgets a source not heard for 30 minutes
**Status:** validated
**Dependencies:** [FUNC-DECODE-006](#func-decode-006)
**Description:** A source last heard 1800 s or more ago is removed. The other sources keep their first-heard order and the count drops by the number removed; nothing else removes a source (the [FUNC-DECODE-006](#func-decode-006) replacement when full is unchanged). A removed source heard again is added as a new source. Removal runs before each page computation ([FUNC-DECODE-007](#func-decode-007)), so an expired source leaves the rotation and an empty list shows the waiting page. Test (native): a source heard at 100 s is kept at 1899 s and removed at 1900 s; of sources heard at 0, 500 and 1000 s, the second and third remain at 1800 s; a source heard at 0 s and again at 1000 s remains at 1800 s; removal on an empty list keeps the count at 0.
