/*
 * Integration tests for FUNC-DECODE-001: X10Decoder rebuilds RFXmeter frames
 * (address, raw value) from level/duration pulses.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/X10Decoder/X10Decoder.h (depends only on <stdint.h>):
 *   class X10Decoder {
 *    public:
 *     bool Feed(bool high, uint16_t durationUs);
 *     uint8_t Address() const;
 *     uint32_t Value() const;
 *   };
 */

#include <stdint.h>
#include <unity.h>

#include <Loopback.h>
#include <TestHal.h>
#include <X10Decoder.h>
#include <vector>
#include <x10rf.h>

struct Timing {
  uint16_t preambleHigh, preambleLow, bitHigh, zeroLow, oneLow;
};

static const Timing nominal = {8960, 4500, 560, 560, 1680};

static std::vector<uint8_t> rfxmeterBytes(uint8_t address, uint32_t value) {
  x10rf encoder(&TestHal, 1);
  encoder.RFXmeter(address, 0, value);
  return std::vector<uint8_t>(encoder.getMessage(), encoder.getMessage() + 6);
}

// Low nibble of the complement of the nibble sum of b0..b4 and b5 >> 4
static uint8_t parityNibble(const std::vector<uint8_t> &bytes) {
  unsigned sum = bytes[5] >> 4;
  for (int i = 0; i < 5; i++) {
    sum += (bytes[i] >> 4) + (bytes[i] & 0x0F);
  }
  return ~sum & 0x0F;
}

static std::vector<Pulse> framePulses(const std::vector<uint8_t> &bytes,
                                      const Timing &t = nominal) {
  std::vector<Pulse> pulses = {{true, t.preambleHigh}, {false, t.preambleLow}};
  for (uint8_t byte : bytes) {
    for (int bit = 7; bit >= 0; bit--) {
      pulses.push_back({true, t.bitHigh});
      pulses.push_back({false, (byte >> bit) & 1 ? t.oneLow : t.zeroLow});
    }
  }
  return pulses;
}

static int countFrames(X10Decoder &decoder, const std::vector<Pulse> &pulses) {
  int frames = 0;
  for (const Pulse &p : pulses) {
    frames += decoder.Feed(p.high, p.durationUs) ? 1 : 0;
  }
  return frames;
}

struct MeterCase {
  const char *id;
  uint8_t address;
  uint32_t value;
};

static const MeterCase meterCases[] = {
    {"zero-address-zero-value", 0x00, 0},
    {"typical", 0x0A, 0x123456},
    {"max-address-max-value", 0xFF, 0xFFFFFF},
    {"alternating-bits", 0x55, 0xAA55AA},
};

// @req FUNC-DECODE-001
void X10Decoder_decodes_each_repeat_of_x10rf_rfxmeter() {
  for (const MeterCase &c : meterCases) {
    // Given
    x10rf encoder(&TestHal, 2);
    TestHal.ClearOrders();
    encoder.RFXmeter(c.address, 0, c.value);
    std::vector<Pulse> pulses = OrdersToPulses(TestHal.Orders);
    X10Decoder decoder;
    int frames = 0;

    // When
    for (const Pulse &p : pulses) {
      if (decoder.Feed(p.high, p.durationUs)) {
        frames++;
        // Then (checked on each true)
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(c.address, decoder.Address(), c.id);
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(c.value, decoder.Value(), c.id);
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
    {"preamble-high-min", {6272, 4500, 560, 560, 1680}, 1},
    {"preamble-high-below", {6271, 4500, 560, 560, 1680}, 0},
    {"preamble-high-max", {11648, 4500, 560, 560, 1680}, 1},
    {"preamble-high-above", {11649, 4500, 560, 560, 1680}, 0},
    {"preamble-low-min", {8960, 3150, 560, 560, 1680}, 1},
    {"preamble-low-below", {8960, 3149, 560, 560, 1680}, 0},
    {"preamble-low-max", {8960, 5850, 560, 560, 1680}, 1},
    {"preamble-low-above", {8960, 5851, 560, 560, 1680}, 0},
    {"bit-high-min", {8960, 4500, 392, 560, 1680}, 1},
    {"bit-high-below", {8960, 4500, 391, 560, 1680}, 0},
    {"bit-high-max", {8960, 4500, 728, 560, 1680}, 1},
    {"bit-high-above", {8960, 4500, 729, 560, 1680}, 0},
    {"zero-low-min", {8960, 4500, 560, 392, 1680}, 1},
    {"zero-low-below", {8960, 4500, 560, 391, 1680}, 0},
    {"zero-low-max", {8960, 4500, 560, 728, 1680}, 1},
    {"zero-low-above", {8960, 4500, 560, 729, 1680}, 0},
    {"one-low-min", {8960, 4500, 560, 560, 1176}, 1},
    {"one-low-below", {8960, 4500, 560, 560, 1175}, 0},
    {"one-low-max", {8960, 4500, 560, 560, 2184}, 1},
    {"one-low-above", {8960, 4500, 560, 560, 2185}, 0},
};

// @req FUNC-DECODE-001
void X10Decoder_accepts_pulses_within_30_percent_windows() {
  for (const WindowCase &c : windowCases) {
    // Given
    std::vector<Pulse> pulses =
        framePulses(rfxmeterBytes(0x0A, 0x123456), c.timing);
    X10Decoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(c.expectedFrames, frames, c.id);
  }
}

struct CorruptionCase {
  const char *id;
  int byteIndex;
  uint8_t xorMask;
  bool fixParity;
};

static const CorruptionCase corruptionCases[] = {
    {"b1-high-nibble-not-complement", 1, 0x10, true},
    {"b1-low-nibble-differs", 1, 0x01, true},
    {"b0-changed-without-b1", 0, 0x10, true},
    {"nonzero-packet-type", 5, 0x10, true},
    {"bad-parity", 5, 0x01, false},
    {"value-changed-without-parity", 3, 0x01, false},
};

// @req FUNC-DECODE-001
void X10Decoder_rejects_corrupted_frames() {
  for (const CorruptionCase &c : corruptionCases) {
    // Given
    std::vector<uint8_t> bytes = rfxmeterBytes(0x0A, 0x123456);
    bytes[c.byteIndex] ^= c.xorMask;
    if (c.fixParity) {
      bytes[5] = (bytes[5] & 0xF0) | parityNibble(bytes);
    }
    X10Decoder decoder;

    // When
    int frames = countFrames(decoder, framePulses(bytes));

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, frames, c.id);
  }
}

// Built by hand, not via x10rf: static init must not depend on TestHal
static const std::vector<Pulse> goodFrame =
    framePulses({0x0A, (~0x0A & 0xF0) | 0x0A, 0x34, 0x56, 0x12, 0x07});

static std::vector<Pulse> truncated(size_t pulseCount) {
  return std::vector<Pulse>(goodFrame.begin(), goodFrame.begin() + pulseCount);
}

struct PrefixCase {
  const char *id;
  std::vector<Pulse> prefix;
};

// Each prefix ends where the decoder state differs; the good frame follows
// with no separator, so its preamble high must be re-checked after the reset.
static const PrefixCase prefixCases[] = {
    {"noise", {{true, 100}, {false, 300}, {true, 20000}, {false, 50}}},
    {"after-preamble-high-expects-low", truncated(1)},
    {"after-preamble-expects-bit-high", truncated(2)},
    {"mid-frame-expects-bit-low", truncated(21)},
    {"mid-frame-expects-bit-high", truncated(40)},
    {"out-of-window-bit-low",
     {{true, 8960}, {false, 4500}, {true, 560}, {false, 3000}}},
    {"wrong-level-after-preamble-high", {{true, 8960}, {true, 4500}}},
};

// @req FUNC-DECODE-001
void X10Decoder_recovers_from_noise_and_partial_frames() {
  for (const PrefixCase &c : prefixCases) {
    // Given
    std::vector<Pulse> pulses = c.prefix;
    pulses.insert(pulses.end(), goodFrame.begin(), goodFrame.end());
    X10Decoder decoder;

    // When
    int frames = countFrames(decoder, pulses);

    // Then
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, frames, c.id);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x0A, decoder.Address(), c.id);
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(0x123456, decoder.Value(), c.id);
  }
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(X10Decoder_decodes_each_repeat_of_x10rf_rfxmeter);
  RUN_TEST(X10Decoder_accepts_pulses_within_30_percent_windows);
  RUN_TEST(X10Decoder_rejects_corrupted_frames);
  RUN_TEST(X10Decoder_recovers_from_noise_and_partial_frames);
  return UNITY_END();
}
