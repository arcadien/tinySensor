#pragma once

// Receiver hardware chosen at build time, no virtual call (TECH-DECODE-002).
// Interface: bool PopPulse(bool &high, uint16_t &durationUs),
// uint32_t NowSeconds() const, void DrawChar(uint8_t row, uint8_t col, char c),
// static const bool HasLineOutput, void WriteLine(const char *line)
#if defined(__AVR_ATmega32U4__)
#include "ProMicroReceiverHal.h"
typedef ProMicroReceiverHal ReceiverHal;
#elif defined(__AVR_ATtiny84A__)
#include "Attiny84aReceiverHal.h"
typedef Attiny84aReceiverHal ReceiverHal;
#elif !defined(AVR)
#include "TestReceiverHal.h"
typedef TestReceiverHal ReceiverHal;
#else
#error "no ReceiverHal for this MCU"
#endif
