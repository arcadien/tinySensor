#ifdef AVR
#include <readVcc.h>
#include <avr/io.h>

static const uint16_t VREF = 1100l;// internal reference voltage of ADC
static const uint16_t ADCMAX =1024l;

static inline void startADCReading() { ADCSRA |= (1 << ADSC); }
static inline bool ADCreadingAvailable() { return (ADCSRA & (1 << ADSC)); }

		
/*!
* Read the ADC, discarding `discard` first readings,
* and then return average of `samples` readings
*/
static uint16_t adcRead(uint8_t discard, uint8_t samples) {
  uint16_t result = 0;

  for (uint8_t loopSamples = 0; loopSamples < (samples + discard);
       ++loopSamples) {
    startADCReading();
    while (!ADCreadingAvailable()) {}
    if (loopSamples > discard)
      result += ADC;
  }
  result /= samples;
  return result;
}

/*!
 *
 * Reads Vcc using internal 1.1v tension reference
 *
 */
uint16_t readVcc(void) {
  // analog ref = VCC, input channel = VREF
  ADMUX =0b00100001;

  // wait for ADC to settle
  _delay_ms(2);        

  uint16_t result = adcRead(8, 8);

  // now to convert to vcc. Work in millivolts
  // adc = 1024*vref/vcc. Therefore vcc = 1024*vref/adc
  uint32_t intermediate = (ADCMAX * VREF) / result;
  // result = intermediate & 0xffff;
  
  return (intermediate);
}

/*!
 *
 * Reads voltage on ADC1 pin, relative to Vcc
 *
 */
uint16_t readBatteryVoltage(void) {

  // analog ref = VCC, input channel = ADC1
  ADMUX =0b00000001;
  
  _delay_ms(2);         // need a 2ms delay for vref to settle

  uint16_t result = adcRead(8, 8);

  float mvPerAdcStep = ADCMAX / (float)readVcc();
  result /= mvPerAdcStep;
  return (result);
}

#endif