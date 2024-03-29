# TinySensor 

[![Travis build status](https://api.travis-ci.org/arcadien/tinySensor.svg?branch=master)](https://travis-ci.org/arcadien/tinySensor)

TinySensor aims to provide a customisable firmware and a hardware design for low-power environment sensors using wireless communication.

In the complete system, data is processed by a [Domoticz](https://domoticz.com/)/[RFlink](http://www.rflink.nl/blog2/) couple.
Data decoding is also tested using [RTL_433](https://github.com/merbanan/rtl_433) decoder.
Large part of the code, especially wireless encoders, are unit-tested.

## Supported MCUs
* ATTiny84a
* ATMega328p (ongoing)

## Supported sensors
* I2C sensors (eg. [Bosch BME/BMP280](https://www.bosch-sensortec.com/bst/products/all_products/bme280))
* Analog sensors (eg. [LM35](http://www.ti.com/lit/ds/symlink/lm35.pdf))
* OneWire sensors (eg. [Dallas DS18S20](https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf))

## Wireless technology
* "one pin" emmiters. Tested with low-cost 433Mhz emmitters.

## Wireless protocol
* Oregon(c) for environment measurement transmission
* X10(c) for (optional) battery level transmission
* Lacrosse Technology for enviroment measurement transmission

## Used libraries
Various libraries are used here, and some where tweaked to remove any Arduino dependencies, making the whole firmware more compact:
* Sparkfun [BME280 sensor library](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library) with all SPI related code removed
* [Tiny I2C](https://github.com/arcadien/tiny-i2c.git) of [David Johnson-Davies](https://github.com/technoblogy)
* [Unity test harness](http://www.throwtheswitch.org/unity) from Throw The Switch, for all testing code
* DS18B20 library of [Jacek Wieczorek](https://github.com/Jacajack/avr-ds18b20)

## Firmware builder
A web application wrapped in a Docker container allow to build the firmware in a sealed environment. NOTE: It is at his early stage!

## Useful documentation about low power AVR
* [AVR4013: picoPower Basics](http://www.microchip.com/downloads/en/AppNotes/doc8349.pdf)

## Development

TinySensor build environment now uses [PlatformIO](https://platformio.org/).

## Hardware options

TinySensor can use combination of sensors, the limit comes from amount of flash memory on the ATTiny.

### DS18B20

OneWire sensor, cheap and efficient.

### BMP/BME 280 environment sensor
The [BMP280](https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/pressure-sensors-bmp280-1.html) and the [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) share the same library, but the BME provide humidity sensing, and thus use more memory.

### Analog sensor

Analog sensor can be used for [LM35](https://www.ti.com/lit/ds/symlink/lm35.pdf) temperature sensor, or [LDR](https://en.wikipedia.org/wiki/Photoresistor).

### LM35 temperature sensor

#### Without negative temperature support
A 0v ground reference is needed.
Solder pads *2* and *3* of **J4**, (rear side) as well **JP3**. 

#### With negative temperature support
In the case of a *-CAZ* version, temperature below 0 can be measured. 
To allow this, two 1N914 diodes must be placed to put the sensor reference below 0v. This setup is documented in the LM35 datasheet.
On TinySensor board v2.1 and up, this can be implemented by:
* solding **D2** and **D3** diodes
*solding **R4** with a value of 18k
* solding **JP3** to bypass C3 capacitor
* solding pads *1* and *2* of **J4** (rear side) to set ground reference.

#### Notes about swing

You can add a filter capacitor *C3* to avoid swing if the wire is long. In this case, do not solder *JP3*.
See datasheet for details.

# LDR
Use the right value for **R4** (should match the used LDR) and solder **JP3** and, on **JP4**, middle and *3* pads. This way, there will be no RC filter (no cap because of **J3**) and Vss analog reference thanks of **JP4**.
