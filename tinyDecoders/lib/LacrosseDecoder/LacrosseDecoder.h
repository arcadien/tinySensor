#pragma once

#include <stdint.h>

// Rebuilds Lacrosse WS7000 frames from radio pulses (FUNC-DECODE-002) and
// exposes their measurements (FUNC-DECODE-003).
// Pure logic: fed with (level, duration) pulses, no hardware access.
class LacrosseDecoder {
 public:
  // Returns true when this pulse completes a valid frame
  bool Feed(bool high, uint16_t durationUs);
  uint8_t Type() const { return type; }
  uint8_t Address() const { return address; }
  int16_t TemperatureTenths() const { return temperatureTenths; }
  uint16_t HumidityTenths() const { return humidityTenths; }
  uint16_t PressureTenths() const { return pressureTenths; }
  uint16_t Lux() const { return lux; }

 private:
  // Type, address, up to 10 data nibbles, XOR and sum
  static const uint8_t MaxNibbles = 14;

  void Reset();
  bool ShiftIn(bool one);
  bool Checked() const;
  void Publish();
  uint16_t Digits(uint8_t first) const;

  bool expectLow = false;
  uint16_t lowNominalUs = 0;
  bool inFrame = false;
  uint8_t syncZeros = 0;
  uint8_t bitIndex = 0;
  uint8_t nibble = 0;
  uint8_t nibbleCount = 0;
  uint8_t frameNibbles = 0;
  uint8_t nibbles[MaxNibbles] = {};

  uint8_t type = 0;
  uint8_t address = 0;
  int16_t temperatureTenths = 0;
  uint16_t humidityTenths = 0;
  uint16_t pressureTenths = 0;
  uint16_t lux = 0;
};
