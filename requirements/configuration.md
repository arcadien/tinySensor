# Configuration Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### CONF-BUILD-001
**Title:** Enforce exclusive protocol selection at compile time
**Status:** validated
**Dependencies:** —
**Description:** Exactly one of `USE_OREGON` or `USE_LACROSSE` must be defined per build environment. Defining neither causes a compile error because no protocol encoder is included; defining both causes a compile error due to conflicting encoder instantiation in `main.cpp`.

### CONF-BUILD-002
**Title:** Select sensor type exclusively via build flags
**Status:** validated
**Dependencies:** —
**Description:** Exactly one of `USE_BMP280`, `USE_BME280`, `USE_DS18B20`, or `USE_BH1750` must be defined per build environment to select the active sensor. When any I2C sensor flag (`USE_BMP280`, `USE_BME280`, or `USE_BH1750`) is present and `USE_I2C` is not explicitly defined, `config.h` auto-derives `USE_I2C`.

### CONF-BUILD-003
**Title:** Set Oregon channel and rolling code per environment
**Status:** validated
**Dependencies:** CONF-BUILD-001
**Description:** When `USE_OREGON` is defined, `OREGON_CHANNEL` must be set to an integer in the range 1–3 and `OREGON_RCODE` must be set to a value no greater than `0xA5`. Both are passed directly to `OregonV3::SetChannel()` and `OregonV3::SetRollingCode()` in `main()`.

### CONF-BUILD-004
**Title:** Set Lacrosse device address per environment
**Status:** validated
**Dependencies:** CONF-BUILD-001
**Description:** When `USE_LACROSSE` is defined, `LACROSSE_ID` must be set to a valid `LacrosseWS7000::Address` enum value. It is passed directly to `LacrosseWS7000::SetAddress()` in `main()`.

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
