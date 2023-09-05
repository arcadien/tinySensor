#pragma once

#include <stdint.h>

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
