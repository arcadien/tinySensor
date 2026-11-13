#include "LacrosseDecoder.h"

static const uint16_t ShortUs = 400;
static const uint16_t LongUs = 800;
static const uint8_t MinSyncZeros = 6;
static const uint8_t SignBit = 0x08;

// Nominal duration +/- 30 %, bounds included
static bool Within(uint16_t durationUs, uint16_t nominalUs) {
  uint16_t tolerance = (uint32_t)nominalUs * 3 / 10;
  return durationUs >= nominalUs - tolerance &&
         durationUs <= nominalUs + tolerance;
}

// Data nibbles after the address nibble; 0 for an unknown type
static uint8_t DataNibbles(uint8_t type) {
  switch (type) {
    case 0:
      return 3;
    case 1:
      return 6;
    case 4:
      return 10;
    case 5:
      return 7;
    default:
      return 0;
  }
}

// A bit is a high pulse (1: short, 0: long) then a low of the other length
bool LacrosseDecoder::Feed(bool high, uint16_t durationUs) {
  bool one = Within(durationUs, ShortUs);
  bool validHigh = high && (one || Within(durationUs, LongUs));
  bool validLow = !high && expectLow && Within(durationUs, lowNominalUs);
  // A wrong-level high still counts: it may start a sync
  if (!validLow && (!validHigh || expectLow))
    Reset();
  expectLow = validHigh;
  if (!validHigh)
    return false;
  lowNominalUs = one ? LongUs : ShortUs;
  return ShiftIn(one);
}

void LacrosseDecoder::Reset() {
  inFrame = false;
  syncZeros = 0;
}

bool LacrosseDecoder::ShiftIn(bool one) {
  if (!inFrame) {
    if (!one) {
      if (syncZeros < MinSyncZeros)
        syncZeros++;
      return false;
    }
    inFrame = syncZeros == MinSyncZeros;
    syncZeros = 0;
    bitIndex = 0;
    nibble = 0;
    nibbleCount = 0;
    frameNibbles = MaxNibbles;  // until the type nibble is known
    return false;
  }
  if (bitIndex < 4) {
    nibble |= (one ? 1 : 0) << bitIndex++;
    return false;
  }
  if (!one) {
    Reset();
    return ShiftIn(one);  // this zero may be a sync zero
  }
  nibbles[nibbleCount++] = nibble;
  bitIndex = 0;
  nibble = 0;
  if (nibbleCount == 1) {
    uint8_t dataNibbles = DataNibbles(nibbles[0]);
    frameNibbles = 2 + dataNibbles + 2;
    if (dataNibbles == 0)
      Reset();
    return false;
  }
  if (nibbleCount < frameNibbles)
    return false;
  bool valid = Checked();
  inFrame = false;
  if (valid)
    Publish();
  return valid;
}

// XOR then sum (initialised to 5, XOR nibble included) of FUNC-LACROSSE-008
bool LacrosseDecoder::Checked() const {
  uint8_t checkXor = 0, checkSum = 5;
  for (uint8_t i = 0; i < frameNibbles - 2; i++) {
    checkXor ^= nibbles[i];
    checkSum += nibbles[i];
  }
  checkSum += checkXor;
  return nibbles[frameNibbles - 2] == checkXor &&
         nibbles[frameNibbles - 1] == (checkSum & 0x0F);
}

// Data nibbles d(first+2), d(first+1), d(first) as hundreds, tens and units;
// a nibble may exceed 9 (e.g. lux 1000 sends 10 hundreds)
uint16_t LacrosseDecoder::Digits(uint8_t first) const {
  const uint8_t *d = nibbles + 1;  // d[1] is the first data nibble
  return d[first + 2] * 100 + d[first + 1] * 10 + d[first];
}

void LacrosseDecoder::Publish() {
  type = nibbles[0];
  address = nibbles[1] & ~SignBit;
  bool negative = nibbles[1] & SignBit;
  temperatureTenths = 0;
  humidityTenths = 0;
  pressureTenths = 0;
  lux = 0;
  if (type == 5) {
    lux = Digits(1);
    return;
  }
  temperatureTenths = negative ? -Digits(1) : Digits(1);
  if (type == 1 || type == 4)
    humidityTenths = Digits(4);
  if (type == 4)
    pressureTenths = Digits(7) * 10 + nibbles[1 + 10] + 2000;
}
