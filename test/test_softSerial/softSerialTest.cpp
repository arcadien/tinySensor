/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019-2023 Aurélien Labrosse
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
 * Production-side requirements for these tests to pass:
 *
 * TECH-SERIAL-003:
 *   - Hal.h must declare: virtual void DelaySerialBitPeriod() = 0;
 *   - TestHal_ must implement it as: Orders.push_back('B');
 *   - Attiny84aHal must implement it as: _delay_us((1000000L / 9600) - 25);
 *
 * TECH-SERIAL-004:
 *   - SoftSerial.cpp must replace all four _delay_us(BIT_PERIOD) calls with
 *     _hal->DelaySerialBitPeriod()
 *   - SoftSerial.cpp must remove #include <util/delay.h>,
 *     #define __DELAY_ROUND_CLOSEST__, and #define BIT_PERIOD
 */

#if defined(NATIVE)

#include <cstring>
#include <stdio.h>
#include <string.h>
#include <unity.h>

#include <SoftSerial.h>
#include <TestHal.h>

/*
 * SoftSerial::print(uint8_t b) bit-bang sequence derivation:
 *
 *   begin():
 *     SerialGoHigh()  -> 'S'
 *     DelaySerialBitPeriod() -> 'B'   (line idle high before first byte)
 *
 *   print(uint8_t b):
 *     1. Start bit: SerialGoLow() -> 'W', DelaySerialBitPeriod() -> 'B'
 *     2. 8 data bits (LSB first, mask = 0x01..0x80):
 *        bit set   -> SerialGoHigh() 'S' + DelaySerialBitPeriod() 'B'
 *        bit clear -> SerialGoLow()  'W' + DelaySerialBitPeriod() 'B'
 *     3. Stop bit: SerialGoHigh() -> 'S', DelaySerialBitPeriod() -> 'B'
 *
 *   For 0x00 (0b00000000):
 *     start:  W B
 *     bit 0:  W B   (0)
 *     bit 1:  W B   (0)
 *     bit 2:  W B   (0)
 *     bit 3:  W B   (0)
 *     bit 4:  W B   (0)
 *     bit 5:  W B   (0)
 *     bit 6:  W B   (0)
 *     bit 7:  W B   (0)
 *     stop:   S B
 *     => "WBWBWBWBWBWBWBWBWBSB" (20 tokens, 10 'B' tokens)
 *
 *   For 0xFF (0b11111111):
 *     start:  W B
 *     bits 0-7: S B x8
 *     stop:   S B
 *     => "WBSBSBSBSBSBSBSBSBSB" (20 tokens, 10 'B' tokens)
 *
 *   For 0x55 (0b01010101):
 *     start:  W B
 *     bit 0 (0x01, set):   S B
 *     bit 1 (0x02, clear): W B
 *     bit 2 (0x04, set):   S B
 *     bit 3 (0x08, clear): W B
 *     bit 4 (0x10, set):   S B
 *     bit 5 (0x20, clear): W B
 *     bit 6 (0x40, set):   S B
 *     bit 7 (0x80, clear): W B
 *     stop:   S B
 *     => "WBSBWBSBWBSBWBSBWBSB" (20 tokens, 10 'B' tokens)
 */

void setUp(void) {
  TestHal.ClearOrders();
}

void tearDown(void) {}

// ---------------------------------------------------------------------------
// TECH-SERIAL-003: TestHal_ records 'B' when DelaySerialBitPeriod() is called
// ---------------------------------------------------------------------------

// @req TECH-SERIAL-003
void Expect_serial_bit_period_token_is_B() {
  // Given
  TestHal.ClearOrders();

  // When
  TestHal.DelaySerialBitPeriod();

  // Then
  TEST_ASSERT_EQUAL_MESSAGE(1u, TestHal.Orders.size(),
                            "Expected exactly 1 token in Orders");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE('B', TestHal.Orders[0],
                                 "Expected token 'B' for DelaySerialBitPeriod");
}

// ---------------------------------------------------------------------------
// TECH-SERIAL-004: SoftSerial::print(uint8_t) produces exactly 10 'B' tokens
// ---------------------------------------------------------------------------

// @req TECH-SERIAL-004
void Expect_softserial_send_char_produces_B_tokens() {
  // Given
  // 0x00: all data bits are zero, so the sequence is deterministic.
  // Expected: 10 bit periods (start + 8 data + stop), each producing one 'B'.
  SoftSerial serial(&TestHal);

  // When
  serial.print((uint8_t)0x00u);

  // Then
  uint8_t bCount = 0;
  for (unsigned char token : TestHal.Orders) {
    if (token == 'B') {
      bCount++;
    }
  }
  TEST_ASSERT_EQUAL_MESSAGE(10u, bCount,
                            "Expected 10 'B' tokens for 10 bit periods "
                            "(start + 8 data + stop) in print(0x00)");
}

// @req TECH-SERIAL-004
void Expect_softserial_send_alternating_bits_produces_B_tokens() {
  // Given
  // 0x55 (0b01010101): alternating bits — exercises both SerialGoHigh and
  // SerialGoLow in the data loop. Each of the 10 bit periods still produces
  // exactly one 'B' token regardless of the bit value.
  SoftSerial serial(&TestHal);

  // When
  serial.print((uint8_t)0x55u);

  // Then
  // clang-format off
  // Expected Orders for 0x55: W B S B W B S B W B S B W B S B W B S B
  //                            ^start  bit0  bit1  bit2  bit3  bit4  bit5  bit6  bit7  stop
  // clang-format on
  const char expected[] = "WBSBWBSBWBSBWBSBWBSB";
  const uint8_t expectedLen = (uint8_t)(sizeof(expected) - 1u);

  TEST_ASSERT_EQUAL_MESSAGE((size_t)expectedLen, TestHal.Orders.size(),
                            "Orders length mismatch for print(0x55)");
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(
      expected, TestHal.Orders.data(), expectedLen,
      "Orders sequence mismatch for print(0x55)");
}

// ---------------------------------------------------------------------------
// TECH-SERIAL-004: SendChar produces 'S' and 'W' tokens (GPIO routing intact)
// ---------------------------------------------------------------------------

// @req TECH-SERIAL-004
void Expect_softserial_uses_hal_serial_pins() {
  // Given
  // 0xFF: start bit is 'W', all 8 data bits are '1' (S), stop bit is 'S'.
  // Expected Orders: W B S B S B S B S B S B S B S B S B S B
  SoftSerial serial(&TestHal);

  // When
  serial.print((uint8_t)0xFFu);

  // Then
  bool hasSerialHigh = false;
  bool hasSerialLow = false;
  for (unsigned char token : TestHal.Orders) {
    if (token == 'S') {
      hasSerialHigh = true;
    }
    if (token == 'W') {
      hasSerialLow = true;
    }
  }
  TEST_ASSERT_TRUE_MESSAGE(hasSerialHigh,
                           "Expected at least one 'S' (SerialGoHigh) token");
  TEST_ASSERT_TRUE_MESSAGE(hasSerialLow,
                           "Expected at least one 'W' (SerialGoLow) token for "
                           "the UART start bit");
}

// @req TECH-SERIAL-004
void Expect_softserial_send_all_zeros_exact_sequence() {
  // Given
  // 0x00: all data bits clear. Verifies the exact full Orders sequence.
  // Expected: start(W B) + 8x data-zero(W B) + stop(S B)
  SoftSerial serial(&TestHal);

  // When
  serial.print((uint8_t)0x00u);

  // Then
  const char expected[] = "WBWBWBWBWBWBWBWBWBSB";
  const uint8_t expectedLen = (uint8_t)(sizeof(expected) - 1u);

  TEST_ASSERT_EQUAL_MESSAGE((size_t)expectedLen, TestHal.Orders.size(),
                            "Orders length mismatch for print(0x00)");
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(
      expected, TestHal.Orders.data(), expectedLen,
      "Orders sequence mismatch for print(0x00)");
}

// ---------------------------------------------------------------------------
// TECH-SERIAL-003: begin() puts the line high and waits one bit period
// ---------------------------------------------------------------------------

// @req TECH-SERIAL-003
void Expect_softserial_begin_emits_idle_high_with_bit_period() {
  // Given
  SoftSerial serial(&TestHal);

  // When
  serial.begin();

  // Then
  // begin() calls SerialGoHigh() then DelaySerialBitPeriod()
  const char expected[] = "SB";
  const uint8_t expectedLen = (uint8_t)(sizeof(expected) - 1u);

  TEST_ASSERT_EQUAL_MESSAGE((size_t)expectedLen, TestHal.Orders.size(),
                            "begin() must emit exactly 'S' then 'B'");
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(expected, TestHal.Orders.data(),
                                       expectedLen,
                                       "begin() Orders sequence mismatch");
}

int main(int, char **) {
  UnityBegin("softSerialTest.cpp");

  RUN_TEST(Expect_serial_bit_period_token_is_B);
  RUN_TEST(Expect_softserial_begin_emits_idle_high_with_bit_period);
  RUN_TEST(Expect_softserial_send_char_produces_B_tokens);
  RUN_TEST(Expect_softserial_send_alternating_bits_produces_B_tokens);
  RUN_TEST(Expect_softserial_uses_hal_serial_pins);
  RUN_TEST(Expect_softserial_send_all_zeros_exact_sequence);

  return UNITY_END();
}

#endif
