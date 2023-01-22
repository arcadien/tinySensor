#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>

#define ADMUX_VREF 0x21
#if not defined(INTERNAL_1v1)
#define INTERNAL_1v1 1100l // internal reference voltage of ADC
#endif
#define ADCMAX 1024l

uint16_t adcRead(uint8_t discard, uint8_t samples) {
  uint16_t result = 0;

  // enable ADC
  PRR &= ~(1 << PRADC);

  // prescaler of 16 = 1MHz/16 = 62.5KHz.
  ADCSRA = (1 << ADPS2);
  ADCSRA |= (1 << ADEN);

  while (discard > 0) {
    ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
    while ((ADCSRA & (1 << ADSC)) != 0)
      ; // wait until ADSC is clear
    discard--;
  }

  for (uint8_t loopSamples = samples; loopSamples > 0; loopSamples--) {
    ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
    while ((ADCSRA & (1 << ADSC)) != 0)
      ; // wait until ADSC is clear

    result += ADC;
  }
  result /= samples;

  ADCSRA &= ~(1 << ADEN);
  PRR |= (1 << PRADC);

  return result;
}

/*!
 *
 * Reads Vcc using internal 1.1v tension reference
 *
 */
uint16_t readVcc(void) {

  // Reference : VCC, single end: 1.1v internal ref
  ADMUX = (1 << MUX5) | (1 << MUX0);

  // need at least 1ms delay for vref to settle
  // see datasheet p. 151
  // The reference has a very high impedance,
  // so it need several conversions to charge
  _delay_ms(2);

  uint16_t result = adcRead(5, 5);

  // for 3.3v, ADC should be around 341
  // for 5v, ADC should be around 225

  // now to convert to vcc. Work in millivolts
  // adc = (VIn*1024)/vRef
  // adc = (vcc*1024)/vRef
  // adc = (INTERNAL_1v1*ADCMAX)/vcc
  // vcc = (INTERNAL_1v1*ADCMAX)/adc
  uint32_t intermediate = (ADCMAX * INTERNAL_1v1) / result;

  result = intermediate & 0xffff;
  return result;
}

/*!
 *
 * Reads voltage on ADC1 pin, relative to Vcc
 *
 */
uint16_t readBatteryVoltage(void) {

  // Read on PA1/ADC1
  ADMUX = (1 << MUX0);

  uint16_t result = adcRead(5, 5);

  float mvPerAdcStep = ADCMAX / (float)readVcc();
  result /= mvPerAdcStep;

  return (result);
}