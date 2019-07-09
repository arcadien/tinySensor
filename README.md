# TinySensor ![Travis build status](https://api.travis-ci.org/arcadien/tinySensor.svg?branch=master "Travis build status")


TinySensor aims to provide a customisable firmware and board design for low-power environment sensors using wireless communication.

In the complete system, data is received by a [Domoticz](https://domoticz.com/)/[RFlink](http://www.rflink.nl/blog2/) couple.

## Supported MCUs
* ATTiny8xa family

## Supported sensors
* I2C sensors (eg. [Bosch BME/BMP280](https://www.bosch-sensortec.com/bst/products/all_products/bme280))
* Analog sensors (eg. [LM35](http://www.ti.com/lit/ds/symlink/lm35.pdf))
* OneWire sensors (eg. [Dallas DS18S20](https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf))

## Wireless technology
* "one pin" emmiters. Tested with low-cost 433Mhz emmitters.

## Wireless protocol
* Oregon(c)

## Used libraries
Various libraries are used here, and some where tweaked to remove any Arduino dependencies, making the whole firmware more compact:
* Oregon(c) library from [connectingstuff.net](http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/)
* Sparkfun [BME280 sensor library](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library) with all SPI related code removed
* [USI wire library](https://github.com/puuu/USIWire.git) with little addition for ATTiny84A support


## Useful documentation about low power AVR
* [AVR4013: picoPower Basics](http://www.microchip.com/downloads/en/AppNotes/doc8349.pdf)

## Development
Sample command for project setup using Atmel Studio, and the Git Bash console below.
It setups project for a ATTiny84a mcu running at 1Mhz.

```bash
mkdir build_avr
cd build_avr
export MCU=attiny84a
PATH="$PATH:/d/embedded/Atmel/Studio/7.0/toolchain/avr8/avr8-gnu-toolchain/bin/:/d/embedded/Atmel/Studio/7.0/shellutils/"  \
PATH="/c/dev/tools/cmake-3.14.3-win64-x64/bin/cmake:$PATH" \
	cmake  ../ \
	-G "Unix Makefiles" \
	-DMCU_SPEED=1000000U \
	-DAVR_MCU=$MCU \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=../third_party/cmake-avr/generic-gcc-avr.cmake

cmake --build .

```
