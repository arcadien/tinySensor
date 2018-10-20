# TinySensor

An AVR project targetting creation of a SDK for low-power sensors wireless nodes using ATTinyX4 MCU from Microchip (formely Atmel) 
and communicating using 433Mhz.

## Supported AVR targets
* ATTiny84a

## Supported sensors
* [Bosch BME/BMP280](https://www.bosch-sensortec.com/bst/products/all_products/bme280)

## Used libraries
Various libraries are used here, and some where tweaked to remove any Arduino dependencies, making the whole firmware more compact:
* Oregon(c) library from [connectingstuff.net](http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/)
* Sparkfun [BME280 sensor library](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library) with all SPI related code removed
* [USI wire library](https://github.com/puuu/USIWire.git) with little addition for ATTiny84A support


## Useful documentation about low power AVR
* [AVR4013: picoPower Basics](http://www.microchip.com/downloads/en/AppNotes/doc8349.pdf)
