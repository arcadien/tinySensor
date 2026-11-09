#pragma once

#include <stdint.h>

// ATtiny84A receiver hardware (PLAT-DECODE-001): radio pulses captured on
// ICP1 (PA7) with Timer1 at 1 us, seconds counted from Timer1 overflows;
// SSD1306 128x32 OLED at I2C 0x3C, one 8x16 glyph per DrawChar and no
// framebuffer (PLAT-DECODE-002).
// Capture state lives in Attiny84aReceiverHal.cpp, shared with the ISRs.
class Attiny84aReceiverHal {
 public:
  void Init();

  bool PopPulse(bool &high, uint16_t &durationUs);
  uint32_t NowSeconds() const;
  void DrawChar(uint8_t row, uint8_t col, char c);
};
