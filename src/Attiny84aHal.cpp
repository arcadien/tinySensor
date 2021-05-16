#if defined(AVR)

#include <Attiny84aHal.h>
#include <hal.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <config.h>

void ATTiny84aHal::LedOn()
{
			PORTB |= (1 << LED_PIN);
}

void ATTiny84aHal::RadioGoLow() 
{
    PORTB &= ~(1 << PB0);
}

void ATTiny84aHal::RadioGoHigh() 
{
    PORTB |= (1 << PB0);
}

#endif