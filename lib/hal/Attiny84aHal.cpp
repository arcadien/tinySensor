#if defined(__AVR_ATtiny84a__)

#include <Hal.h>
#include <avr/io.h>


void ATtiny84aHal::LedOn() const
{
    PORTB |= (1 << PB1);
}

void ATtiny84aHal::RadioGoLow() const
{
    PORTB &= ~(1 << PB0);
}

void ATtiny84aHal::RadioGoHigh() const
{
    PORTB |= (1 << PB0);
}

#endif