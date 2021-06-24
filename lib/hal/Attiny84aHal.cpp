#ifdef AVR
#include <Attiny84aHal.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>


volatile uint8_t sleep_interval;

ISR(BADISR_vect) {
  while (1) {
    PORTB |= _BV(PORTB1);
    _delay_ms(100);
    PORTA |= _BV(PORTB1);
    _delay_ms(100);
  }
}


//
// * Interrupt routine called each 8 seconds in the
// * default setup, which is the longest timeout period
// * accepted by the hardware watchdog
//
ISR(WATCHDOG_vect) {
  wdt_reset();
  ++sleep_interval;
  // Re-enable WDT interrupt
  _WD_CONTROL_REG |= (1 << WDIE);
}

/*
 * Puts MCU to sleep for specified number of seconds
 *
 * As the hardware watchdog is set for timeout after 8s,
 * the value used here will be divided by 8 (and rounded if needed).
 * It is better to use a multiple of 8 as value.
 */
void sleep(uint8_t s) {
  s >>= 3; // or s/8
  if (s == 0)
    s = 1;
  sleep_interval = 0;
  while (sleep_interval < s) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();

    uint8_t PRR_backup = PRR;
    uint8_t porta_backup = PORTA;
    uint8_t portb_backup = PORTB;
    uint8_t ddra_backup = DDRA;
    uint8_t ddrb_backup = DDRB;

    PRR |= (1 << PRUSI) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRADC);

    // all pins as input to avoid power draw
    DDRA = 0;
    DDRB = 0;

    // pull-up all unused pins by default
    PORTA |= 0b01110000;

    // pullup reset only
    // ( only 4 lasts are available - p. 67)
    PORTB |= 0b00001000;

    sleep_mode();
    // here the system is sleeping

    // here the system wakes up
    sleep_disable();

    // restore
    PRR = PRR_backup;
    DDRA = ddra_backup;
    DDRB = ddrb_backup;
    PORTA = porta_backup;
    PORTB = portb_backup;
  }
}

void Attiny84aHal::Hibernate(uint8_t seconds) { sleep(seconds); }
#endif