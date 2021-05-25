#pragma once
#include <cstdint>
#include <vector>

#if !defined(__HAL_LOADED__)
#error "Do not include NativeHal.h, but Hal.h"
#endif

#include <Oregon_v3.h>

class NativeHal : public Hal
{
public:
    static const int ORDERS_COUNT_FOR_A_BYTE = 6;
    const static unsigned char EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE];
    const static unsigned char EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE];
    // 6 unsigned chars per bit, 24 times "1"
    const static int PREAMBLE_BYTE_LENGTH = 24 * 6;
    const static unsigned char EXPECTED_PREAMBLE[PREAMBLE_BYTE_LENGTH];
    const static int POSTAMBLE_BYTE_LENGTH = 2 * 6;
    const static unsigned char EXPECTED_POSTAMBLE[POSTAMBLE_BYTE_LENGTH];

    mutable std::vector<unsigned char> Orders;

    void Delay(uint16_t delay)
    {
        if (delay == OregonV3::DELAY_US)
        {
            DelayPeriod();
        }
        else if (delay == OregonV3::HALF_DELAY_US)
        {
            DelayHalfPeriod();
        }
    }
    void DelayPeriod()
    {
        Orders.push_back('P');
    }
    void DelayHalfPeriod()
    {
        Orders.push_back('D');
    }

    unsigned char *GetOrders() { return &Orders.at(0); }

    void LedOn() const override {}
    inline void RadioGoHigh() const override { Orders.push_back('H'); }
    inline void RadioGoLow() const override { Orders.push_back('L'); }

};