#ifndef __HAL_LOADED__
#define __HAL_LOADED__ 1

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
#define OREGON_GO_LOW GetHal()->RadioGoLow();
#define OREGON_GO_HIGH GetHal()->RadioGoHigh();
#define OREGON_DELAY_US(x) _delay_us(x);
#include <ATtiny84aHal.h>
#else

#define OREGON_GO_LOW GetHal()->RadioGoLow();
#define OREGON_GO_HIGH GetHal()->RadioGoHigh();
#define OREGON_DELAY_US(x) (void *)0;
#include <NativeHal.h>
#endif

static Hal* GetHal()
{
    static Hal *hal;
    if(0 == hal)
#if defined(AVR)
        hal = new ATtiny84aHal();
#else
        hal = new NativeHal();
#endif
    return hal;
}

#endif