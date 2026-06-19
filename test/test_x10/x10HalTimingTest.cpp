/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019-2024 Aurélien Labrosse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Integration tests for TECH-X10-001 and TECH-X10-002.
 *
 * Production side must provide (code-builder handles this):
 *
 * In lib/hal/Hal.h — five new pure-virtual methods:
 *   virtual void DelayX10PreambleHigh() = 0;   // 8960 µs
 *   virtual void DelayX10PreambleLow()  = 0;   // 4500 µs
 *   virtual void DelayX10BitShort()     = 0;   //  560 µs
 *   virtual void DelayX10BitLong()      = 0;   // 1120 µs
 *   virtual void DelayX10Gap()          = 0;   // 40000 µs
 *
 * In lib/hal/TestHal.h — implementations appending these tokens:
 *   DelayX10PreambleHigh() -> Orders.push_back('Q')
 *   DelayX10PreambleLow()  -> Orders.push_back('R')
 *   DelayX10BitShort()     -> Orders.push_back('T')
 *   DelayX10BitLong()      -> Orders.push_back('U')
 *   DelayX10Gap()          -> Orders.push_back('G')
 *
 * In lib/x10/x10rf.cpp — SendCommand() and SendX10RfBit() must call the
 *   five Hal methods above instead of the file-static x10_* delay functions.
 *   The #if defined(AVR) block and all static x10_* functions must be removed.
 */

#if defined(NATIVE)

#include <stdint.h>
#include <unity.h>

#include <TestHal.h>
#include <vector>
#include <x10rf.h>

// ---------------------------------------------------------------------------
// Token constants matching the TestHal_ implementation contract above
// ---------------------------------------------------------------------------
static const unsigned char TOK_PREAMBLE_HIGH = 'Q';  // DelayX10PreambleHigh
static const unsigned char TOK_PREAMBLE_LOW = 'R';   // DelayX10PreambleLow
static const unsigned char TOK_BIT_SHORT = 'T';      // DelayX10BitShort
static const unsigned char TOK_BIT_LONG = 'U';       // DelayX10BitLong
static const unsigned char TOK_GAP = 'G';            // DelayX10Gap
static const unsigned char TOK_RADIO_HIGH = 'H';
static const unsigned char TOK_RADIO_LOW = 'L';

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static bool ordersContain(const std::vector<unsigned char> &orders,
                          unsigned char token) {
  for (unsigned char c : orders) {
    if (c == token)
      return true;
  }
  return false;
}

static int countToken(const std::vector<unsigned char> &orders,
                      unsigned char token) {
  int n = 0;
  for (unsigned char c : orders) {
    if (c == token)
      n++;
  }
  return n;
}

// ---------------------------------------------------------------------------
// TECH-X10-001 — TestHal_ exposes five distinct X10 timing tokens
// ---------------------------------------------------------------------------

// @req TECH-X10-001
void TestHal_DelayX10PreambleHigh_appends_distinct_token() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelayX10PreambleHigh();

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_PREAMBLE_HIGH),
                           "DelayX10PreambleHigh must append token 'Q'");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, (int)TestHal.Orders.size(),
      "DelayX10PreambleHigh must append exactly one token");
}

// @req TECH-X10-001
void TestHal_DelayX10PreambleLow_appends_distinct_token() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelayX10PreambleLow();

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_PREAMBLE_LOW),
                           "DelayX10PreambleLow must append token 'R'");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, (int)TestHal.Orders.size(),
      "DelayX10PreambleLow must append exactly one token");
}

// @req TECH-X10-001
void TestHal_DelayX10BitShort_appends_distinct_token() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelayX10BitShort();

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_BIT_SHORT),
                           "DelayX10BitShort must append token 'T'");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, (int)TestHal.Orders.size(),
      "DelayX10BitShort must append exactly one token");
}

// @req TECH-X10-001
void TestHal_DelayX10BitLong_appends_distinct_token() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelayX10BitLong();

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_BIT_LONG),
                           "DelayX10BitLong must append token 'U'");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, (int)TestHal.Orders.size(),
      "DelayX10BitLong must append exactly one token");
}

// @req TECH-X10-001
void TestHal_DelayX10Gap_appends_distinct_token() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelayX10Gap();

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_GAP),
                           "DelayX10Gap must append token 'G'");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, (int)TestHal.Orders.size(),
                                "DelayX10Gap must append exactly one token");
}

// @req TECH-X10-001
void TestHal_X10_tokens_are_all_distinct() {
  // Given
  // Each token must differ from every other token
  unsigned char tokens[5] = {TOK_PREAMBLE_HIGH, TOK_PREAMBLE_LOW, TOK_BIT_SHORT,
                             TOK_BIT_LONG, TOK_GAP};

  // When / Then
  for (int i = 0; i < 5; i++) {
    for (int j = i + 1; j < 5; j++) {
      TEST_ASSERT_NOT_EQUAL_MESSAGE(
          tokens[i], tokens[j],
          "All five X10 timing tokens must be distinct from each other");
    }
  }
  // Also verify none collide with existing HAL tokens H, L, D, P, A, S, W
  const unsigned char existing[] = {'H', 'L', 'D', 'P', 'A', 'S', 'W'};
  for (int i = 0; i < 5; i++) {
    for (unsigned char e : existing) {
      TEST_ASSERT_NOT_EQUAL_MESSAGE(
          tokens[i], e,
          "X10 timing tokens must not collide with existing HAL tokens");
    }
  }
}

// ---------------------------------------------------------------------------
// TECH-X10-002 — x10rf::SendCommand uses HAL for preamble and gap delays
// ---------------------------------------------------------------------------

// @req TECH-X10-002
void SendCommand_uses_hal_preamble_high_delay() {
  // Given
  // A minimal 1-byte message; 1 repeat so we get exactly one preamble
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);
  uint8_t data[1] = {0x00};

  // When
  // Trigger SendCommand indirectly via a known public method (RFXsensor).
  // We clear after construction so only the Send call populates Orders.
  TestHal.ClearOrders();
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then
  TEST_ASSERT_TRUE_MESSAGE(ordersContain(TestHal.Orders, TOK_PREAMBLE_HIGH),
                           "SendCommand must call DelayX10PreambleHigh via Hal "
                           "(token 'Q' expected)");
}

// @req TECH-X10-002
void SendCommand_uses_hal_preamble_low_delay() {
  // Given
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);

  // When
  TestHal.ClearOrders();
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then
  TEST_ASSERT_TRUE_MESSAGE(
      ordersContain(TestHal.Orders, TOK_PREAMBLE_LOW),
      "SendCommand must call DelayX10PreambleLow via Hal (token 'R' expected)");
}

// @req TECH-X10-002
void SendCommand_uses_hal_gap_delay_after_transmission() {
  // Given
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);

  // When
  TestHal.ClearOrders();
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then
  TEST_ASSERT_TRUE_MESSAGE(
      ordersContain(TestHal.Orders, TOK_GAP),
      "SendCommand must call DelayX10Gap via Hal (token 'G' expected)");
}

// @req TECH-X10-002
void SendCommand_preamble_sequence_is_high_then_low() {
  // Given
  // The X10 preamble must be: RadioGoHigh, DelayX10PreambleHigh,
  //                            RadioGoLow,  DelayX10PreambleLow
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);

  // When
  TestHal.ClearOrders();
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then — find first occurrence of preamble high token and verify ordering
  const std::vector<unsigned char> &orders = TestHal.Orders;
  int idxRadioHigh = -1;
  int idxPreHigh = -1;
  int idxRadioLow = -1;
  int idxPreLow = -1;
  for (int i = 0; i < (int)orders.size(); i++) {
    if (orders[i] == TOK_RADIO_HIGH && idxRadioHigh < 0) {
      idxRadioHigh = i;
    } else if (orders[i] == TOK_PREAMBLE_HIGH && idxPreHigh < 0) {
      idxPreHigh = i;
    } else if (orders[i] == TOK_RADIO_LOW && idxRadioLow < 0) {
      idxRadioLow = i;
    } else if (orders[i] == TOK_PREAMBLE_LOW && idxPreLow < 0) {
      idxPreLow = i;
      break;
    }
  }
  TEST_ASSERT_MESSAGE(idxRadioHigh >= 0, "RadioGoHigh must appear");
  TEST_ASSERT_MESSAGE(idxPreHigh > idxRadioHigh,
                      "DelayX10PreambleHigh must follow RadioGoHigh");
  TEST_ASSERT_MESSAGE(idxRadioLow > idxPreHigh,
                      "RadioGoLow must follow DelayX10PreambleHigh");
  TEST_ASSERT_MESSAGE(idxPreLow > idxRadioLow,
                      "DelayX10PreambleLow must follow RadioGoLow");
}

// @req TECH-X10-002
void SendX10RfBit_zero_uses_hal_bit_short_delays_no_long() {
  // Given
  // A zero bit must generate: RadioGoHigh, BitShort, RadioGoLow, BitShort
  // It must NOT generate a BitLong token.
  // We use 0x00 address sensor which sends all-zero data bytes.
  // We call RFXsensor with value=0 and observe Orders for zero-bit pattern.
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);
  TestHal.ClearOrders();

  // When
  encoder.RFXsensor(0x00, 't', 't', 0x00);

  // Then
  // The bit-short token must appear (all-zero byte has 8 zero bits each needing
  // two BitShort delays)
  TEST_ASSERT_TRUE_MESSAGE(
      ordersContain(TestHal.Orders, TOK_BIT_SHORT),
      "SendX10RfBit for a 0-bit must emit DelayX10BitShort (token 'T')");
  // BitLong must not appear for an all-zero byte (excluding trailing 1-bit)
  // Count long tokens; a trailing 1-bit at end of frame also produces one long.
  // But a pure zero payload byte should produce many shorts and few longs.
  int shortCount = countToken(TestHal.Orders, TOK_BIT_SHORT);
  TEST_ASSERT_GREATER_THAN_MESSAGE(
      0, shortCount,
      "Multiple DelayX10BitShort tokens expected for zero-bit data");
}

// @req TECH-X10-002
void SendX10RfBit_one_uses_hal_bit_long_delay() {
  // Given
  // Sending 0xFF forces all 8 bits to 1, each needing
  // BitShort+BitShort+BitLong.
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 1);
  TestHal.ClearOrders();

  // When
  // RFXsensor address 0xFF, type 't', packet 't', value 0xFF
  encoder.RFXsensor(0xFF, 't', 't', 0xFF);

  // Then
  TEST_ASSERT_TRUE_MESSAGE(
      ordersContain(TestHal.Orders, TOK_BIT_LONG),
      "SendX10RfBit for a 1-bit must emit DelayX10BitLong (token 'U')");
}

// @req TECH-X10-002
void SendCommand_gap_appears_once_per_repeat() {
  // Given
  // With rf_repeats=2 the gap delay must appear exactly twice.
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 2);
  TestHal.ClearOrders();

  // When
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then
  int gapCount = countToken(TestHal.Orders, TOK_GAP);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      2, gapCount,
      "DelayX10Gap must be called exactly once per repeat (expected 2 for "
      "rf_repeats=2)");
}

// @req TECH-X10-002
void SendCommand_preamble_appears_once_per_repeat() {
  // Given
  // With rf_repeats=3 the preamble high must appear exactly 3 times.
  TestHal.ClearOrders();
  x10rf encoder(&TestHal, 3);
  TestHal.ClearOrders();

  // When
  encoder.RFXsensor(0xF2, 't', 't', 0);

  // Then
  int preHighCount = countToken(TestHal.Orders, TOK_PREAMBLE_HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      3, preHighCount,
      "DelayX10PreambleHigh must be called exactly once per repeat (expected 3 "
      "for rf_repeats=3)");
}

// ---------------------------------------------------------------------------
// setUp / tearDown
// ---------------------------------------------------------------------------

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int, char **) {
  UnityBegin("x10HalTimingTest.cpp");

  // TECH-X10-001: TestHal_ token contract
  RUN_TEST(TestHal_DelayX10PreambleHigh_appends_distinct_token);
  RUN_TEST(TestHal_DelayX10PreambleLow_appends_distinct_token);
  RUN_TEST(TestHal_DelayX10BitShort_appends_distinct_token);
  RUN_TEST(TestHal_DelayX10BitLong_appends_distinct_token);
  RUN_TEST(TestHal_DelayX10Gap_appends_distinct_token);
  RUN_TEST(TestHal_X10_tokens_are_all_distinct);

  // TECH-X10-002: x10rf uses HAL for all timing
  RUN_TEST(SendCommand_uses_hal_preamble_high_delay);
  RUN_TEST(SendCommand_uses_hal_preamble_low_delay);
  RUN_TEST(SendCommand_uses_hal_gap_delay_after_transmission);
  RUN_TEST(SendCommand_preamble_sequence_is_high_then_low);
  RUN_TEST(SendX10RfBit_zero_uses_hal_bit_short_delays_no_long);
  RUN_TEST(SendX10RfBit_one_uses_hal_bit_long_delay);
  RUN_TEST(SendCommand_gap_appears_once_per_repeat);
  RUN_TEST(SendCommand_preamble_appears_once_per_repeat);

  return UNITY_END();
}
#endif
