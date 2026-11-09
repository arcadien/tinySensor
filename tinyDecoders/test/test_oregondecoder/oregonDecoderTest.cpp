/*
 * Integration tests for FUNC-DECODE-004: OregonDecoder rebuilds Oregon v3
 * frames (device ID, channel, rolling code, battery) from level/duration
 * pulses.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/OregonDecoder/OregonDecoder.h (depends only on
 * <stdint.h>, default constructor):
 *   class OregonDecoder {
 *    public:
 *     bool Feed(bool high, uint16_t durationUs);
 *     uint16_t DeviceId() const;
 *     uint8_t Channel() const;
 *     uint8_t RollingCode() const;
 *     bool BatteryLow() const;
 *   };
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

static size_t frameBits(const std::vector<uint8_t> &bytes) {
  return bytes[0] == 0xEC ? 60 : bytes[0] == 0x1D ? 72 : 88;
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

// THN132N, channel 1, rolling code 0x3C, battery low, -10.2 °C
static const std::vector<uint8_t> thn132nBytes = {
    0xEC, 0x40, 0x13, 0xC4, 0x20, 0x18, 0x30, 0x03, 0x00, 0x00, 0x00};
// THGN123N, channel 2, rolling code 0xA5, 23.7 °C, 45 %
static const std::vector<uint8_t> thgn123nBytes = {
    0x1D, 0x20, 0x2A, 0x50, 0x73, 0x20, 0x54, 0x00, 0x2C, 0x00, 0x00};
// BTHR918, channel 3, rolling code 0x7E, battery low, 21.5 °C, 60 %, 1013 hPa
static const std::vector<uint8_t> bthr918Bytes = {
    0x5A, 0x5D, 0x47, 0xE4, 0x51, 0x20, 0x06, 0x0A, 0xD0, 0x60, 0x5F};

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
    {"thn132n", {1, 0x3C, true}, {TEMP, -10.2f, 0, 0}, 0xEC40},
    {"thgn123n", {2, 0xA5, false}, {TEMP | HUM, 23.7f, 45, 0}, 0x1D20},
    {"bthr918", {3, 0x7E, true}, {TEMP | HUM | PRES, 21.5f, 60, 1013}, 0x5A5D},
};

// @req FUNC-DECODE-004
void OregonDecoder_decodes_each_repeat_of_oregon_v3() {
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

struct WindowCase {
  const char *id;
  uint16_t shortUs, longUs;
  int expectedFrames;
};

static const WindowCase windowCases[] = {
    {"short-min", 358, 1024, 1}, {"short-below", 357, 1024, 0},
    {"short-max", 666, 1024, 1}, {"short-above", 667, 1024, 0},
    {"long-min", 512, 717, 1},   {"long-below", 512, 716, 0},
    {"long-max", 512, 1331, 1},  {"long-above", 512, 1332, 0},
    {"both-min", 358, 717, 1},   {"both-max", 666, 1331, 1},
};

// @req FUNC-DECODE-004
void OregonDecoder_accepts_pulses_within_30_percent_windows() {
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

struct CorruptionCase {
  const char *id;
  std::vector<uint8_t> bytes;
};

static const CorruptionCase corruptionCases[] = {
    {"thn132n-data-changed-checksum-stale", flipped(thn132nBytes, 4, 0x01)},
    {"thn132n-checksum-low-nibble", flipped(thn132nBytes, 6, 0x10)},
    {"thn132n-checksum-high-nibble", flipped(thn132nBytes, 7, 0x01)},
    {"thgn123n-checksum", flipped(thgn123nBytes, 8, 0x80)},
    {"thgn123n-humidity-changed", flipped(thgn123nBytes, 6, 0x01)},
    {"bthr918-checksum", flipped(bthr918Bytes, 10, 0x01)},
    {"bthr918-pressure-changed", flipped(bthr918Bytes, 8, 0x10)},
    {"unknown-id-ec41", withChecksum(flipped(thn132nBytes, 1, 0x01), 13)},
    {"unknown-id-ed40", withChecksum(flipped(thn132nBytes, 0, 0x01), 13)},
    {"unknown-id-1d21", withChecksum(flipped(thgn123nBytes, 1, 0x01), 16)},
    {"unknown-id-5a5c", withChecksum(flipped(bthr918Bytes, 1, 0x01), 20)},
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

// 0x1D starts with a one bit, which counts in the sync run
static const SyncCase syncCases[] = {
    {"no-one", thn132nBytes, 0, 0},
    {"15-ones", thn132nBytes, 15, 0},
    {"16-ones", thn132nBytes, 16, 1},
    {"17-ones", thn132nBytes, 17, 1},
    {"24-ones", thn132nBytes, 24, 1},
    {"thgn123n-14-plus-first-bit", thgn123nBytes, 14, 0},
    {"thgn123n-15-plus-first-bit", thgn123nBytes, 15, 1},
    {"bthr918-15-ones", bthr918Bytes, 15, 0},
    {"bthr918-16-ones", bthr918Bytes, 16, 1},
};

// @req FUNC-DECODE-004
void OregonDecoder_requires_at_least_16_sync_ones() {
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
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(0xEC40, decoder.DeviceId(), c.id);
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
  TEST_ASSERT_EQUAL_HEX16(0xEC40, decoder.DeviceId());
  TEST_ASSERT_EQUAL_UINT8(1, decoder.Channel());
  TEST_ASSERT_EQUAL_HEX8(0x3C, decoder.RollingCode());
  TEST_ASSERT_TRUE(decoder.BatteryLow());
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(OregonDecoder_decodes_each_repeat_of_oregon_v3);
  RUN_TEST(OregonDecoder_maps_channel_nibble_to_channel);
  RUN_TEST(OregonDecoder_accepts_pulses_within_30_percent_windows);
  RUN_TEST(OregonDecoder_rejects_corrupted_frames);
  RUN_TEST(OregonDecoder_requires_at_least_16_sync_ones);
  RUN_TEST(OregonDecoder_recovers_from_noise_and_partial_frames);
  RUN_TEST(OregonDecoder_completes_frame_on_last_checksum_bit);
  return UNITY_END();
}
