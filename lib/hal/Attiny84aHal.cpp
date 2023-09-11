#ifdef AVR
#include <AnalogFilter.h>
#include <Attiny84aHal.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

// #define LED_PIN PB1
// #define TX_RADIO_PIN PB0
// #define PWM_PIN PA7
// #define SENSOR_VCC PA2
// #define BAT_SENSOR_PIN PA1

volatile uint8_t sleep_interval;

ISR(BADISR_vect) {
  while (1) {
    PORTB |= (1 << PB1);
    _delay_ms(100);
    PORTB &= ~(1 << PB1);
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

static void UseLessPowerAsPossible() {
  // AVR4013: picoPower Basics
  // unused pins should be set as
  // input + pullup to minimize consumption
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  //
  PORTA |= 0b01111001;
  PORTB |= 0b11111100;

  // no timer1
  PRR &= ~_BV(PRTIM1);

  // no analog p. 129, 146, 131
  ADCSRA = 0;

  // better, but not easy to invert for VCC sensing
  // PRR &= ~_BV(PRADC);
  // ADCSRA &= ~(1 << ADEN);
  // ACSR |= (1 << ACD);
  // DIDR0 |= (1 << ADC2D) | (1 << ADC1D); // buffers

  // deactivate brownout detection during sleep (p.36)
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR |= (1 << BODS);
  MCUSR &= ~(1 << BODSE);
}

static inline void startADCReading() { ADCSRA |= (1 << ADSC); }
static inline bool ADCReadInProgress() { return (ADCSRA & (1 << ADSC)) == ADSC; }
/*!
 * Read the ADC, discarding `discard` first readings,
 * and then return average of `samples` readings
 */
static uint16_t adcRead(uint8_t discard, uint8_t samples) {
  AnalogFilter filter(discard, samples);

  for (uint8_t loopSamples = 0; loopSamples < (samples + discard);
       ++loopSamples) {
    startADCReading();
    while (ADCReadInProgress()) {
    }
    filter.Push(ADC);
  }
  return filter.Get();
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

Attiny84aHal::Attiny84aHal() {
  PORTB = 0;
  UseLessPowerAsPossible();

  // Watchdog setup - 8s sleep time
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);

  // set output pins
  DDRB |= _BV(PB0); // TX_RADIO_PIN
  DDRB |= _BV(PB1); // LED_PIN
  DDRA |= _BV(PA2); // sensor VCC
}

void Attiny84aHal::PowerOnSensors() { PORTA |= _BV(PA2); }

void Attiny84aHal::PowerOffSensors() { PORTA &= ~_BV(PA2); }

void inline Attiny84aHal::LedOn() { PORTB |= (1 << PB1); }

void Attiny84aHal::LedOff() { PORTB &= ~(1 << PB1); }

void Attiny84aHal::RadioGoLow() { PORTB &= ~(1 << PB0); }

void Attiny84aHal::RadioGoHigh() { PORTB |= (1 << PB0); }

void Attiny84aHal::Delay30ms() { _delay_ms(30); }
void Attiny84aHal::Delay400Us() { _delay_us(400); }
void Attiny84aHal::Delay512Us() { _delay_us(512); }
void Attiny84aHal::Delay1024Us() { _delay_us(1024); }
void Attiny84aHal::Delay1s() { _delay_ms(1000); }

/*!
 *
 * Reads voltage on ADC1 pin, relative to Vcc
 *
 */
uint16_t Attiny84aHal::GetBatteryVoltageMv(void) {
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	
	// analog ref = VCC, input channel = ADC1 (PA1)
	ADMUX = 0b00000001;
	_delay_ms(1);

	uint16_t batteryAdcRead = adcRead(4, 12);
	uint16_t vccVoltageMv = GetVccVoltageMv();
	float mvPerAdcStep = (vccVoltageMv / 1024.f);
	uint16_t batteryVoltageMv = (uint16_t)(batteryAdcRead * mvPerAdcStep);
	return batteryVoltageMv;
}

/*!
 *
 * Reads internal 1.1v tension with VCC reference
 *
 */
uint16_t Attiny84aHal::GetVccVoltageMv(void) {
	
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	ADMUX = 0b00100001;
	_delay_ms(1);

	uint16_t vccAdcRead = adcRead(4, 12);
	uint16_t vccMv = (uint16_t)((INTERNAL_1v1 * 1024.f) / vccAdcRead);
	return vccMv;
}

void Attiny84aHal::Hibernate(uint8_t seconds) { sleep(seconds); }

void Init(void) {
#if defined(USE_I2C)
  TinyI2C.init();
#endif
}

#endif