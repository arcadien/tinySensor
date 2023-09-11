#include <conversionTools.h>

namespace ConversionTools {

uint16_t dec16ToHex(uint16_t input) {
  uint16_t result = 0;
  static const uint8_t FACTOR_COUNT = 3;
  static const uint16_t factors[FACTOR_COUNT] = {10, 100, 1000};

  for (uint8_t i = FACTOR_COUNT; i > 0; i--) {
    uint16_t factor = factors[i - 1];
    uint8_t howManyPow10 = (input / factor);
    if (howManyPow10 > 0) {
      result += (howManyPow10 & 0x0f) << (i * 4);
      input -= (howManyPow10 * factor);
    }
  }
  if (input > 0) {
    result += (input & 0x0f);
  }
  return result;
}

uint32_t dec32ToHex(uint32_t input) {
  uint32_t result = 0;
  static const uint8_t FACTOR_COUNT = 7;
  static const uint32_t factors[FACTOR_COUNT] = {
      10, 100, 1000, 10000, 100000, 1000000, 10000000};

  for (uint8_t i = FACTOR_COUNT; i > 0; i--) {
    uint32_t factor = factors[i - 1];
    uint8_t howManyPow10 = (input / factor);
    if (howManyPow10 > 0) {
      result += (howManyPow10 & 0x0f) << (i * 4);
      input -= (howManyPow10 * factor);
    }
  }
  if (input > 0) {
    result += (input & 0x0f);
  }
  return result;
}
} // namespace ConversionTools