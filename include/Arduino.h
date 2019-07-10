#pragma once

#if defined(__AVR_ATtiny84A__)
#define USI_DDR_PORT DDRA
#define USI_SCK_PORT DDRA
#define USCK_DD_PIN DDA4
#define DO_DD_PIN DDA5
#define DI_DD_PIN DDA6
#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define PORT_USI_SDA PORTA6
#define PORT_USI_SCL PORTA4
#define PIN_USI_SDA PINA6
#define PIN_USI_SCL PINA4
#define USI_START_VECTOR USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#define DDR_USI_CL DDR_USI
#define PORT_USI_CL PORT_USI
#define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#define USI_START_COND_INT USISIF
#endif
#endif
