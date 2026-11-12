#pragma once

#include <stdint.h>

// Pro Mini (ATmega328P) or Pro Micro (ATmega32U4) 5 V / 16 MHz receiver
// hardware: radio pulses captured on ICP1 (PB0/D8 or PD4) with Timer1 at
// 0.5 us, seconds counted from its overflows (PLAT-DECODE-003); SSD1306
// 128x32 OLED at I2C 0x3C over Wire (PLAT-DECODE-004); RFLink lines on Serial,
// hardware UART or USB CDC (PLAT-DECODE-005).
// Capture state lives in AtmegaReceiverHal.cpp, shared with the ISRs.
class AtmegaReceiverHal {
 public:
  static const bool HasLineOutput = true;

  void Init();

  bool PopPulse(bool &high, uint16_t &durationUs);
  uint32_t NowSeconds() const;
  void DrawChar(uint8_t row, uint8_t col, char c);
  void WriteLine(const char *line);
};
