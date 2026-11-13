#pragma once

// Receiver hardware chosen at build time, no virtual call (TECH-DECODE-002).
// Interface: bool PopPulse(bool &high, uint16_t &durationUs),
// uint32_t NowSeconds() const,
// void DrawChar(uint8_t page, uint8_t col, char c),
// void DrawBigChar(uint8_t col, char c), static const bool HasLineOutput,
// void WriteLine(const char *line)
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)
#include "AtmegaReceiverHal.h"
typedef AtmegaReceiverHal ReceiverHal;
#elif !defined(AVR)
#include "TestReceiverHal.h"
typedef TestReceiverHal ReceiverHal;
#else
#error "no ReceiverHal for this MCU"
#endif
