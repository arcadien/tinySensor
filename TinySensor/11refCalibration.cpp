
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>


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
* Reads internal 1.1v tension reference using VCC
*
*/
uint16_t GetVccVoltageMv(void) {
	// analog ref = VCC, input channel = VREF
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	ADMUX = 0b00100001;
	_delay_ms(1);

	static const uint16_t REF11 = 1101;
	uint16_t vccAdcRead = adcRead(4, 12);
	uint16_t vccMv = (uint16_t)((REF11 * 1023.0) / vccAdcRead);
	return vccMv;
}

/*!
*
* Reads voltage on ADC1 pin, relative to Vcc
*
*/
uint16_t GetBatteryVoltageMv(void) {

	uint16_t vccVoltageMv = GetVccVoltageMv();

	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	
	// analog ref = VCC, input channel = ADC1 (PA1)
	ADMUX = 0b00000001;
	_delay_ms(1);

	uint16_t batteryAdcRead = adcRead(4, 12);
	float mvPerAdcStep = (vccVoltageMv / 1024.0);
	uint16_t batteryVoltageMv = (uint16_t)(batteryAdcRead * mvPerAdcStep);
	return batteryVoltageMv;
}

int main(){
	auto vcc = GetVccVoltageMv();
	vcc += 1;
	
	auto batt = GetBatteryVoltageMv();
	batt += 1;
}