#pragma once

#include <stdint.h>

// Rebuilds X10 RFXmeter frames from radio pulses (FUNC-DECODE-001).
// Pure logic: fed with (level, duration) pulses, no hardware access.
class X10Decoder {
 public:
  // Returns true when this pulse completes a valid frame
  bool Feed(bool high, uint16_t durationUs);
  uint8_t Address() const { return address; }
  uint32_t Value() const { return value; }

 private:
  enum State : uint8_t { PreambleHigh, PreambleLow, BitHigh, BitLow };

  static const uint8_t FrameBytes = 6;

  void ShiftIn(bool one);
  bool Publish();

  State state = PreambleHigh;
  uint8_t bitCount = 0;
  uint8_t bytes[FrameBytes] = {};
  uint8_t address = 0;
  uint32_t value = 0;
};
