/*
 * Integration tests for TECH-DECODE-001: OrdersToPulses() converts TestHal_
 * Orders tokens into {high, durationUs} pulses.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/Loopback/Loopback.h:
 *   struct Pulse { bool high; uint16_t durationUs; };  // aggregate, this order
 *   std::vector<Pulse> OrdersToPulses(const std::vector<unsigned char>
 * &orders);
 */

#include <stdint.h>
#include <unity.h>

#include <Loopback.h>
#include <TestHal.h>
#include <string>
#include <vector>
#include <x10rf.h>

static std::vector<unsigned char> tokens(const char *orders) {
  std::string s(orders);
  return std::vector<unsigned char>(s.begin(), s.end());
}

static void assertPulsesEqual(const char *id,
                              const std::vector<Pulse> &expected,
                              const std::vector<Pulse> &actual) {
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected.size(), actual.size(), id);
  for (size_t i = 0; i < expected.size(); i++) {
    TEST_ASSERT_EQUAL_MESSAGE(expected[i].high, actual[i].high, id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected[i].durationUs,
                                     actual[i].durationUs, id);
  }
}

struct TokenCase {
  const char *id;
  const char *orders;
  std::vector<Pulse> expected;
};

static const TokenCase tokenCases[] = {
    {"empty-orders-no-pulse", "", {}},
    {"line-starts-low", "D", {{false, 512}}},
    {"start-high-no-leading-low", "HD", {{true, 512}}},
    {"level-change-emits-pulse", "HDLP", {{true, 512}, {false, 1024}}},
    {"same-level-delays-merged", "HDHD", {{true, 1024}}},
    {"repeated-H-is-no-change", "HHD", {{true, 512}}},
    {"other-tokens-ignored", "HDSWXD", {{true, 1024}}},
    {"zero-duration-high-skipped", "HLD", {{false, 512}}},
    {"trailing-pulse-emitted", "HDLD", {{true, 512}, {false, 512}}},
    {"trailing-zero-duration-skipped", "HDL", {{true, 512}}},
    {"duration-A-400", "HA", {{true, 400}}},
    {"duration-D-512", "HD", {{true, 512}}},
    {"duration-P-1024", "HP", {{true, 1024}}},
    {"duration-T-560", "HT", {{true, 560}}},
    {"duration-U-1120", "HU", {{true, 1120}}},
    {"duration-Q-8960", "HQ", {{true, 8960}}},
    {"duration-R-4500", "HR", {{true, 4500}}},
    {"duration-G-40000", "HG", {{true, 40000}}},
    {"saturates-at-65535", "HGG", {{true, 65535}}},
};

// @req TECH-DECODE-001
void OrdersToPulses_converts_hand_written_tokens() {
  for (const TokenCase &c : tokenCases) {
    // Given
    std::vector<unsigned char> orders = tokens(c.orders);

    // When
    std::vector<Pulse> pulses = OrdersToPulses(orders);

    // Then
    assertPulsesEqual(c.id, c.expected, pulses);
  }
}

// @req TECH-DECODE-001
void OrdersToPulses_loops_back_x10_rfxmeter_frame() {
  // Given
  // one repeat of a 6-byte RFXmeter frame: preamble (2 pulses), 48 data bits
  // and a trailing 1-bit (2 pulses each); the gap stretches the last low pulse
  x10rf encoder(&TestHal, 1);
  TestHal.ClearOrders();
  encoder.RFXmeter(0x0A, 0x00, 0);

  // When
  std::vector<Pulse> pulses = OrdersToPulses(TestHal.Orders);

  // Then
  TEST_ASSERT_EQUAL_INT(2 + 2 * (6 * 8 + 1), pulses.size());
  assertPulsesEqual("x10-preamble-and-first-bit-high",
                    {{true, 8960}, {false, 4500}, {true, 560}},
                    std::vector<Pulse>(pulses.begin(), pulses.begin() + 3));
  assertPulsesEqual("x10-last-bit-low-plus-gap", {{false, 560 + 1120 + 40000}},
                    {pulses.back()});
  for (size_t i = 1; i < pulses.size(); i++) {
    TEST_ASSERT_NOT_EQUAL_MESSAGE(pulses[i - 1].high, pulses[i].high,
                                  "x10 pulses must alternate levels");
  }
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(OrdersToPulses_converts_hand_written_tokens);
  RUN_TEST(OrdersToPulses_loops_back_x10_rfxmeter_frame);
  return UNITY_END();
}
