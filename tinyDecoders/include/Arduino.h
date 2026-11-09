#pragma once

// TinyI2C includes <Arduino.h> for the USI pin names only; the attiny84a env
// has no framework, so this gives the ATtiny84 ones (USI: SDA PA6, SCL PA4).
#include <avr/io.h>

#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define PIN_USI_SDA PINA6
#define PIN_USI_SCL PINA4
#define DDR_USI_CL DDR_USI
#define PORT_USI_CL PORT_USI
#define PIN_USI_CL PIN_USI
