#include <AnalogFilter.h>

void AnalogFilter::Push(uint16_t value) {
  if (actualValuesCount >= exclusion && actualValuesCount < samples) {
    accumulator += value;
    actualValuesCount++;
  }
}

uint16_t AnalogFilter::Get() {
  uint16_t result = 0;
  if (actualValuesCount > 0) {
    result = (uint16_t)(accumulator / actualValuesCount);
  }
  return result;
}
