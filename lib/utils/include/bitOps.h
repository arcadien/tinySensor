#pragma once

// Single definition of the bit helpers shared by the radio encoders
// (Oregon, Lacrosse, X10). Macros keep the generated AVR code identical
// to the previous per-file copies.

#define BITREAD(value, bit) (((value) >> (bit)) & 0x01)

#define BITWRITE(value, bit, bitvalue) \
  do {                                 \
    if (bitvalue)                      \
      (value) |= (1 << (bit));         \
    else                               \
      (value) &= ~(1 << (bit));        \
  } while (0)
