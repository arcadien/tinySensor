# Configuration Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### CONF-BUILD-001
**Title:** Select at most one environment protocol per build
**Status:** validated
**Dependencies:** —
**Description:** At most one of `USE_OREGON` or `USE_LACROSSE` may be defined per build environment; defining both is rejected at compile time. With neither defined, the sensor sends no environment frame; X10 meters and luminosity are still transmitted.

### CONF-BUILD-002
**Title:** Select temperature and luminosity sensors via build flags
**Status:** validated
**Dependencies:** FUNC-SENSOR-001, FUNC-SENSOR-002, FUNC-SENSOR-003
**Description:** `USE_BMP280` or `USE_BME280` enables the BMx280 sensor for temperature; `USE_BME280` adds humidity and pressure and takes precedence when both are defined. `USE_DS18B20` enables the DS18B20 temperature sensor; combined with a BMx280, the DS18B20 provides the temperature and the BMx280 humidity and pressure. With no temperature sensor defined, no environment frame is sent. `USE_BH1750` enables luminosity reporting independently of the temperature sensor and of the environment protocol. When `USE_BMP280`, `USE_BME280` or `USE_BH1750` is defined and `USE_I2C` is not, `USE_I2C` is derived automatically.

### CONF-BUILD-003
**Title:** Set Oregon channel and rolling code per environment
**Status:** validated
**Dependencies:** CONF-BUILD-001
**Description:** When `USE_OREGON` is defined, `OREGON_CHANNEL` must be set to an integer in the range 1–3 and `OREGON_RCODE` must be set to a value no greater than `0xA5`. Both are passed directly to `OregonV3::SetChannel()` and `OregonV3::SetRollingCode()` in `main()`.

### CONF-BUILD-004
**Title:** Set Lacrosse device address per environment
**Status:** validated
**Dependencies:** CONF-BUILD-001, FUNC-SENSOR-003
**Description:** `LACROSSE_ID` must be set to a valid `LacrosseWS7000::Address` value whenever `USE_LACROSSE` or `USE_BH1750` is defined, whatever the environment protocol. A build defining either flag without `LACROSSE_ID` is rejected at compile time with an error naming CONF-BUILD-004.

### CONF-BUILD-005
**Title:** Supply individually calibrated internal 1.1 V reference per board
**Status:** validated
**Dependencies:** FUNC-BATTERY-001
**Description:** `INTERNAL_1v1` must be defined in millivolts for each physical ATtiny84a board via `build_flags`. Its value represents the actual measured voltage of that chip's internal 1.1 V ADC reference, which varies chip to chip, and is consumed by `Hal::ComputeVccMv()`.

### CONF-BUILD-006
**Title:** Configure hibernate duration via SLEEP_TIME_IN_SECONDS
**Status:** validated
**Dependencies:** —
**Description:** `SLEEP_TIME_IN_SECONDS` sets the duration in seconds that the firmware hibernates between measurement cycles. When not defined, `config.h` defaults it to 32 seconds.

### CONF-BUILD-007
**Title:** Configure low-battery threshold via LOW_BATTERY_VOLTAGE
**Status:** validated
**Dependencies:** FUNC-BATTERY-003
**Description:** `LOW_BATTERY_VOLTAGE` sets the battery voltage threshold in millivolts below which the `lowBattery` flag is raised. When not defined, `config.h` defaults it to 2000 mV.

### CONF-BUILD-008
**Title:** Assign a unique X10 meter address for battery voltage reporting
**Status:** validated
**Dependencies:** FUNC-X10-006
**Description:** `BATTERY_VOLTAGE_X10_ID` must be defined and must be unique across all boards transmitting to the same receiver. It is passed as the address argument to `x10rf::RFXmeter()` when reporting battery voltage.

### CONF-BUILD-009
**Title:** Optionally assign an X10 meter address for auxiliary analog reporting
**Status:** validated
**Dependencies:** FUNC-X10-007, FUNC-ANALOG-001
**Description:** When `ANALOG1_X10_ID` is defined, the firmware reads PA0 and transmits the converted millivolt value via `x10rf::RFXmeter()` using that address. When undefined, no analog channel transmission occurs. The address must be unique across all boards reporting to the same receiver.

### CONF-BUILD-010
**Title:** Signal direct VCC battery measurement via BATTERY_IS_VCC
**Status:** validated
**Dependencies:** FUNC-BATTERY-002
**Description:** When `BATTERY_IS_VCC` is defined, `batteryVoltageInMv` is set directly to `vccMv` and `GetRawBattery()` is not called. This applies to boards where no external resistor divider is present on PA1 and VCC equals battery voltage.

### CONF-BUILD-011
**Title:** Document charge-pump sensor rail via USE_CHARGE_PUMP
**Status:** validated
**Dependencies:** —
**Description:** `USE_CHARGE_PUMP` is an informational build flag documenting that the sensor VCC rail on a given board is boosted via a charge pump. No firmware code branches on this flag; it exists solely for build-system documentation purposes.

### CONF-DECODE-002
**Title:** CI and pre-commit cover tinyDecoders: lint, native tests and the promini and promicro builds
**Status:** validated
**Dependencies:** CONF-DECODE-003, CONF-DECODE-004, TECH-DECODE-001
**Description:** In `.github/workflows/main.yml`, the `lint` job runs the `.rbd/config.yml` linter command (adds `tinyDecoders`, prunes `*/.pio`); `test-native` also runs `pio test -d tinyDecoders -e native`; a `build-tinydecoders` job runs `pio run -d tinyDecoders -e promini` and `pio run -d tinyDecoders -e promicro`, uploads the promini `firmware.hex`/`firmware.elf` as `firmware-tinydecoders` and the promicro ones as `firmware-tinydecoders-promicro`, and `release` also needs it. Cache keys hash `tinyDecoders/platformio.ini` too. `.pre-commit-config.yaml` clang-format `files` becomes `^(src|lib|include|test|tinyDecoders/(src|lib|test))/`. Verified by a green CI run.

### CONF-DECODE-003
**Title:** tinyDecoders promicro env builds the receiver firmware for the Pro Micro 5 V / 16 MHz breadboard prototype with the Arduino framework
**Status:** validated
**Dependencies:** FUNC-DECODE-009, PLAT-DECODE-003, PLAT-DECODE-004, PLAT-DECODE-005
**Description:** The Pro Micro is a breadboard prototype only; the production receiver is the Pro Mini (CONF-DECODE-004). `tinyDecoders/platformio.ini` has `[env:promicro]`: `platform = atmelavr`, `board = sparkfun_promicro16`, `framework = arduino`, default upload (`avr109`, Caterina bootloader over USB), no `lib_deps` (`Wire` is in the core). It builds the shared entry point `tinyDecoders/src/main.cpp`, the only file in `src/`, so no env sets `build_src_filter`: `main.cpp` defines `hal` (`ReceiverHal`) and `receiver` (`Receiver`, built on `&hal`) at file scope; `setup()` calls `hal.Init()`, `loop()` calls `receiver.Poll()` (the Arduino `main` keeps the USB stack running on the Pro Micro). Verified by `pio run -d tinyDecoders -e promicro`.

### CONF-DECODE-004
**Title:** tinyDecoders promini env builds the production receiver firmware for the Pro Mini 5 V / 16 MHz (ATmega328P) with the Arduino framework
**Status:** validated
**Dependencies:** FUNC-DECODE-007, FUNC-DECODE-009, PLAT-DECODE-003, PLAT-DECODE-004, PLAT-DECODE-005
**Description:** The production tinyDecoders receiver is a standalone viewer on an Arduino Pro Mini 5 V / 16 MHz (ATmega328P): RXB6 DATA on D8 (PB0), SSD1306 OLED on A4/A5, debug UART TX on D1, all wired at 5 V. The board is fed USB 5 V on its VCC pin, bypassing the onboard regulator. `tinyDecoders/platformio.ini` has `[env:promini]`: `platform = atmelavr`, `board = pro16MHzatmega328`, `framework = arduino`, default upload (`arduino` protocol through the Pro Mini bootloader, over a USB-serial adapter on the programming header), no `lib_deps`, `build_flags = -DSERIAL_TX_BUFFER_SIZE=128` (the core default of 64 B leaves 63 free bytes, exactly the longest RFLink line plus `
`; 128 B gives room for a second line). It builds the same `tinyDecoders/src/main.cpp` as CONF-DECODE-003. Verified by `pio run -d tinyDecoders -e promini`.
