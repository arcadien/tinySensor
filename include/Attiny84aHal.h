#ifndef __ATTINY84AHAL_H__
#define __ATTINY84AHAL_H__

#include <stdint.h>

class ATTiny84aHal
{
public:

	static void LedOn();
	
    // Radio related
    static void RadioGoLow();
    static void RadioGoHigh();
};

#endif