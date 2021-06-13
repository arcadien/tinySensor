#pragma once
/*
 * Interface definition for lower level hardware function
 *
 */
class Hal
{
public:
    // Radio related
    virtual void LedOn() const = 0;

    // Radio related
    virtual void RadioGoLow() const = 0;
    virtual void RadioGoHigh() const = 0;
};


#if defined(AVR)
#include <util/delay.h>
#define OREGON_DELAY_US(x) _delay_us(x);
#else
#define OREGON_DELAY_US(x) (void *)0;
#endif