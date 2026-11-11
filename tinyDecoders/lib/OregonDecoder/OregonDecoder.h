#pragma once

#include <stdint.h>

// Rebuilds Oregon Scientific v2.1 frames (RFLink Plugin_048.c layout) from
// radio pulses (FUNC-DECODE-004) and exposes their measurements
// (FUNC-DECODE-005). Pure logic: fed with (level, duration) pulses, no hardware
// access.
class OregonDecoder {
 public:
  // Returns true when this pulse completes a valid frame
  bool Feed(bool high, uint16_t durationUs);
  uint16_t DeviceId() const { return deviceId; }
  uint8_t Channel() const { return channel; }
  uint8_t RollingCode() const { return rollingCode; }
  bool BatteryLow() const { return batteryLow; }
  int16_t TemperatureTenths() const { return temperatureTenths; }
  uint8_t HumidityPercent() const { return humidityPercent; }
  uint16_t PressureHpa() const { return pressureHpa; }

 private:
  // Pulses expected around the long middle pulse carrying each bit
  enum State : uint8_t {
    Idle,             // waiting for a high long pulse (a sync one)
    AfterBit,         // separator: one long or a first short, opposite level
    EqualBitNext,     // middle of a bit equal to the last one
    SecondShort,      // second short, same level as the last bit
    InvertedBitNext,  // middle of a bit inverted from the last one
  };

  static const uint8_t MaxFrameBytes = 11;

  bool Bit(bool one);
  bool ShiftIn(bool one);
  bool Restart(bool syncHigh);
  void Reset();
  void Store(bool one);
  uint16_t Id() const;
  uint8_t Nibble(uint8_t k) const;
  bool Checked() const;
  void Publish();

  State state = Idle;
  bool level = false;
  uint8_t syncOnes = 0;
  uint8_t bitCount = 0;   // 0 while waiting for the sync run to end
  uint8_t frameBits = 0;  // 0 until the device ID is known
  uint8_t bytes[MaxFrameBytes] = {};

  uint16_t deviceId = 0;
  uint8_t channel = 0;
  uint8_t rollingCode = 0;
  bool batteryLow = false;
  int16_t temperatureTenths = 0;
  uint8_t humidityPercent = 0;
  uint16_t pressureHpa = 0;
};
