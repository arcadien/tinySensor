
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "bh1750.h"

#define INTERNAL_1v1 1100

/**
* @brief Accumulate and mean values with exclusion behavior
*
*/
class AnalogFilter {

	private:
	uint8_t exclusion;
	uint8_t samples;
	uint8_t actualValuesCount;
	uint16_t accumulator;

	public:
	/**
	* Create a filter able to accumulate up to 'samples' values (max 255), and
	* ignore 'exclusion' first values
	*/
	AnalogFilter(uint8_t exclusion, uint8_t samples)
	: exclusion(exclusion), samples(samples), actualValuesCount(0),
	accumulator(0) {}

	/**
	* @brief Push a value in the accumulator
	*
	* @param value
	*/
	void Push(uint16_t value);

	/**
	* @brief return current accumulator value divided by expected sample count
	*
	* @return uint16_t
	*/
	uint16_t Get();
};

void AnalogFilter::Push(uint16_t value) {
	if (exclusion == 0) {
		if (actualValuesCount < (samples + exclusion)) {
			accumulator += value;
			actualValuesCount++;
		}
		} else {
		exclusion--;
	}
}

uint16_t AnalogFilter::Get() {
	uint16_t result = 0;
	if (actualValuesCount > 0) {
		result = (uint16_t)(accumulator / actualValuesCount);
	}
	return result;
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

/*!
*
* Reads internal 1.1v tension with VCC reference
*
*/
uint16_t GetVccVoltageMv(void) {

	ADCSRA |= (1 << ADEN) | (1 << ADPS2)| (1 << ADPS1)| (1 << ADPS0);
	ADMUX = 0b00100001;
	_delay_ms(10);
	
	uint16_t vccAdcRead = adcRead(4, 12);
	uint16_t vccMv = (uint16_t)((INTERNAL_1v1 * 1024.f) / vccAdcRead);
	return vccMv;
}


static uint16_t GetAnalogMv(uint8_t admux) {

	uint16_t vccVoltageMv = GetVccVoltageMv();
	ADCSRA |= (1 << ADEN) | (1 << ADPS2)| (1 << ADPS1)| (1 << ADPS0);
	ADMUX = admux;
	_delay_ms(10);

	uint16_t adcReading = adcRead(4, 12);
	float mvPerAdcStep = (vccVoltageMv / 1024.f);
	uint16_t valueInMv = (uint16_t)(adcReading * mvPerAdcStep);
	ADCSRA &= ~(1 << ADEN);
	return valueInMv;
}

uint16_t GetBatteryVoltageMv(void) {

	return GetAnalogMv(0b00000001);
}

uint16_t GetAnalog1VoltageMv() {
	return GetAnalogMv(0b00000000);
}

#define _BV(bit) (1 << (bit))

int main(){

	PORTA = 0b01111000;
	
	DDRA |= _BV(PA2); // sensor VCC
	PORTA |= _BV(PA2);

	TinyI2C.init();

	auto vcc = GetVccVoltageMv();
	vcc += 1;
	
	auto batt = GetBatteryVoltageMv();
	batt += 1;
	
	auto analog1 = GetAnalog1VoltageMv();
	analog1 += 1;
	
	BH1750 lightMeter;
	lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,0x23,
	&TinyI2C);
	
	_delay_ms(150);

	uint16_t lux = lightMeter.readLightLevel();
	lux += 1;

}