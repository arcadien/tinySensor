#pragma once

// Receiver hardware chosen at build time, no virtual call (TECH-DECODE-002).
// Interface: bool PopPulse(bool &high, uint16_t &durationUs),
// uint32_t NowSeconds() const, void DrawChar(uint8_t row, uint8_t col, char c)
#ifdef AVR
#include "Attiny84aReceiverHal.h"
typedef Attiny84aReceiverHal ReceiverHal;
#else
#include "TestReceiverHal.h"
typedef TestReceiverHal ReceiverHal;
#endif
