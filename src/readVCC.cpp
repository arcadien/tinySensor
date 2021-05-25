#if !defined(NATIVE)

#include <readVcc.h>
#include <avr/io.h>

static const uint16_t VREF = 1100l;// internal reference voltage of ADC
static const uint16_t ADCSTEPS =1024l;

static inline void startADCReading() { ADCSRA |= (1 << ADSC) | (1 << ADEN); }
static inline bool ADCReadInProgress() { return (ADCSRA & (1 << ADSC)) == ADSC; }

		
/*!
* Read the ADC, discarding `discard` first readings,
* and then return average of `samples` readings
*/
static uint16_t adcRead(uint8_t discard, uint8_t samples) {
  uint16_t result = 0;

  for (uint8_t loopSamples = 0; loopSamples < (samples + discard);
       ++loopSamples) {
    startADCReading();
    while(ADCReadInProgress()) {}
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
float readVccMv(void) {
  // analog ref = VCC, input channel = VREF
  ADMUX =0b00100001;

  uint16_t result = adcRead(8, 12);
  
  float vccMv =  1100.0 * 1024.0 / result;
  
  return vccMv;
}

/*!
 *
 * Reads voltage on ADC1 pin, relative to Vcc
 *
 */
uint16_t readBatteryVoltage(void) {

  // analog ref = VCC, input channel = ADC1
  ADMUX =0b00000001;
  
  uint16_t result = adcRead(8, 12);

  float mvPerAdcStep =  readVccMv() / ADCSTEPS;
  result *= mvPerAdcStep;
  return (result);
}

#endif