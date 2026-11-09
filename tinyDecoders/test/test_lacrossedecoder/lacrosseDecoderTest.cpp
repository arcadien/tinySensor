/*
 * Integration tests for FUNC-DECODE-002: LacrosseDecoder rebuilds WS7000
 * frames (type, address) from level/duration pulses.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/LacrosseDecoder/LacrosseDecoder.h (depends only on
 * <stdint.h>, default constructor):
 *   class LacrosseDecoder {
 *    public:
 *     bool Feed(bool high, uint16_t durationUs);
 *     uint8_t Type() const;
 *     uint8_t Address() const;
 *   };
 */

#include <stdint.h>
#include <unity.h>

#include <LacrosseDecoder.h>
#include <LacrosseWS7000.h>
#include <Loopback.h>
#include <TestHal.h>
#include <vector>

enum Field : uint8_t { TEMP = 1, HUM = 2, PRES = 4, LUX = 8 };

struct Reading {
  uint8_t fields;
  float temperature, humidity, pressure;
  uint16_t lux;
};

static std::vector<Pulse> sentPulses(uint8_t address, const Reading &r,
                                     int sends) {
  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(static_cast<LacrosseWS7000::Address>(address));
  if (r.fields & TEMP)
    encoder.SetTemperature(r.temperature);
  if (r.fields & HUM)
    encoder.SetHumidity(r.humidity);
  if (r.fields & PRES)
    encoder.SetPressure(r.pressure);
  if (r.fields & LUX)
    encoder.SetLuminosity(r.lux);
  TestHal.ClearOrders();
  for (int i = 0; i < sends; i++) {
    encoder.Send();
  }
  return OrdersToPulses(TestHal.Orders);
}

struct Timing {
  uint16_t oneHigh, oneLow, zeroHigh, zeroLow;
};

static const Timing nominal = {400, 800, 800, 400};

// Appends the XOR and sum nibbles of FUNC-LACROSSE-008; the flips corrupt
// them, the sum staying consistent with the transmitted XOR nibble.
static std::vector<uint8_t> withChecks(std::vector<uint8_t> nibbles,
                                       uint8_t xorFlip = 0,
                                       uint8_t sumFlip = 0) {
  uint8_t checkXor = 0, checkSum = 5;
  for (uint8_t n : nibbles) {
    checkXor ^= n;
    checkSum += n;
  }
  checkXor = (checkXor ^ xorFlip) & 0x0F;
  nibbles.push_back(checkXor);
  nibbles.push_back(((checkSum + checkXor) ^ sumFlip) & 0x0F);
  return nibbles;
}

static std::vector<uint8_t> flipped(std::vector<uint8_t> nibbles, size_t index,
                                    uint8_t mask) {
  nibbles[index] ^= mask;
  return nibbles;
}

// Sync zeros and a one, then each nibble LSB-first plus its separator bit
static std::vector<Pulse> framePulses(const std::vector<uint8_t> &nibbles,
                                      const Timing &t = nominal,
                                      int syncZeros = 10,
                                      int zeroSeparatorAfter = -1) {
  std::vector<Pulse> pulses;
  auto bit = [&](bool one) {
    pulses.push_back({true, one ? t.oneHigh : t.zeroHigh});
    pulses.push_back({false, one ? t.oneLow : t.zeroLow});
  };
  for (int i = 0; i < syncZeros; i++) {
    bit(false);
  }
  bit(true);
  for (size_t i = 0; i < nibbles.size(); i++) {
    for (int b = 0; b < 4; b++) {
      bit((nibbles[i] >> b) & 1);
    }
    bit(static_cast<int>(i) != zeroSeparatorAfter);
  }
  return pulses;
}

static int countFrames(LacrosseDecoder &decoder,
                       const std::vector<Pulse> &pulses) {
  int frames = 0;
  for (const Pulse &p : pulses) {
    frames += decoder.Feed(p.high, p.durationUs) ? 1 : 0;
  }
  return frames;
}

// Type 1, address 3, 21.5 °C, 63.7 %
static const std::vector<uint8_t> typeOneNibbles = {1, 3, 5, 1, 2, 7, 3, 6};
static const std::vector<Pulse> goodFrame =
    framePulses(withChecks(typeOneNibbles));

struct FrameCase {
  const char *id;
  uint8_t address;
  Reading reading;
  uint8_t type;
};

static const FrameCase frameCases[] = {
    {"type0-temperature", 0, {TEMP, 21.5f, 0, 0, 0}, 0},
    {"type0-negative-sign-not-in-address", 6, {TEMP, -5.0f, 0, 0, 0}, 0},
    {"type1-temperature-humidity", 3, {TEMP | HUM, 21.5f, 63.7f, 0, 0}, 1},
    {"type4-with-pressure", 5, {TEMP | HUM | PRES, -1.5f, 40, 1013, 0}, 4},
    {"type5-luminosity", 7, {LUX, 0, 0, 0, 1000}, 5},
};

// @req FUNC-DECODE-002
void LacrosseDecoder_decodes_each_repeat_of_lacrosse_ws7000() {
  for (const FrameCase &c : frameCases) {
    // Given
    std::vector<Pulse> pulses = sentPulses(c.address, c.reading, 2);
    LacrosseDecoder decoder;
    int frames = 0;

    // When
    for (const Pulse &p : pulses) {
      if (decoder.Feed(p.high, p.durationUs)) {
        frames++;
        // Then (checked on each true)
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.type, decoder.Type(), c.id);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.address, decoder.Address(), c.id);
      }
    }

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, frames, c.id);
  }
}

struct WindowCase {
  const char *id;
  Timing timing;
  int expectedFrames;
};

static const WindowCase windowCases[] = {
    {"one-high-min", {280, 800, 800, 400}, 1},
    {"one-high-below", {279, 800, 800, 400}, 0},
    {"one-high-max", {520, 800, 800, 400}, 1},
    {"one-high-above", {521, 800, 800, 400}, 0},
    {"one-low-min", {400, 560, 800, 400}, 1},
    {"one-low-below", {400, 559, 800, 400}, 0},
    {"one-low-max", {400, 1040, 800, 400}, 1},
    {"one-low-above", {400, 1041, 800, 400}, 0},
    {"zero-high-min", {400, 800, 560, 400}, 1},
    {"zero-high-below", {400, 800, 559, 400}, 0},
    {"zero-high-max", {400, 800, 1040, 400}, 1},
    {"zero-high-above", {400, 800, 1041, 400}, 0},
    {"zero-low-min", {400, 800, 800, 280}, 1},
    {"zero-low-below", {400, 800, 800, 279}, 0},
    {"zero-low-max", {400, 800, 800, 520}, 1},
    {"zero-low-above", {400, 800, 800, 521}, 0},
};

// @req FUNC-DECODE-002
void LacrosseDecoder_accepts_pulses_within_30_percent_windows() {
  for (const WindowCase &c : windowCases) {
    // Given
    std::vector<Pulse> pulses =
        framePulses(withChecks(typeOneNibbles), c.timing);
    LacrosseDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(c.expectedFrames, frames, c.id);
  }
}

struct CorruptionCase {
  const char *id;
  std::vector<uint8_t> nibbles;
  int zeroSeparatorAfter;
};

// Nibble indexes: 0 type, 1 address, 2..7 data, 8 XOR, 9 sum
static const CorruptionCase corruptionCases[] = {
    {"xor-wrong-sum-consistent", withChecks(typeOneNibbles, 0x2, 0), -1},
    {"sum-wrong", withChecks(typeOneNibbles, 0, 0x1), -1},
    {"data-changed-checks-stale", flipped(withChecks(typeOneNibbles), 3, 0x1),
     -1},
    {"type-2", withChecks(flipped(typeOneNibbles, 0, 0x3)), -1},
    {"type-3", withChecks(flipped(typeOneNibbles, 0, 0x2)), -1},
    {"type-6", withChecks(flipped(typeOneNibbles, 0, 0x7)), -1},
    {"type-15", withChecks(flipped(typeOneNibbles, 0, 0xE)), -1},
    {"zero-separator-after-type", withChecks(typeOneNibbles), 0},
    {"zero-separator-after-data", withChecks(typeOneNibbles), 4},
    {"zero-separator-after-sum", withChecks(typeOneNibbles), 9},
};

// @req FUNC-DECODE-002
void LacrosseDecoder_rejects_corrupted_frames() {
  for (const CorruptionCase &c : corruptionCases) {
    // Given
    std::vector<Pulse> pulses =
        framePulses(c.nibbles, nominal, 10, c.zeroSeparatorAfter);
    LacrosseDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, frames, c.id);
  }
}

struct SyncCase {
  const char *id;
  int syncZeros;
  int expectedFrames;
};

static const SyncCase syncCases[] = {
    {"no-zero", 0, 0},     {"five-zeros", 5, 0}, {"six-zeros", 6, 1},
    {"seven-zeros", 7, 1}, {"ten-zeros", 10, 1},
};

// @req FUNC-DECODE-002
void LacrosseDecoder_requires_at_least_six_sync_zeros() {
  for (const SyncCase &c : syncCases) {
    // Given
    std::vector<Pulse> pulses =
        framePulses(withChecks(typeOneNibbles), nominal, c.syncZeros);
    LacrosseDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(c.expectedFrames, frames, c.id);
  }
}

static std::vector<Pulse> truncated(size_t pulseCount) {
  return std::vector<Pulse>(goodFrame.begin(), goodFrame.begin() + pulseCount);
}

struct PrefixCase {
  const char *id;
  std::vector<Pulse> prefix;
};

// The good frame follows each prefix with no gap; pulses 0..19 are the sync
// zeros, 20..21 the sync one, 22..31 the type bits, 32..33 its separator.
static const PrefixCase prefixCases[] = {
    {"noise", {{true, 100}, {false, 300}, {true, 20000}, {false, 50}}},
    {"low-while-idle", {{false, 400}}},
    {"mid-sync-expects-low", truncated(5)},
    {"after-sync-one-high-expects-low", truncated(21)},
    {"after-sync-expects-type-bit", truncated(22)},
    {"mid-type-expects-low", truncated(27)},
    {"after-type-nibble", truncated(34)},
    {"mid-data-expects-high", truncated(60)},
    {"before-last-separator", truncated(goodFrame.size() - 2)},
    {"out-of-window-low",
     {{true, 800}, {false, 400}, {true, 800}, {false, 2000}}},
    {"wrong-level-mid-sync", {{true, 800}, {false, 400}, {false, 400}}},
};

// @req FUNC-DECODE-002
void LacrosseDecoder_recovers_from_noise_and_partial_frames() {
  for (const PrefixCase &c : prefixCases) {
    // Given
    std::vector<Pulse> pulses = c.prefix;
    pulses.insert(pulses.end(), goodFrame.begin(), goodFrame.end());
    LacrosseDecoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, decoder.Type(), c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, decoder.Address(), c.id);
  }
}

// @req FUNC-DECODE-002
void LacrosseDecoder_completes_frame_on_last_separator_high() {
  // Given
  std::vector<Pulse> pulses = truncated(goodFrame.size() - 1);
  LacrosseDecoder decoder;
  for (size_t i = 0; i + 1 < pulses.size(); i++) {
    TEST_ASSERT_FALSE(decoder.Feed(pulses[i].high, pulses[i].durationUs));
  }

  // When
  bool completed = decoder.Feed(pulses.back().high, pulses.back().durationUs);

  // Then
  TEST_ASSERT_TRUE(completed);
  TEST_ASSERT_EQUAL_UINT8(1, decoder.Type());
  TEST_ASSERT_EQUAL_UINT8(3, decoder.Address());
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(LacrosseDecoder_decodes_each_repeat_of_lacrosse_ws7000);
  RUN_TEST(LacrosseDecoder_accepts_pulses_within_30_percent_windows);
  RUN_TEST(LacrosseDecoder_rejects_corrupted_frames);
  RUN_TEST(LacrosseDecoder_requires_at_least_six_sync_zeros);
  RUN_TEST(LacrosseDecoder_recovers_from_noise_and_partial_frames);
  RUN_TEST(LacrosseDecoder_completes_frame_on_last_separator_high);
  return UNITY_END();
}
