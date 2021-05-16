#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>

// #define ADMUX_VREF 0x21
// #if not defined(INTERNAL_1v1)
// #define INTERNAL_1v1 1100l // internal reference voltage of ADC
// #endif

// #define ADCMAX  1024

uint16_t readVcc(void);
uint16_t readBatteryVoltage(void);

// uint16_t adcRead(uint8_t discard, uint8_t samples, uint8_t admux)
// {
//   uint16_t result = 0;

//   // enable ADC
//   PRR &= ~(1 << PRADC);

//   // prescaler of 16 = 1MHz/16 = 62.5KHz.
//   ADCSRA = (1 << ADPS2);

//   ADMUX = admux;

//   ADCSRA |= (1 << ADEN);

//   // need at least 1ms delay for vref to settle
//   // see datasheet p. 151
//   // The reference may have a very high impedance,
//   // so it need several conversions to charge
//   _delay_ms(2);

//   while (discard > 0)
//   {
//     ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
//     while ((ADCSRA & (1 << ADSC)) != 0)
//       ; // wait until ADSC is clear
//     discard--;
//   }

//   for (uint8_t loopSamples = samples; loopSamples > 0; loopSamples--)
//   {
//     ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
//     while ((ADCSRA & (1 << ADSC)) != 0)
//       ; // wait until ADSC is clear

//     result += ADC;
//   }
//   result /= samples;

//   ADCSRA &= ~(1 << ADEN);
//   PRR |= (1 << PRADC);

//   return result;
// }

// /*!
//  *
//  * Reads Vcc using internal 1.1v tension reference
//  *
//  */
// uint16_t readVcc(void)
// {

//   // Reference : VCC, single end: 1.1v internal ref
//   uint8_t admux = (1 << MUX5) | (1 << MUX0);

//   uint16_t result = adcRead(5, 5, admux);

//   // for 3.3v, ADC should be around 341
//   // for 5v, ADC should be around 225

//   // now to convert to vcc. Work in millivolts
//   // adc = (VIn*1024)/vRef
//   // adc = (INTERNAL_1v1*ADCMAX)/vcc
//   // vcc = (INTERNAL_1v1*ADCMAX)/adc
//   uint32_t intermediate = (ADCMAX * INTERNAL_1v1) / result;

//   result = intermediate & 0xffff;
//   return result;
// }

// /*!
//  *
//  * Reads voltage on ADC1 pin, relative to Vcc
//  *
//  */
// uint16_t readBatteryVoltage(void)
// {

//   // Read on PA1/ADC1, ref VCC
//   uint8_t admux = (1 << MUX0);

//   uint16_t result = adcRead(5, 5, admux);

//   float mvPerAdcStep = ADCMAX / (float)readVcc();
//   result /= mvPerAdcStep;

//   return (result);
// }
