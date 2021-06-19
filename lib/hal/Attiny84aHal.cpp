
#include <Attiny84aHal.h>

void Attiny84aHal::DelayUs(const long us) const {
#if defined(__AVR__)
  _delay_us(us);
#endif
}

void Attiny84aHal::LedOn() const { PORTB |= (1 << LED_PIN); }

void Attiny84aHal::LedOff() const { PORTB &= ~(1 << LED_PIN); }

void Attiny84aHal::RadioGoLow() const { PORTB &= ~(1 << TX_RADIO_PIN); }

void Attiny84aHal::RadioGoHigh() const { PORTB |= (1 << TX_RADIO_PIN); }