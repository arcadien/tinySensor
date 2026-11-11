#pragma once

#include <stdint.h>

// Pro Micro 5 V / 16 MHz receiver hardware (ATmega32U4): radio pulses
// captured on ICP1 (PD4) with Timer1 at 0.5 us, seconds counted from its
// overflows (PLAT-DECODE-003); SSD1306 128x32 OLED at I2C 0x3C over Wire
// (PLAT-DECODE-004); RFLink lines on the USB CDC Serial (PLAT-DECODE-005).
// Capture state lives in ProMicroReceiverHal.cpp, shared with the ISRs.
class ProMicroReceiverHal {
 public:
  static const bool HasLineOutput = true;

  void Init();

  bool PopPulse(bool &high, uint16_t &durationUs);
  uint32_t NowSeconds() const;
  void DrawChar(uint8_t row, uint8_t col, char c);
  void WriteLine(const char *line);
};
