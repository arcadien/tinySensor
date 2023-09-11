#include <AnalogFilter.h>

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
