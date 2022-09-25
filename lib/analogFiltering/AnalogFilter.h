#pragma once

#include <stdint.h>

/**
 * @brief Accumulate and mean values
 *
 */
class AnalogFilter
{

private:
    uint8_t exclusion;
    uint8_t samples;
    uint8_t currentSampleIndex;
    uint16_t accumulator;

public:
    AnalogFilter(uint8_t exclusion, uint8_t samples) : exclusion(exclusion), samples(samples), currentSampleIndex(0), accumulator(0) {}

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
