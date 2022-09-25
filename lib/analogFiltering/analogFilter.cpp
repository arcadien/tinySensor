#include <AnalogFilter.h>
#include <math.h>

void AnalogFilter::Push(uint16_t value)
{
    if (currentSampleIndex >= exclusion)
        accumulator += value;
    currentSampleIndex++;
}

uint16_t AnalogFilter::Get()
{ 
    return floor(accumulator / samples);
}
