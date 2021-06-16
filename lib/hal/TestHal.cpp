#include <TestHal.h>

TestHal_ TestHal;

const unsigned char TestHal_::EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE] =
    {'H', 'D', 'L', 'P', 'H', 'D'};

const unsigned char TestHal_::EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE] =
    {'L', 'D', 'H', 'P', 'L', 'D'};

const unsigned char
    TestHal_::EXPECTED_POSTAMBLE[TestHal_::POSTAMBLE_BYTE_LENGTH] = {
        // postamble : two nibbles at zero
        'H', 'D', 'L', 'P', 'H', 'D', // 0
        'H', 'D', 'L', 'P', 'H', 'D'  // 0
};

const unsigned char TestHal_::EXPECTED_PREAMBLE[TestHal_::PREAMBLE_BYTE_LENGTH] =
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