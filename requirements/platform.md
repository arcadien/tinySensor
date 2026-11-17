# Platform Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->
<!-- Platform requirements state how the hardware is used (pins, peripherals, registers, timings); class, method and macro names belong in technical.md (TECH-HAL-007, TECH-DECODE-005). -->

### PLAT-POWER-001
**Title:** Hibernate is s / 8 watchdog periods of power-down sleep, rounded down, at least one
**Status:** validated
**Dependencies:** [TECH-HAL-004](technical.md#tech-hal-004)
**derives_from:** [TECH-HAL-004](technical.md#tech-hal-004)
**Description:** A hibernate of `s` seconds puts the ATtiny84A in power-down sleep for `s / 8` periods of the 8-second hardware watchdog, rounded down, and at least one: a request under 8 s still sleeps one period. Brownout detection is off during each sleep period. Verified by inspection (AVR only, no native test).

---

### PLAT-POWER-002
**Title:** Sensor power rail PA2 high from before the first read until after the last transmission
**Status:** validated
**Dependencies:** [FUNC-SENSOR-001](functional.md#func-sensor-001)
**derives_from:** [FUNC-SENSOR-001](functional.md#func-sensor-001)
**Description:** Each cycle drives PA2 (`SENSOR_VCC`) high before any sensor read and low right after the last transmission, before hibernating. No sensor read or transmission happens outside this window.

---

### PLAT-POWER-003
**Title:** Hibernate stops USI, Timer0, Timer1 and ADC, holds the sensor rail and radio data low, leaves the sensor-rail pull-up pins unpulled; everything is restored on wake
**Status:** validated
**Dependencies:** [PLAT-POWER-001](#plat-power-001), [PLAT-POWER-002](#plat-power-002), [TECH-HAL-004](technical.md#tech-hal-004)
**derives_from:** [TECH-HAL-004](technical.md#tech-hal-004)
**Description:** Before the first sleep period, the power reduction register stops the USI, Timer0, Timer1 and the ADC. PA2 (`SENSOR_VCC`) and PB0 (radio data, the radio being always powered) are outputs held low, so the sensor rail stays off and the transmitter cannot key itself from a floating input. PA3 (1-Wire), PA4 (SCL), PA5 (MISO) and PA6 (SDA) are inputs without internal pull-up: their external pull-ups go to `SENSOR_VCC`, or (PA5) they may reach the sensor SDO in SPI wiring, so an internal pull-up would back-power the unpowered sensors. A floating input costs nothing in power-down, where the input buffers are clamped. Every other pin is an input: PA0 and PA1 without pull-up, PB3 (RESET) pulled up, PB2 pulled up except with `USE_SWITCHED_5V` ([PLAT-POWER-006](#plat-power-006)), and PA7 (digital output) and PB1 (LED) keep their last output level as pull-up state. After the last sleep period, PRR, DDRA, DDRB, PORTA and PORTB get back their values from before sleep. Verified by inspection (AVR only, no native test).

---

### PLAT-POWER-004
**Title:** Every radio transmission is framed by the LED: on before, off after, then a 30 ms pause
**Status:** validated
**Dependencies:** [TECH-HAL-005](technical.md#tech-hal-005)
**derives_from:** [TECH-HAL-005](technical.md#tech-hal-005)
**Description:** A transmission is one frame of an encoder, with all its copies: the repeated copies of an X10 frame ([FUNC-X10-005](functional.md#func-x10-005)) are one transmission, while each luminosity copy ([FUNC-SENSOR-003](functional.md#func-sensor-003)) and each environment copy ([FUNC-MAIN-001](functional.md#func-main-001)) is its own transmission. Every transmission of a cycle turns the LED on before its first bit, turns it off after its last bit, then waits 30 ms; for an X10 frame, the 30 ms come after the 40 ms gap of its last copy.

---

### PLAT-POWER-005
**Title:** Target MCU is the ATtiny84A at 1 MHz; all radio and delay timings are calibrated for that clock
**Status:** validated
**Dependencies:** [TECH-HAL-001](technical.md#tech-hal-001)
**derives_from:** [TECH-HAL-001](technical.md#tech-hal-001)
**Description:** Every physical build environment runs an ATtiny84A at 1 MHz (`[env:avr_base]`: `board_build.mcu = attiny84a`, `board_build.f_cpu = 1000000L`). The 512 µs, 1024 µs, 400 µs and 30 ms delays are calibrated for this clock only. Verified by inspection of `emitter/platformio.ini` and by the AVR builds.

---

### PLAT-POWER-006
**Title:** With USE_SWITCHED_5V, PB2 is driven low only inside the sensor power window to switch the off-board 5 V module
**Status:** validated
**Dependencies:** [PLAT-POWER-002](#plat-power-002), [PLAT-POWER-003](#plat-power-003), [TECH-HAL-004](technical.md#tech-hal-004)
**derives_from:** [FUNC-ANALOG-001](functional.md#func-analog-001)
**Description:** With `USE_SWITCHED_5V` (AVR build), PB2 (J8) switches the off-board 5 V module through a PNP (`hardware/tinysensor/VARIANTE_LM35.md`). PB2 is driven low while the sensors are powered ([PLAT-POWER-002](#plat-power-002)) and is an input without pull-up, in high impedance, the rest of the time, hibernate included. PB2 is never driven high. Without the flag, PB2 behaviour is unchanged. Verified by inspection and on the bench (AVR only, no native test).

---

### PLAT-POWER-007
**Title:** Every sensor board pulls its sensor buses (SCL on PA4, SDA on PA6, 1-Wire on PA3) up with external resistors to the switched sensor rail SENSOR_VCC (PA2)
**Status:** validated
**Dependencies:** [PLAT-POWER-002](#plat-power-002)
**derives_from:** [FUNC-SENSOR-001](functional.md#func-sensor-001)
**Description:** Every board running this firmware has external pull-ups on its sensor buses, to `SENSOR_VCC`, the sensor rail switched by PA2: SCL (PA4) and SDA (PA6) for the I2C bus, and the 1-Wire bus (PA3), which serves the DS18B20. The firmware enables no internal pull-up on these pins ([TECH-HAL-004](technical.md#tech-hal-004)), so a board without them has no working I2C bus. On v3.0 they are three 4.7 kΩ resistors: R3 on SCL, R2 on SDA, R6 on 1-Wire. Both buses are therefore unpowered whenever PA2 is low ([PLAT-POWER-002](#plat-power-002)). Verified by inspection of the v3.0 schematic (`hardware/tinysensor/tinysensor.kicad_sch`); other board revisions are not checked: bench check pending.

---

### PLAT-DECODE-003
**Title:** Receiver captures radio pulses on ICP1 (PB0/D8 on the Pro Mini, PD4 on the Pro Micro) with Timer1 at 0.5 µs and counts seconds from its overflow
**Status:** validated
**Dependencies:** [TECH-DECODE-002](technical.md#tech-decode-002)
**derives_from:** [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** The receiver runs on an ATmega328P (Pro Mini 5 V / 16 MHz) or an ATmega32U4 (Pro Micro 5 V / 16 MHz); the only difference here is the ICP1 pin: PB0 (Pro Mini D8) or PD4 (Pro Micro pin 4). ICP1 is an input without pull-up: the RXB6 is powered at 5 V and drives it directly, without level shifting. Timer1 is fully reconfigured (Arduino only uses Timer0 for its time base): normal mode, prescaler 8 (0.5 µs ticks), noise canceller on, capture and overflow interrupts on, capture edge opposite to the current ICP1 level. Each capture ends a pulse (low when the captured edge rose), lasting the ticks since the previous capture / 2, or 65535 µs when a whole timer period has elapsed, then flips the edge. Ended pulses go into a 64-pulse buffer, which absorbs the I2C and serial waits; when it is full, the pulse is dropped. Each overflow adds 32768 µs to the time base, which counts one second per 1 000 000 µs. Pulses and time are read with interrupts briefly disabled. Verified by inspection and on the bench on both boards.

---

### PLAT-DECODE-004
**Title:** Receiver drives the SSD1306 128x32 OLED at I2C 0x3C over hardware TWI at 400 kHz, one character cell at a time, without framebuffer
**Status:** validated
**Dependencies:** [TECH-DECODE-002](technical.md#tech-decode-002), [UI-DECODE-002](ui.md#ui-decode-002), [UI-DECODE-003](ui.md#ui-decode-003)
**derives_from:** [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** The OLED is on the hardware TWI at 400 kHz: SDA A4 (PC4) and SCL A5 (PC5) on the Pro Mini, SDA D2 (PD1) and SCL D3 (PD0) on the Pro Micro. At start the receiver sends, in one transaction after control byte `0x00`, the SSD1306 set-up `AE D5 80 A8 1F D3 00 40 8D 14 20 02 A1 C8 DA 02 81 8F D9 F1 DB 40 A4 A6`, writes zeros to the 4 pages × 128 columns, then sends `AF`. An 8x16 character at text page `page` and column `col` is written on display pages `page` and `page + 1`: for each, commands `B0+page`, `col*8 & 0x0F`, `0x10 | (col*8 >> 4)` in one transaction, then `0x40` and the 8 glyph bytes in another. A 16x32 character at column `col` is written on pages 0 to 3: commands `B0+page`, `col*16 & 0x0F`, `0x10 | (col*16 >> 4)`, then `0x40` and the 16 bytes of that page of the big glyph (17 bytes, within the 32-byte I2C buffer of the Arduino core); about 2 ms at 400 kHz, short against the 64-pulse buffer of [PLAT-DECODE-003](#plat-decode-003). No display buffer is kept in RAM. The OLED module has its own regulator and accepts a 5 V supply and 5 V I2C levels, so it is wired directly to the 5 V board, without level shifting. Verified by inspection and on the bench on both boards.

---

### PLAT-DECODE-005
**Title:** Receiver writes RFLink lines on the serial port (debug UART TX D1 on the Pro Mini, USB CDC on the Pro Micro) without blocking the pulse capture
**Status:** validated
**Dependencies:** [TECH-DECODE-002](technical.md#tech-decode-002)
**derives_from:** [TECH-DECODE-002](technical.md#tech-decode-002)
**Description:** On the Pro Mini, the serial port is the hardware USART (TX D1/PD1, 57600 baud 8N1, interrupt-driven 128-byte transmit buffer set by [CONF-DECODE-004](configuration.md#conf-decode-004)): a debug output only, which works with or without a USB-serial adapter connected. On the Pro Micro it is the native USB CDC serial, whose rate is ignored. A line is followed by `\r\n`, and is dropped when the transmit buffer lacks room for it, so the receiver never waits for the serial port to drain, nor for a USB host on the Pro Micro. Incoming bytes are ignored and no startup banner is sent. Verified on the bench with `pio device monitor -b 57600` (through a USB-serial adapter on the Pro Mini).