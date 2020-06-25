#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>

#define ADMUX_VREF 0x21
#define VREF 1100l // internal reference voltage of ADC
#define ADCMAX 1024l

uint16_t adcRead(uint8_t discard, uint8_t samples)
{
	uint16_t result = 0;

	while (discard > 0)
	{
		ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
		while ((ADCSRA & (1 << ADSC)) != 0)
			; // wait until ADSC is clear
		discard--;
	}

	for (uint8_t loopSamples = samples; loopSamples > 0; loopSamples--)
	{
		ADCSRA |= (1 << ADSC); // start conversion by writing 1 to ADSC
		while ((ADCSRA & (1 << ADSC)) != 0)
			; // wait until ADSC is clear
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
uint16_t readVcc(void)
{
	// ADC setup
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADEN); // prescaler of 64 = 8MHz/64 = 125KHz.
	ADMUX = ADMUX_VREF;									// 0x21 = analogue ref = VCC, input channel = VREF
	_delay_ms(2);										// need a 2ms delay for vref to settle

	uint16_t result = adcRead(8, 8);

	// now to convert to vcc. Work in millivolts
	// adc = 1024*vref/vcc. Therefore vcc = 1024*vref/adc
	uint32_t intermediate = (ADCMAX * VREF) / result;
	result = intermediate & 0xffff;
	ADCSRA = 0; // turn off ADC
	return (result);
}

/*!
*
* Reads voltage on ADC1 pin, relative to Vcc
*
*/
uint16_t readBatteryVoltage(void)
{
	// ADC setup
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADEN); // prescaler of 64 = 8MHz/64 = 125KHz.
	ADMUX = (1 << MUX0);								// Read on ADC1
	_delay_ms(2);										// need a 2ms delay for vref to settle

	uint16_t result = adcRead(8, 8);

	float mvPerAdcStep = ADCMAX / (float)readVcc();
	result /= mvPerAdcStep;

	ADCSRA = 0; // turn off ADC

	return (result);
}