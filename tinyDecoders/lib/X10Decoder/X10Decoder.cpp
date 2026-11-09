#include "X10Decoder.h"

static const uint16_t PreambleHighUs = 8960;
static const uint16_t PreambleLowUs = 4500;
static const uint16_t ShortUs = 560;
static const uint16_t LongUs = 1680;

// Nominal duration +/- 30 %, bounds included
static bool Within(uint16_t durationUs, uint16_t nominalUs) {
  uint16_t tolerance = (uint32_t)nominalUs * 3 / 10;
  return durationUs >= nominalUs - tolerance &&
         durationUs <= nominalUs + tolerance;
}

static uint8_t NibbleSum(uint8_t byte) {
  return (byte >> 4) + (byte & 0x0F);
}

bool X10Decoder::Feed(bool high, uint16_t durationUs) {
  switch (state) {
    case PreambleLow:
      if (!high && Within(durationUs, PreambleLowUs)) {
        bitCount = 0;
        state = BitHigh;
        return false;
      }
      break;
    case BitHigh:
      if (high && Within(durationUs, ShortUs)) {
        state = BitLow;
        return false;
      }
      break;
    case BitLow: {
      bool one = Within(durationUs, LongUs);
      if (!high && (one || Within(durationUs, ShortUs))) {
        ShiftIn(one);
        if (bitCount < FrameBytes * 8) {
          state = BitHigh;
          return false;
        }
        state = PreambleHigh;
        if (Publish())
          return true;
      }
      break;
    }
    case PreambleHigh:
      break;
  }
  // Reset: this pulse may be the start of the next frame
  state =
      high && Within(durationUs, PreambleHighUs) ? PreambleLow : PreambleHigh;
  return false;
}

void X10Decoder::ShiftIn(bool one) {
  uint8_t &byte = bytes[bitCount / 8];
  byte = (byte << 1) | (one ? 1 : 0);
  bitCount++;
}

// Checks the frame and, if valid, exposes its address and value
bool X10Decoder::Publish() {
  uint8_t packetType = bytes[5] >> 4;
  uint8_t sum = packetType;
  for (uint8_t i = 0; i < 5; i++) sum += NibbleSum(bytes[i]);
  bool valid = bytes[1] == ((~bytes[0] & 0xF0) | (bytes[0] & 0x0F)) &&
               packetType == 0 && (bytes[5] & 0x0F) == (~sum & 0x0F);
  if (valid) {
    address = bytes[0];
    value = ((uint32_t)bytes[4] << 16) | ((uint16_t)bytes[2] << 8) | bytes[3];
  }
  return valid;
}
