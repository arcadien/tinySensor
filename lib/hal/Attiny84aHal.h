#pragma once

#ifndef __AVR_ATtiny84A__
#define __AVR_ATtiny84A__ 1
#endif

#include <Hal.h>

#include <stdint.h>

#if defined(USE_I2C)
#include <TinyI2CMaster.h>
#endif

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>

// #define LED_PIN PB1
// #define TX_RADIO_PIN PB0
// #define PWM_PIN PA7
// #define SENSOR_VCC PA2
// #define BAT_SENSOR_PIN PA1

class Attiny84aHal : public Hal {
public:
  Attiny84aHal() {
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

  void LedOn() override { PORTB |= (1 << PB1); }

  void LedOff() override { PORTB &= ~(1 << PB1); }

  void RadioGoLow() override { PORTB &= ~(1 << PB0); }

  void RadioGoHigh() override { PORTB |= (1 << PB0); }

  void Hibernate(uint8_t seconds) override;
  void PowerOnSensors() override {}
  void PowerOffSensors() override {}

  /*!
   *
   * Reads voltage on ADC1 pin, relative to Vcc
   *
   */
  uint16_t GetBatteryVoltageMv(void) {
    // analog ref = VCC, input channel = ADC1
    ADMUX = 0b00000001;

    uint16_t result = adcRead(8, 12);
    uint16_t mvPerAdcStep = GetVccVoltageMv() / ADCSTEPS;
    result *= mvPerAdcStep;
    return result;
  }
  /*!
   *
   * Reads Vcc using internal 1.1v tension reference
   *
   */
  uint16_t GetVccVoltageMv(void) override {
    // analog ref = VCC, input channel = VREF
    ADMUX = 0b00100001;
    uint16_t result = adcRead(8, 12);
    uint16_t vccMv = (uint16_t)(1100.0 * 1024.0 / result);
    return vccMv;
  }


  void inline Delay30ms() override { _delay_ms(30); }
  void inline Delay512Us() override { _delay_us(512); }
  void inline Delay1024Us() override { _delay_us(1024); }

  void InitI2C(void) override {
#if defined(USE_I2C)
    TinyI2C.init();
#endif
  }

private:
  static const uint16_t VREF = 1100l; // internal reference voltage of ADC
  static const uint16_t ADCSTEPS = 1024l;

  void UseLessPowerAsPossible() {
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

  static inline void startADCReading() { ADCSRA |= (1 << ADSC) | (1 << ADEN); }
  static inline bool ADCReadInProgress() {
    return (ADCSRA & (1 << ADSC)) == ADSC;
  }

  /*!
   * Read the ADC, discarding `discard` first readings,
   * and then return average of `samples` readings
   */
  static uint16_t adcRead(uint8_t discard, uint8_t samples) {
    uint16_t result = 0;

    for (uint8_t loopSamples = 0; loopSamples < (samples + discard);
         ++loopSamples) {
      startADCReading();
      while (ADCReadInProgress()) {
      }
      if (loopSamples > discard)
        result += ADC;
    }
    result /= samples;
    return result;
  }
};
