# Configuration Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->
<!-- Configuration requirements state build flags, environments and their effect; the code that reads them belongs in technical.md. -->

### CONF-BUILD-001
**Title:** Select at most one environment protocol per build
**Status:** validated
**Dependencies:** —
**Description:** A build selects at most one environment protocol, `USE_OREGON` or `USE_LACROSSE`; selecting both fails at compile time. With neither, the sensor sends no environment frame; the X10 meters and the luminosity are still sent.

### CONF-BUILD-002
**Title:** Select temperature and luminosity sensors via build flags
**Status:** validated
**Dependencies:** [FUNC-SENSOR-001](functional.md#func-sensor-001), [FUNC-SENSOR-003](functional.md#func-sensor-003)
**Description:** `USE_BMP280` or `USE_BME280` selects a BMx280 sensor for the temperature; `USE_BME280` adds humidity and pressure and wins when both are set. `USE_DS18B20` selects the DS18B20; combined with a BMx280, the DS18B20 gives the temperature and the BMx280 the humidity and pressure. With no temperature sensor, no environment frame is sent. `USE_BH1750` adds luminosity reporting, whatever the temperature sensor and the environment protocol. `USE_BMP280`, `USE_BME280` or `USE_BH1750` turns on the I2C bus (`USE_I2C`) when the build does not set it.

### CONF-BUILD-003
**Title:** Set Oregon channel and rolling code per environment
**Status:** validated
**Dependencies:** [CONF-BUILD-001](#conf-build-001)
**Description:** An Oregon build sets `OREGON_CHANNEL` to 1, 2 or 3 and `OREGON_RCODE` to a rolling code 0x00–0xFF ([FUNC-OREGON-010](functional.md#func-oregon-010); the old 0xA5 limit came from the former layout that split the code over bytes 2 and 3). Every Oregon frame carries them ([FUNC-OREGON-009](functional.md#func-oregon-009), [FUNC-OREGON-010](functional.md#func-oregon-010)). Verification: inspection; the native cycle tests build Oregon variants with these flags but compare against frames built from the same values.

### CONF-BUILD-004
**Title:** Set Lacrosse device address per environment
**Status:** validated
**Dependencies:** [CONF-BUILD-001](#conf-build-001), [FUNC-SENSOR-003](functional.md#func-sensor-003)
**Description:** A build with `USE_LACROSSE` or `USE_BH1750` sets `LACROSSE_ID` to a Lacrosse address, whatever the environment protocol. Without it, the build fails at compile time with an error naming [CONF-BUILD-004](#conf-build-004).

### CONF-BUILD-005
**Title:** Supply individually calibrated internal 1.1 V reference per board
**Status:** validated
**Dependencies:** [FUNC-BATTERY-001](functional.md#func-battery-001)
**Description:** Each physical board's environment sets `INTERNAL_1v1` to the measured voltage, in mV, of that chip's internal 1.1 V reference, which varies from chip to chip. The VCC measurement uses it ([FUNC-BATTERY-001](functional.md#func-battery-001)).

### CONF-BUILD-006
**Title:** Configure hibernate duration via SLEEP_TIME_IN_SECONDS
**Status:** validated
**Dependencies:** —
**Description:** `SLEEP_TIME_IN_SECONDS` sets how long, in seconds, the sensor sleeps between two cycles. The default is 32 s.

### CONF-BUILD-007
**Title:** Configure low-battery threshold via LOW_BATTERY_VOLTAGE
**Status:** validated
**Dependencies:** [FUNC-BATTERY-003](functional.md#func-battery-003)
**Description:** `LOW_BATTERY_VOLTAGE` sets the battery voltage, in mV, below which the battery is low ([FUNC-BATTERY-003](functional.md#func-battery-003)). The default is 2000 mV. Verification: inspection; the native cycle tests use readings above the threshold, and the low-battery effect is tested on its own ([FUNC-BATTERY-003](functional.md#func-battery-003)).

### CONF-BUILD-008
**Title:** Assign a unique X10 meter address for battery voltage reporting
**Status:** validated
**Dependencies:** [FUNC-X10-006](functional.md#func-x10-006)
**Description:** `BATTERY_VOLTAGE_X10_ID` sets the X10 meter address of the battery voltage ([FUNC-X10-006](functional.md#func-x10-006)). It is set on every board and is unique among the boards heard by the same receiver.

### CONF-BUILD-009
**Title:** Optionally assign an X10 meter address for auxiliary analog reporting
**Status:** validated
**Dependencies:** [FUNC-X10-007](functional.md#func-x10-007), [FUNC-ANALOG-001](functional.md#func-analog-001)
**Description:** `ANALOG1_X10_ID`, when set, turns on the auxiliary analog measurement on PA0 and sets its X10 meter address ([FUNC-ANALOG-001](functional.md#func-analog-001), [FUNC-X10-007](functional.md#func-x10-007)). Without it, nothing is measured or sent for that input. The address is unique among the boards heard by the same receiver.

### CONF-BUILD-010
**Title:** Signal direct VCC battery measurement via BATTERY_IS_VCC
**Status:** validated
**Dependencies:** [FUNC-BATTERY-002](functional.md#func-battery-002)
**Description:** `BATTERY_IS_VCC` marks a board with no divider on PA1, where the battery powers the chip: it selects the VCC case of [FUNC-BATTERY-002](functional.md#func-battery-002).

### CONF-BUILD-011
**Title:** Document charge-pump sensor rail via USE_CHARGE_PUMP
**Status:** validated
**Dependencies:** —
**Description:** `USE_CHARGE_PUMP` only records that the board's sensor rail is boosted by a charge pump. It changes nothing in the firmware. Verification: inspection, no firmware source reads `USE_CHARGE_PUMP`.

### CONF-BUILD-012
**Title:** BATTERY_DIVIDER sets the battery divider ratio on PA1
**Status:** validated
**Dependencies:** [FUNC-BATTERY-002](functional.md#func-battery-002)
**Description:** `BATTERY_DIVIDER` sets the divider ratio of [FUNC-BATTERY-002](functional.md#func-battery-002) on boards with a resistor divider on PA1: 2 for the 1 MΩ / 1 MΩ divider of the v2.2 rework and of v3.0, which closes BACKLOG F1.

### CONF-BUILD-013
**Title:** OREGON_NO_PRESSURE makes a BME280 Oregon sensor send temperature and humidity only
**Status:** validated
**Dependencies:** [CONF-BUILD-001](#conf-build-001), [CONF-BUILD-002](#conf-build-002), [FUNC-OREGON-012](functional.md#func-oregon-012)
**Description:** With `OREGON_NO_PRESSURE`, a BME280 board drops the pressure reading. It sends a THGR228N temperature-and-humidity frame, like the commercial outdoor sensors, instead of a BTHR918 frame. Setting the flag on a build without the Oregon protocol fails at compile time. Acceptance: such a build sends a THGR228N frame with the measured temperature and humidity.

### CONF-BUILD-014
**Title:** `mushroom` environment builds the 18650-powered BME280 Oregon sensor in the mushroom enclosure
**Status:** validated
**Dependencies:** [CONF-BUILD-003](#conf-build-003), [CONF-BUILD-005](#conf-build-005), [CONF-BUILD-007](#conf-build-007), [CONF-BUILD-008](#conf-build-008), [CONF-BUILD-012](#conf-build-012), [CONF-BUILD-013](#conf-build-013)
**Description:** The `mushroom` board is a cut-down v2.2 with a BME280. Its 18650 is charged by a TP4056, and a 3.3 V buck-boost supplies VCC, so VCC is not the battery voltage and PA1 reads half of it. It sends Oregon temperature and humidity without pressure on channel 3 with rolling code 0x3C. The X10 meter 7 reports the battery through a divider of 2 every 5 minutes. The low-battery threshold is 3000 mV, so the discharge stays tracked down to 3.0 V while the buck-boost keeps VCC steady. The 1.1 V reference is calibrated at 1104 mV (4040 mV measured on J13 against 4026 mV reported). The environment is part of the default build, and the environment docs list it with its battery and threshold. Acceptance: the environment builds.

### CONF-DECODE-002
**Title:** CI and pre-commit cover tinyDecoders: lint, native tests and the promini and promicro builds
**Status:** validated
**Dependencies:** [CONF-DECODE-003](#conf-decode-003), [CONF-DECODE-004](#conf-decode-004), [TECH-DECODE-001](technical.md#tech-decode-001)
**Description:** The CI (`.github/workflows/main.yml`) lints `tinyDecoders` with the `.rbd/config.yml` linter command (build trees `*/.pio` skipped), runs the `tinyDecoders` native tests, and builds the `promini` and `promicro` environments. It publishes the promicro firmware (`.hex`, `.elf`, production) as `firmware-tinydecoders-promicro` and the promini one as `firmware-tinydecoders-promini`, and the release waits for these builds. The PlatformIO cache follows changes to `tinyDecoders/platformio.ini`. The pre-commit clang-format hook covers `tinyDecoders/src`, `tinyDecoders/lib` and `tinyDecoders/test`. Verified by a green CI run.

### CONF-DECODE-003
**Title:** tinyDecoders promicro environment builds the receiver for the Pro Micro 5 V / 16 MHz breadboard prototype
**Status:** validated
**Dependencies:** [FUNC-DECODE-009](functional.md#func-decode-009), [PLAT-DECODE-003](platform.md#plat-decode-003), [PLAT-DECODE-004](platform.md#plat-decode-004), [PLAT-DECODE-005](platform.md#plat-decode-005)
**Description:** The Pro Micro is a breadboard prototype only; the production receiver is the Pro Mini ([CONF-DECODE-004](#conf-decode-004)). `tinyDecoders/platformio.ini` has `[env:promicro]`: `platform = atmelavr`, `board = sparkfun_promicro16`, `framework = arduino`, the default upload (`avr109`, Caterina bootloader over USB) and no `lib_deps`. It builds the same receiver firmware as the Pro Mini (entry point: [TECH-DECODE-005](technical.md#tech-decode-005)); the Arduino core keeps the Pro Micro USB stack running. Verified by `pio run -d tinyDecoders -e promicro`.

### CONF-DECODE-004
**Title:** tinyDecoders promini environment builds the production receiver for the Pro Mini 5 V / 16 MHz (ATmega328P)
**Status:** validated
**Dependencies:** [FUNC-DECODE-007](functional.md#func-decode-007), [FUNC-DECODE-009](functional.md#func-decode-009), [PLAT-DECODE-003](platform.md#plat-decode-003), [PLAT-DECODE-004](platform.md#plat-decode-004), [PLAT-DECODE-005](platform.md#plat-decode-005)
**Description:** The production tinyDecoders receiver is a standalone viewer on an Arduino Pro Mini 5 V / 16 MHz (ATmega328P), fed USB 5 V on its VCC pin, bypassing the onboard regulator; its wiring is in [PLAT-DECODE-003](platform.md#plat-decode-003) to 005. `tinyDecoders/platformio.ini` has `[env:promini]`: `platform = atmelavr`, `board = pro16MHzatmega328`, `framework = arduino`, the default upload (`arduino` protocol through the Pro Mini bootloader, over a USB-serial adapter on the programming header), no `lib_deps`, and `build_flags = -DSERIAL_TX_BUFFER_SIZE=128`: the core default of 64 B leaves 63 free bytes, exactly the longest RFLink line plus `\r\n`, and 128 B leaves room for a second line. It builds the same receiver firmware as the Pro Micro ([CONF-DECODE-003](#conf-decode-003)). Verified by `pio run -d tinyDecoders -e promini`.