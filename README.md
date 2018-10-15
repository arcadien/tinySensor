TinySensor
==========

An AVR project targetting creation of a SDK for low-power sensors wireless nodes using ATTinyX4 MCU from Microchip (formely Atmel) and communicating using 433Mhz.

Various libraries are used here, and some where tweaked to remove any Arduino dependencies, making the whole firmware more compact.
* Oregon(c) library from [[http://www.connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/|connectingstuff.net]]
* Sparkfun [[https://github.com/sparkfun/SparkFun_BME280_Arduino_Library|library for BME280 sensor]] (with all SPI related code removed)
* USI wire library from https://github.com/puuu/USIWire.git (little addition for ATTiny84A support)
