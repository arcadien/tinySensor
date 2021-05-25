#include <Hal.h>

#if defined(NATIVE)
#if !defined(__HAL_LOADED__)
#error "Do not include NativeHal.h, but Hal.h"
#endif

const unsigned char NativeHal::EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE] =
    {'H', 'D', 'L', 'P', 'H', 'D'};

const unsigned char NativeHal::EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE] =
    {'L', 'D', 'H', 'P', 'L', 'D'};

const unsigned char
    NativeHal::EXPECTED_POSTAMBLE[NativeHal::POSTAMBLE_BYTE_LENGTH] = {
        // postamble : two nibbles at zero
        'H', 'D', 'L', 'P', 'H', 'D', // 0
        'H', 'D', 'L', 'P', 'H', 'D'  // 0
};

const unsigned char NativeHal::EXPECTED_PREAMBLE[NativeHal::PREAMBLE_BYTE_LENGTH] =
    {
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
};
#endif