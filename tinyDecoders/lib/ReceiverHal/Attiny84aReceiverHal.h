#pragma once

#include <stdint.h>

// ATtiny84A receiver hardware (PLAT-DECODE-001): radio pulses captured on
// ICP1 (PA7) with Timer1 at 1 us, seconds counted from Timer1 overflows.
// Capture state lives in Attiny84aReceiverHal.cpp, shared with the ISRs.
class Attiny84aReceiverHal {
 public:
  void Init();

  bool PopPulse(bool &high, uint16_t &durationUs);
  uint32_t NowSeconds() const;
};
