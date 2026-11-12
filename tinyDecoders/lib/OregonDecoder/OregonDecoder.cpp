#include "OregonDecoder.h"

// Windows from RFLink; long threshold at the 488/976 us midpoint
static const uint16_t MinShortUs = 150;
static const uint16_t MinLongUs = 732;
static const uint16_t MaxLongUs = 1199;
static const uint8_t MinSyncOnes = 12;
static const uint16_t Thn132nId = 0xEA4C;
static const uint16_t Thgr228nId = 0x1A2D;
static const uint16_t Thgr918Id = 0x1A3D;
static const uint16_t Bthr918Id = 0x5A5D;

bool OregonDecoder::Feed(bool high, uint16_t durationUs) {
  bool isLong = durationUs >= MinLongUs && durationUs <= MaxLongUs;
  bool isShort = durationUs >= MinShortUs && durationUs < MinLongUs;
  bool sameLevel = high == level;
  switch (state) {
    case AfterBit:
      if (!sameLevel && (isLong || isShort)) {
        state = isLong ? EqualBitNext : SecondShort;
        return false;
      }
      break;
    case EqualBitNext:
      if (sameLevel && isLong)
        return Bit(high);
      break;
    case SecondShort:
      if (sameLevel && isShort) {
        state = InvertedBitNext;
        return false;
      }
      break;
    case InvertedBitNext:
      if (!sameLevel && isLong)
        return Bit(high);
      break;
    case Idle:
      break;
  }
  return Restart(high && isLong);
}

// The middle pulse of a bit has the bit value as level
bool OregonDecoder::Bit(bool one) {
  level = one;
  state = AfterBit;
  return ShiftIn(one);
}

// Resets the decoder; a high long pulse is re-checked as a sync one
bool OregonDecoder::Restart(bool syncHigh) {
  Reset();
  return syncHigh && Bit(true);
}

void OregonDecoder::Reset() {
  state = Idle;
  syncOnes = 0;
  bitCount = 0;
}

bool OregonDecoder::ShiftIn(bool one) {
  if (bitCount == 0) {
    if (one) {
      if (syncOnes < MinSyncOnes)
        syncOnes++;
      return false;
    }
    if (syncOnes < MinSyncOnes)
      return Restart(false);
    // This zero is the first bit of b0, whose low nibble is the sync 0xA
    for (uint8_t &byte : bytes) byte = 0;
    frameBits = 0;
  }
  Store(one);
  if (bitCount == 16) {
    // Bits up to the last checksum nibble (checksum at nibble 13, 16 or 20)
    uint16_t id = Id();
    frameBits = id == Thn132nId                       ? 60
                : id == Thgr228nId || id == Thgr918Id ? 72
                : id == Bthr918Id                     ? 88
                                                      : 0;
    if (frameBits == 0)
      return Restart(one);
  }
  if (bitCount < frameBits || frameBits == 0)
    return false;
  if (!Checked())
    return Restart(one);
  Publish();
  Reset();
  return true;
}

void OregonDecoder::Store(bool one) {
  if (one)
    bytes[bitCount / 8] |= 1 << (bitCount % 8);
  bitCount++;
}

uint16_t OregonDecoder::Id() const {
  return (bytes[0] << 8) | bytes[1];
}

// Nibble 2i is the low nibble of byte i
uint8_t OregonDecoder::Nibble(uint8_t k) const {
  return k % 2 ? bytes[k / 2] >> 4 : bytes[k / 2] & 0x0F;
}

// Last two nibbles: (sum of the previous nibbles - 0xA) & 0xFF, low first
bool OregonDecoder::Checked() const {
  uint8_t c = frameBits / 4 - 2;
  uint8_t sum = 0;
  for (uint8_t k = 0; k < c; k++) sum += Nibble(k);
  sum -= 0xA;
  return Nibble(c) == (sum & 0x0F) && Nibble(c + 1) == (sum >> 4);
}

void OregonDecoder::Publish() {
  deviceId = Id();
  // Channels 1, 2 and 3 are sent as bits 0, 1 and 2
  uint8_t channelBits = bytes[2] >> 4;
  channel = channelBits == 4 ? 3 : channelBits <= 2 ? channelBits : 0;
  rollingCode = bytes[3];
  batteryLow = (bytes[4] & 0x0C) != 0;
  int16_t temperature =
      (bytes[5] >> 4) * 100 + (bytes[5] & 0x0F) * 10 + (bytes[4] >> 4);
  temperatureTenths = bytes[6] & 0x08 ? -temperature : temperature;
  humidityPercent = 0;
  pressureHpa = 0;
  if (deviceId != Thn132nId)
    humidityPercent = (bytes[7] & 0x0F) * 10 + (bytes[6] >> 4);
  if (deviceId == Bthr918Id)
    pressureHpa = bytes[8] + 795;
}
