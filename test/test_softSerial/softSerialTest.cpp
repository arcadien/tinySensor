/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019 Aurélien Labrosse
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
#if defined(NATIVE)

#include <SoftSerial.h>
#include <TestHal.h>
#include <string>
#include <unity.h>

// Each SerialWriteBit call records one token: 'S' = high, 'W' = low.
// A UART frame is 10 tokens: start (W), 8 data bits LSB first, stop (S).
static const size_t FRAME_LEN = 10;

// Decodes the recorded frames back to the transmitted string.
// Fails the test if a frame is malformed.
static void DecodeFrames(std::string &decoded) {
  const std::vector<unsigned char> &orders = TestHal.Orders;
  TEST_ASSERT_EQUAL_MESSAGE(0, orders.size() % FRAME_LEN,
                            "Orders must contain whole UART frames");
  for (size_t f = 0; f < orders.size(); f += FRAME_LEN) {
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('W', orders[f], "Start bit must be low");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE('S', orders[f + 9], "Stop bit must be high");
    uint8_t byte = 0;
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (orders[f + 1 + bit] == 'S') {
        byte |= (1 << bit);
      }
    }
    decoded += (char)byte;
  }
}

// @req TECH-SERIAL-001
void Expect_begin_sets_line_idle_high() {
  SoftSerial serial(&TestHal);
  serial.begin();
  TEST_ASSERT_EQUAL(1, TestHal.Orders.size());
  TEST_ASSERT_EQUAL_CHAR('S', TestHal.Orders[0]);
}

// @req TECH-SERIAL-001
void Expect_byte_frame_is_start_lsb_first_data_stop() {
  SoftSerial serial(&TestHal);
  serial.print((uint8_t)0x41);  // 'A' = 0b01000001
  const unsigned char expected[] = {'W', 'S', 'W', 'W', 'W',
                                    'W', 'W', 'S', 'W', 'S'};
  TEST_ASSERT_EQUAL(FRAME_LEN, TestHal.Orders.size());
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, TestHal.Orders.data(), FRAME_LEN);
}

// @req TECH-SERIAL-001
void Expect_string_is_sent_frame_by_frame() {
  SoftSerial serial(&TestHal);
  serial.print("Vbatt: ");
  std::string decoded;
  DecodeFrames(decoded);
  TEST_ASSERT_EQUAL_STRING("Vbatt: ", decoded.c_str());
}

// @req TECH-SERIAL-001
void Expect_numbers_are_sent_as_decimal_text() {
  SoftSerial serial(&TestHal);
  serial.print(0);
  serial.print(' ');
  serial.print(3712u);
  serial.print(' ');
  serial.print(-42L);
  serial.print(' ');
  serial.print(4294967295UL);
  std::string decoded;
  DecodeFrames(decoded);
  TEST_ASSERT_EQUAL_STRING("0 3712 -42 4294967295", decoded.c_str());
}

// @req TECH-SERIAL-001
void Expect_base_argument_selects_radix() {
  SoftSerial serial(&TestHal);
  serial.print(255L, 16);
  serial.print(' ');
  serial.print(5L, 2);
  serial.print(' ');
  serial.print(65L, 0);  // base 0 = raw character
  std::string decoded;
  DecodeFrames(decoded);
  TEST_ASSERT_EQUAL_STRING("FF 101 A", decoded.c_str());
}

// @req TECH-SERIAL-001
void Expect_println_appends_crlf() {
  SoftSerial serial(&TestHal);
  serial.println("ok");
  serial.println();
  std::string decoded;
  DecodeFrames(decoded);
  TEST_ASSERT_EQUAL_STRING("ok\r\n\r\n", decoded.c_str());
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UnityBegin("softSerialTest.cpp");

  RUN_TEST(Expect_begin_sets_line_idle_high);
  RUN_TEST(Expect_byte_frame_is_start_lsb_first_data_stop);
  RUN_TEST(Expect_string_is_sent_frame_by_frame);
  RUN_TEST(Expect_numbers_are_sent_as_decimal_text);
  RUN_TEST(Expect_base_argument_selects_radix);
  RUN_TEST(Expect_println_appends_crlf);

  return UNITY_END();
}
#endif
