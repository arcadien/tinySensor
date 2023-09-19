#ifdef AVR
#include <AnalogFilter.h>
#include <Attiny84aHal.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define LED_PIN PB1
#define TX_RADIO_PIN PB0
#define DIGITAL_OUT PA7
#define SENSOR_VCC PA2
#define BAT_SENSOR_PIN PA1

volatile uint8_t sleep_interval;

ISR(BADISR_vect) {
  while (1) {
    PORTB |= _BV(LED_PIN);
    _delay_ms(100);
    PORTB &= ~_BV(LED_PIN);
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
  _WD_CONTROL_REG |= _BV(WDIE);
}

static void UseLessPowerAsPossible() {
  // AVR4013: picoPower Basics
  // unused pins should be set as
  // input + pullup to minimize consumption
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  PORTA = 0b01111000;
  PORTB = 0b11111100;

  // Analog comparator power down
  ACSR |= _BV(ACD);

  // no timer1
  PRR &= ~_BV(PRTIM1);

  // no analog p. 129, 146, 131
  ADCSRA = 0;
  PRR &= ~_BV(PRADC);

  // better, but not easy to invert for VCC sensing
  DIDR0 |= _BV(ADC2D) | _BV(ADC1D); // digital buffers

  // deactivate brownout detection during sleep (p.36)
  MCUCR |= _BV(BODS) | _BV(BODSE);
  MCUCR |= _BV(BODS);
  MCUSR &= ~_BV(BODSE);
}

/*
 * Puts MCU to sleep for specified number of seconds
 *
 * As the hardware watchdog is set for timeout after 8s,
 * the value used here will be divided by 8 (and rounded if needed).
 * It is better to use a multiple of 8 as value.
 */
void sleep(uint16_t s) {
  s >>= 3; // or s/8
  if (s == 0)
    s = 1;
  sleep_interval = 0;

  uint8_t PRR_backup = PRR;
  uint8_t porta_backup = PORTA;
  uint8_t portb_backup = PORTB;
  uint8_t ddra_backup = DDRA;
  uint8_t ddrb_backup = DDRB;

  PRR |= _BV(PRUSI) | _BV(PRTIM0) | _BV(PRTIM1) | _BV(PRADC);

  // all pins as input to avoid power draw
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  PORTA |= 0b01110000;

  // pullup reset only
  // ( only 4 lasts are available - p. 67)
  PORTB |= 0b00001000;

  while (sleep_interval < s) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();

    sleep_mode();
    // here the system is sleeping
    // here the system wakes up
    sleep_disable();
  }
  // restore
  PRR = PRR_backup;
  DDRA = ddra_backup;
  DDRB = ddrb_backup;
  PORTA = porta_backup;
  PORTB = portb_backup;
}

Attiny84aHal::Attiny84aHal() {

  UseLessPowerAsPossible();

  // Watchdog setup - 8s sleep time
  _WD_CONTROL_REG |= _BV(WDCE) | _BV(WDE);
  _WD_CONTROL_REG = _BV(WDP3) | _BV(WDP0) | _BV(WDIE);

  // ADC sampling time setup
  ADCSRA = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);

  // set output pins
  DDRB |= _BV(TX_RADIO_PIN);
  DDRB |= _BV(LED_PIN);
  DDRA |= _BV(SENSOR_VCC);
  DDRA |= _BV(DIGITAL_OUT);
}

void Attiny84aHal::PowerOnSensors() { PORTA |= _BV(SENSOR_VCC); }

void Attiny84aHal::PowerOffSensors() { PORTA &= ~_BV(SENSOR_VCC); }

void inline Attiny84aHal::LedOn() { PORTB |= (1 << LED_PIN); }

void Attiny84aHal::LedOff() { PORTB &= ~(1 << LED_PIN); }

void Attiny84aHal::RadioGoLow() { PORTB &= ~(1 << TX_RADIO_PIN); }

void Attiny84aHal::RadioGoHigh() { PORTB |= (1 << TX_RADIO_PIN); }

inline void Attiny84aHal::SerialGoHigh() { PORTA |= (1 << DIGITAL_OUT); };

inline void Attiny84aHal::SerialGoLow() { PORTA &= ~(1 << DIGITAL_OUT); };

void Attiny84aHal::Delay30ms() { _delay_ms(30); }
void Attiny84aHal::Delay400Us() { _delay_us(400); }
void Attiny84aHal::Delay512Us() { _delay_us(512); }
void Attiny84aHal::Delay1024Us() { _delay_us(1024); }
void Attiny84aHal::Delay1s() { _delay_ms(1000); }

static uint16_t AdcRead(Hal &hal, uint8_t admux) {

  PRR &= ~_BV(PRADC);
  ADCSRA |= _BV(ADEN);
  ADMUX = admux;
  _delay_ms(10);

  static const uint8_t IGNORED_SAMPLES = 8;
  static const uint8_t COUNTED_SAMPLES = 16;
  uint32_t accumulator = 0;
  uint8_t loopSamples = 0;

  for (loopSamples = 0; loopSamples < IGNORED_SAMPLES; loopSamples++) {
    ADCSRA |= _BV(ADSC);
    //loop_until_bit_is_clear(ADCSRA, ADSC);
    while(!bit_is_set(ADCSRA,ADIF));
    ADCSRA |= _BV(ADIF); 
    accumulator += ADC;
  }
  accumulator = 0;
  for (loopSamples = 0; loopSamples < COUNTED_SAMPLES; loopSamples++) {
    ADCSRA |= _BV(ADSC);
    //loop_until_bit_is_clear(ADCSRA, ADSC);
    while(!bit_is_set(ADCSRA,ADIF));
    ADCSRA |= _BV(ADIF); 
    accumulator += ADC;
  }
  ADCSRA &= ~_BV(ADEN);
  PRR |= _BV(PRADC);
  return (uint16_t)(accumulator / COUNTED_SAMPLES);
}

uint16_t Attiny84aHal::GetRawBattery(void) {
  return AdcRead(*this, _BV(MUX0)); // PA1
}

uint16_t Attiny84aHal::GetRawAnalogSensor() {
  return AdcRead(*this, 0x00); // PA0
}

uint16_t Attiny84aHal::GetRawInternal11Ref(void) {
  // measure 1.1v internal ref against VCC
  return AdcRead(*this, _BV(MUX0) | _BV(MUX5));
}

void Attiny84aHal::Hibernate(uint16_t seconds) { sleep(seconds); }

void Init(void) {
#if defined(USE_I2C)
  TinyI2C.init();
#endif
}

#endif