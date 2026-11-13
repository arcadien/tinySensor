/*
 * Integration tests for FUNC-DECODE-004: OregonDecoder rebuilds Oregon v2.1
 * frames (device ID, channel, rolling code, battery) from level/duration
 * pulses, with the RFLink OregonDecoderV2 windows and layout (RFLink
 * Plugins/Plugin_048.c, couin3).
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/OregonDecoder/OregonDecoder.h (depends only on
 * <stdint.h>, default constructor):
 *   class OregonDecoder {
 *    public:
 *     bool Feed(bool high, uint16_t durationUs);
 *     uint16_t DeviceId() const;  // (b0 << 8) | b1: 0xEA4C, 0x1A2D, ...
 *     uint8_t Channel() const;
 *     uint8_t RollingCode() const;  // b3
 *     bool BatteryLow() const;      // (b4 & 0x0C) != 0
 *     int16_t TemperatureTenths() const;  // FUNC-DECODE-005
 *     uint8_t HumidityPercent() const;    // FUNC-DECODE-005
 *     uint16_t PressureHpa() const;       // FUNC-DECODE-005, b8 + 795
 *   };
 *
 * Also FUNC-DECODE-005: decoded measurements in tenths of °C, % and hPa.
 */

#include <stdint.h>
#include <unity.h>

#include <Loopback.h>
#include <OregonDecoder.h>
#include <Oregon_v3.h>
#include <TestHal.h>
#include <vector>

enum Field : uint8_t { TEMP = 1, HUM = 2, PRES = 4 };

struct Reading {
  uint8_t fields;
  float temperature;
  uint8_t humidity;
  uint16_t pressure;
};

struct Identity {
  uint8_t channel, rollingCode;
  bool batteryLow;
};

static std::vector<Pulse> sentPulses(const Identity &who, const Reading &r,
                                     int sends) {
  OregonV3 encoder(&TestHal);
  encoder.SetChannel(who.channel);
  encoder.SetRollingCode(who.rollingCode);
  encoder.SetBatteryLow(who.batteryLow);
  if (r.fields & TEMP)
    encoder.SetTemperature(r.temperature);
  if (r.fields & HUM)
    encoder.SetHumidity(r.humidity);
  if (r.fields & PRES)
    encoder.SetPressure(r.pressure);
  TestHal.ClearOrders();
  for (int i = 0; i < sends; i++) {
    encoder.Send();
  }
  return OrdersToPulses(TestHal.Orders);
}

static int countFrames(OregonDecoder &decoder,
                       const std::vector<Pulse> &pulses) {
  int frames = 0;
  for (const Pulse &p : pulses) {
    frames += decoder.Feed(p.high, p.durationUs) ? 1 : 0;
  }
  return frames;
}

// Nibble 2i is the low nibble of byte i
static uint8_t nibble(const std::vector<uint8_t> &bytes, int k) {
  return k % 2 ? bytes[k / 2] >> 4 : bytes[k / 2] & 0x0F;
}

static void setNibble(std::vector<uint8_t> &bytes, int k, uint8_t value) {
  uint8_t shift = k % 2 ? 4 : 0;
  bytes[k / 2] = (bytes[k / 2] & ~(0x0F << shift)) | ((value & 0x0F) << shift);
}

// Nibbles c and c+1: (sum of nibbles 0..c-1 - 0xA) & 0xFF
static std::vector<uint8_t> withChecksum(std::vector<uint8_t> bytes, int c) {
  unsigned sum = 0;
  for (int k = 0; k < c; k++) {
    sum += nibble(bytes, k);
  }
  sum = (sum - 0xA) & 0xFF;
  setNibble(bytes, c, sum & 0x0F);
  setNibble(bytes, c + 1, sum >> 4);
  return bytes;
}

static std::vector<uint8_t> flipped(std::vector<uint8_t> bytes, size_t index,
                                    uint8_t mask) {
  bytes[index] ^= mask;
  return bytes;
}

static std::vector<uint8_t> withByte(std::vector<uint8_t> bytes, size_t index,
                                     uint8_t value) {
  bytes[index] = value;
  return bytes;
}

// Bits up to the last checksum nibble: 15, 18 or 22 nibbles
static size_t frameBits(const std::vector<uint8_t> &bytes) {
  return bytes[0] == 0xEA ? 60 : bytes[0] == 0x1A ? 72 : 88;
}

// Sync ones then the frame bits LSB-first, ending on the long pulse of the
// last frame bit (the encoder's trailing half bit is dropped).
static std::vector<Pulse> framePulses(const std::vector<uint8_t> &bytes,
                                      int syncOnes = 24) {
  OregonV3 encoder(&TestHal);
  TestHal.ClearOrders();
  for (int i = 0; i < syncOnes; i++) {
    encoder.SendOne();
  }
  for (size_t i = 0; i < frameBits(bytes); i++) {
    if ((bytes[i / 8] >> (i % 8)) & 1) {
      encoder.SendOne();
    } else {
      encoder.SendZero();
    }
  }
  std::vector<Pulse> pulses = OrdersToPulses(TestHal.Orders);
  pulses.pop_back();
  return pulses;
}

// Encoder output scaled to other short (512 µs) and long (1024 µs) durations
static std::vector<Pulse> retimed(std::vector<Pulse> pulses, uint16_t shortUs,
                                  uint16_t longUs) {
  for (Pulse &p : pulses) {
    p.durationUs = p.durationUs == 512 ? shortUs : longUs;
  }
  return pulses;
}

// FUNC-OREGON-014 reference payloads
// THN132N, channel 1, rolling code 0x3C, battery low, -10.2 °C
static const std::vector<uint8_t> thn132nBytes = {
    0xEA, 0x4C, 0x10, 0x3C, 0x24, 0x10, 0xD8, 0x03, 0x00, 0x00, 0x00};
// THGR228N, channel 3, rolling code 0xC4, 21.5 °C, 67 %
static const std::vector<uint8_t> thgr228nBytes = {
    0x1A, 0x2D, 0x40, 0xC4, 0x50, 0x21, 0x70, 0x06, 0x39, 0x00, 0x00};
// BTHR918, channel 3, rolling code 0x7E, battery low, 21.5 °C, 60 %, 1013 hPa
static const std::vector<uint8_t> bthr918Bytes = {
    0x5A, 0x5D, 0x40, 0x7E, 0x54, 0x21, 0x00, 0x06, 0xDA, 0x60, 0x5F};

// RFLink Plugins/Plugin_048.c (couin3) captures, lines 644-645 and 685-686
static const std::vector<uint8_t> captureThn1 = {0xEA, 0x4C, 0x20, 0x72,
                                                 0x5C, 0x21, 0xD0, 0x83};
static const std::vector<uint8_t> captureThn2 = {0xEA, 0x4C, 0x10, 0x13,
                                                 0x60, 0x19, 0x30, 0x23};
static const std::vector<uint8_t> captureThgr1 = {0x1A, 0x2D, 0x40, 0xC4, 0x51,
                                                  0x21, 0x70, 0x46, 0x3E};
static const std::vector<uint8_t> captureThgr2 = {0x1A, 0x2D, 0x10, 0x72, 0x51,
                                                  0x20, 0x80, 0xE7, 0x3F};
// THGR228N capture re-labelled with the other temperature-humidity ID
static const std::vector<uint8_t> capture1a3d =
    withChecksum(withByte(captureThgr1, 1, 0x3D), 16);

// Built on first use: TestHal may not be constructed during static init
static const std::vector<Pulse> &goodFrame() {
  static const std::vector<Pulse> pulses = framePulses(thn132nBytes);
  return pulses;
}

struct FrameCase {
  const char *id;
  Identity identity;
  Reading reading;
  uint16_t deviceId;
};

static const FrameCase frameCases[] = {
    {"thn132n", {1, 0x3C, true}, {TEMP, -10.2f, 0, 0}, 0xEA4C},
    {"thgr228n", {3, 0xC4, false}, {TEMP | HUM, 21.5f, 67, 0}, 0x1A2D},
    {"bthr918", {3, 0x7E, true}, {TEMP | HUM | PRES, 21.5f, 60, 1013}, 0x5A5D},
};

// @req FUNC-DECODE-004
void OregonDecoder_decodes_each_repeat_of_oregon_v21() {
  for (const FrameCase &c : frameCases) {
    // Given
    std::vector<Pulse> pulses = sentPulses(c.identity, c.reading, 2);
    OregonDecoder decoder;
    int frames = 0;

    // When
    for (const Pulse &p : pulses) {
      if (decoder.Feed(p.high, p.durationUs)) {
        frames++;
        // Then (checked on each true)
        TEST_ASSERT_EQUAL_HEX16_MESSAGE(c.deviceId, decoder.DeviceId(), c.id);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.identity.channel, decoder.Channel(),
                                        c.id);
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(c.identity.rollingCode,
                                       decoder.RollingCode(), c.id);
        TEST_ASSERT_EQUAL_MESSAGE(c.identity.batteryLow, decoder.BatteryLow(),
                                  c.id);
      }
    }

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, frames, c.id);
  }
}

struct CaptureCase {
  const char *id;
  std::vector<uint8_t> bytes;
  uint16_t deviceId;
  Identity identity;
};

static const CaptureCase captureCases[] = {
    {"rflink-thn132n-ch2-low", captureThn1, 0xEA4C, {2, 0x72, true}},
    {"rflink-thn132n-ch1-ok", captureThn2, 0xEA4C, {1, 0x13, false}},
    {"rflink-thgr228n-ch3-ok", captureThgr1, 0x1A2D, {3, 0xC4, false}},
    {"rflink-thgr228n-ch1-ok", captureThgr2, 0x1A2D, {1, 0x72, false}},
    {"1a3d-recomputed-checksum", capture1a3d, 0x1A3D, {3, 0xC4, false}},
};

// @req FUNC-DECODE-004
void OregonDecoder_decodes_rflink_captures() {
  for (const CaptureCase &c : captureCases) {
    // Given
    std::vector<Pulse> pulses = framePulses(c.bytes);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(c.deviceId, decoder.DeviceId(), c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.identity.channel, decoder.Channel(),
                                    c.id);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(c.identity.rollingCode,
                                   decoder.RollingCode(), c.id);
    TEST_ASSERT_EQUAL_MESSAGE(c.identity.batteryLow, decoder.BatteryLow(),
                              c.id);
  }
}

struct ChannelCase {
  const char *id;
  uint8_t channelNibble, channel;
};

static const ChannelCase channelCases[] = {
    {"nibble-1", 1, 1}, {"nibble-2", 2, 2}, {"nibble-4", 4, 3},
    {"nibble-0", 0, 0}, {"nibble-3", 3, 0}, {"nibble-8", 8, 0},
};

// @req FUNC-DECODE-004
void OregonDecoder_maps_channel_nibble_to_channel() {
  for (const ChannelCase &c : channelCases) {
    // Given
    std::vector<uint8_t> bytes = thn132nBytes;
    setNibble(bytes, 5, c.channelNibble);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, framePulses(withChecksum(bytes, 13)));

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.channel, decoder.Channel(), c.id);
  }
}

struct BatteryCase {
  const char *id;
  uint8_t flagsNibble;
  bool batteryLow;
};

static const BatteryCase batteryCases[] = {
    {"flags-0", 0x0, false},        {"flags-bit-2", 0x4, true},
    {"flags-bit-3", 0x8, true},     {"flags-bits-2-3", 0xC, true},
    {"flags-bits-0-1", 0x3, false},
};

// @req FUNC-DECODE-004
void OregonDecoder_reads_battery_low_from_flags_bits_2_and_3() {
  for (const BatteryCase &c : batteryCases) {
    // Given
    std::vector<uint8_t> bytes = thn132nBytes;
    setNibble(bytes, 8, c.flagsNibble);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, framePulses(withChecksum(bytes, 13)));

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_MESSAGE(c.batteryLow, decoder.BatteryLow(), c.id);
  }
}

struct WindowCase {
  const char *id;
  uint16_t shortUs, longUs;
  int expectedFrames;
};

// Short 150-731 µs, long 732-1199 µs
static const WindowCase windowCases[] = {
    {"short-min-150", 150, 1024, 1},   {"short-below-149", 149, 1024, 0},
    {"short-max-731", 731, 1024, 1},   {"short-732-is-long", 732, 1024, 0},
    {"long-min-732", 512, 732, 1},     {"long-731-is-short", 512, 731, 0},
    {"long-max-1199", 512, 1199, 1},   {"long-above-1200", 512, 1200, 0},
    {"both-min-150-732", 150, 732, 1}, {"both-max-731-1199", 731, 1199, 1},
};

// @req FUNC-DECODE-004
void OregonDecoder_accepts_pulses_within_rflink_windows() {
  for (const WindowCase &c : windowCases) {
    // Given
    std::vector<Pulse> pulses = retimed(goodFrame(), c.shortUs, c.longUs);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(c.expectedFrames, frames, c.id);
  }
}

// Encoder output with level-dependent durations, as a weak sensor on the
// RXB6 gives (highs short, lows long)
static std::vector<Pulse> retimedByLevel(std::vector<Pulse> pulses,
                                         uint16_t shortHighUs,
                                         uint16_t shortLowUs,
                                         uint16_t longHighUs,
                                         uint16_t longLowUs) {
  for (Pulse &p : pulses) {
    bool isShort = p.durationUs == 512;
    p.durationUs = p.high ? (isShort ? shortHighUs : longHighUs)
                          : (isShort ? shortLowUs : longLowUs);
  }
  return pulses;
}

// @req FUNC-DECODE-004
void OregonDecoder_decodes_asymmetric_weak_sensor_frame() {
  // Given
  std::vector<Pulse> pulses =
      retimedByLevel(framePulses(thgr228nBytes), 400, 620, 880, 1060);
  OregonDecoder decoder;

  // When
  int frames = countFrames(decoder, pulses);

  // Then
  TEST_ASSERT_EQUAL_INT(1, frames);
  TEST_ASSERT_EQUAL_HEX16(0x1A2D, decoder.DeviceId());
  TEST_ASSERT_EQUAL_UINT8(3, decoder.Channel());
  TEST_ASSERT_EQUAL_HEX8(0xC4, decoder.RollingCode());
}

struct CorruptionCase {
  const char *id;
  std::vector<uint8_t> bytes;
};

static const CorruptionCase corruptionCases[] = {
    {"thn132n-data-changed-checksum-stale", flipped(thn132nBytes, 5, 0x01)},
    {"thn132n-checksum-low-nibble", flipped(thn132nBytes, 6, 0x10)},
    {"thn132n-checksum-high-nibble", flipped(thn132nBytes, 7, 0x01)},
    {"thgr228n-checksum", flipped(thgr228nBytes, 8, 0x80)},
    {"thgr228n-humidity-changed", flipped(thgr228nBytes, 7, 0x01)},
    {"bthr918-checksum", flipped(bthr918Bytes, 10, 0x01)},
    {"bthr918-pressure-changed", flipped(bthr918Bytes, 8, 0x10)},
    {"rflink-capture-one-data-bit-flipped", flipped(captureThn1, 5, 0x01)},
    {"rflink-thgr-capture-one-data-bit-flipped",
     flipped(captureThgr2, 3, 0x40)},
    {"unknown-id-ea4d", withChecksum(flipped(thn132nBytes, 1, 0x01), 13)},
    {"unknown-id-fa4c", withChecksum(flipped(thn132nBytes, 0, 0x10), 13)},
    {"unknown-id-1a2c", withChecksum(flipped(thgr228nBytes, 1, 0x01), 16)},
    {"unknown-id-5a5c", withChecksum(flipped(bthr918Bytes, 1, 0x01), 20)},
    {"old-layout-id-ec40",
     {0xEC, 0x40, 0x13, 0xC4, 0x20, 0x18, 0x30, 0x03, 0x00, 0x00, 0x00}},
};

// @req FUNC-DECODE-004
void OregonDecoder_rejects_corrupted_frames() {
  for (const CorruptionCase &c : corruptionCases) {
    // Given
    std::vector<Pulse> pulses = framePulses(c.bytes);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, frames, c.id);
  }
}

struct SyncCase {
  const char *id;
  std::vector<uint8_t> bytes;
  int syncOnes;
  int expectedFrames;
};

// Every ID starts with the sync nibble 0xA, whose first bit is a zero
static const SyncCase syncCases[] = {
    {"no-one", thn132nBytes, 0, 0},
    {"11-ones", thn132nBytes, 11, 0},
    {"12-ones", thn132nBytes, 12, 1},
    {"13-ones", thn132nBytes, 13, 1},
    {"24-ones", thn132nBytes, 24, 1},
    {"thgr228n-11-ones", thgr228nBytes, 11, 0},
    {"thgr228n-12-ones", thgr228nBytes, 12, 1},
    {"bthr918-11-ones", bthr918Bytes, 11, 0},
    {"bthr918-12-ones", bthr918Bytes, 12, 1},
};

// @req FUNC-DECODE-004
void OregonDecoder_requires_at_least_12_sync_ones() {
  for (const SyncCase &c : syncCases) {
    // Given
    std::vector<Pulse> pulses = framePulses(c.bytes, c.syncOnes);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(c.expectedFrames, frames, c.id);
  }
}

// First pulses of the good frame, then a 20 ms gap of the opposite level
static std::vector<Pulse> truncated(size_t pulseCount) {
  std::vector<Pulse> pulses(goodFrame().begin(),
                            goodFrame().begin() + pulseCount);
  pulses.push_back({!pulses.back().high, 20000});
  return pulses;
}

struct PrefixCase {
  const char *id;
  std::vector<Pulse> prefix;
};

// @req FUNC-DECODE-004
void OregonDecoder_recovers_from_noise_and_partial_frames() {
  // The good frame follows each prefix with no gap; pulse 0 is a leading
  // low, 1..47 the 24 sync ones, 48..50 the first zero bit.
  const PrefixCase prefixCases[] = {
      {"noise", {{true, 100}, {false, 300}, {true, 20000}, {false, 50}}},
      {"low-while-idle", {{false, 1024}}},
      {"out-of-window-low", {{true, 1024}, {false, 2000}}},
      {"wrong-level-mid-sync", {{true, 1024}, {false, 1024}, {false, 1024}}},
      {"mid-sync", truncated(10)},
      {"after-sync-ones", truncated(48)},
      {"after-first-zero", truncated(51)},
      {"mid-device-id", truncated(60)},
      {"mid-data", truncated(100)},
      {"before-last-bit", truncated(goodFrame().size() - 1)},
      {"bad-checksum-frame", framePulses(flipped(thn132nBytes, 7, 0x01))},
      {"unknown-id-frame",
       framePulses(withChecksum(flipped(thn132nBytes, 1, 0x01), 13))},
  };
  for (const PrefixCase &c : prefixCases) {
    // Given
    std::vector<Pulse> pulses = c.prefix;
    pulses.insert(pulses.end(), goodFrame().begin(), goodFrame().end());
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xEA4C, decoder.DeviceId(), c.id);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x3C, decoder.RollingCode(), c.id);
  }
}

// @req FUNC-DECODE-004
void OregonDecoder_completes_frame_on_last_checksum_bit() {
  // Given
  const std::vector<Pulse> &pulses = goodFrame();
  OregonDecoder decoder;
  for (size_t i = 0; i + 1 < pulses.size(); i++) {
    TEST_ASSERT_FALSE(decoder.Feed(pulses[i].high, pulses[i].durationUs));
  }

  // When
  bool completed = decoder.Feed(pulses.back().high, pulses.back().durationUs);

  // Then
  TEST_ASSERT_TRUE(completed);
  TEST_ASSERT_EQUAL_HEX16(0xEA4C, decoder.DeviceId());
  TEST_ASSERT_EQUAL_UINT8(1, decoder.Channel());
  TEST_ASSERT_EQUAL_HEX8(0x3C, decoder.RollingCode());
  TEST_ASSERT_TRUE(decoder.BatteryLow());
}

struct ValueCase {
  const char *id;
  Reading reading;
  int16_t temperature;
  uint8_t humidity;
  uint16_t pressure;
};

// Fields the device ID does not carry are expected to read 0
static const ValueCase valueCases[] = {
    {"spec-thn132n", {TEMP, -10.2f, 0, 0}, -102, 0, 0},
    {"spec-thgr228n", {TEMP | HUM, 21.5f, 67, 0}, 215, 67, 0},
    {"spec-bthr918", {TEMP | HUM | PRES, 21.5f, 60, 1013}, 215, 60, 1013},
    {"zero-temperature", {TEMP, 0, 0, 0}, 0, 0, 0},
    {"negative-below-one-degree", {TEMP, -0.5f, 0, 0}, -5, 0, 0},
    {"negative-with-humidity", {TEMP | HUM, -25.3f, 87, 0}, -253, 87, 0},
    {"positive-high", {TEMP, 59.9f, 0, 0}, 599, 0, 0},
    {"humidity-min", {TEMP | HUM, 20, 0, 0}, 200, 0, 0},
    {"humidity-max", {TEMP | HUM, 20, 99, 0}, 200, 99, 0},
    {"pressure-offset", {TEMP | HUM | PRES, 20, 50, 795}, 200, 50, 795},
    {"pressure-1050", {TEMP | HUM | PRES, 20, 50, 1050}, 200, 50, 1050},
};

// @req FUNC-DECODE-005
void OregonDecoder_exposes_measurements_of_oregon_v21() {
  for (const ValueCase &c : valueCases) {
    // Given
    std::vector<Pulse> pulses = sentPulses({1, 0x3C, false}, c.reading, 1);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_INT16_MESSAGE(c.temperature, decoder.TemperatureTenths(),
                                    c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.humidity, decoder.HumidityPercent(),
                                    c.id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(c.pressure, decoder.PressureHpa(), c.id);
  }
}

struct ByteValueCase {
  const char *id;
  std::vector<uint8_t> bytes;
  int16_t temperature;
  uint8_t humidity;
  uint16_t pressure;
};

static const ByteValueCase byteValueCases[] = {
    {"rflink-thn132n-1", captureThn1, 215, 0, 0},
    {"rflink-thn132n-2", captureThn2, 196, 0, 0},
    {"rflink-thgr228n-1", captureThgr1, 215, 67, 0},
    {"rflink-thgr228n-2", captureThgr2, 205, 78, 0},
    {"1a3d-carries-humidity", capture1a3d, 215, 67, 0},
    // Old-firmware tinySensor on the bench: byte 8 = 0xB4 shown by RFLink as
    // 975 hPa (FUNC-OREGON-008)
    {"bench-old-firmware-b4-is-975",
     withChecksum(withByte(bthr918Bytes, 8, 0xB4), 20), 215, 60, 975},
};

// @req FUNC-DECODE-005
void OregonDecoder_exposes_measurements_of_rflink_captures() {
  for (const ByteValueCase &c : byteValueCases) {
    // Given
    std::vector<Pulse> pulses = framePulses(c.bytes);
    OregonDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_INT16_MESSAGE(c.temperature, decoder.TemperatureTenths(),
                                    c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.humidity, decoder.HumidityPercent(),
                                    c.id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(c.pressure, decoder.PressureHpa(), c.id);
  }
}

// @req FUNC-DECODE-005
void OregonDecoder_clears_fields_absent_from_the_next_device() {
  // Given
  OregonDecoder decoder;
  countFrames(decoder, sentPulses({3, 0x7E, false},
                                  {TEMP | HUM | PRES, 21.5f, 60, 1013}, 1));

  // When
  int frames = countFrames(
      decoder, sentPulses({1, 0x3C, false}, {TEMP, -10.2f, 0, 0}, 1));

  // Then
  TEST_ASSERT_EQUAL_INT(1, frames);
  TEST_ASSERT_EQUAL_HEX16(0xEA4C, decoder.DeviceId());
  TEST_ASSERT_EQUAL_INT16(-102, decoder.TemperatureTenths());
  TEST_ASSERT_EQUAL_UINT8(0, decoder.HumidityPercent());
  TEST_ASSERT_EQUAL_UINT16(0, decoder.PressureHpa());
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(OregonDecoder_decodes_each_repeat_of_oregon_v21);
  RUN_TEST(OregonDecoder_decodes_rflink_captures);
  RUN_TEST(OregonDecoder_maps_channel_nibble_to_channel);
  RUN_TEST(OregonDecoder_reads_battery_low_from_flags_bits_2_and_3);
  RUN_TEST(OregonDecoder_accepts_pulses_within_rflink_windows);
  RUN_TEST(OregonDecoder_decodes_asymmetric_weak_sensor_frame);
  RUN_TEST(OregonDecoder_rejects_corrupted_frames);
  RUN_TEST(OregonDecoder_requires_at_least_12_sync_ones);
  RUN_TEST(OregonDecoder_recovers_from_noise_and_partial_frames);
  RUN_TEST(OregonDecoder_completes_frame_on_last_checksum_bit);
  RUN_TEST(OregonDecoder_exposes_measurements_of_oregon_v21);
  RUN_TEST(OregonDecoder_exposes_measurements_of_rflink_captures);
  RUN_TEST(OregonDecoder_clears_fields_absent_from_the_next_device);
  return UNITY_END();
}
