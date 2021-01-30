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
#if !defined(AVR)

#include <protocol/Oregon_v2.h>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

namespace {


std::string MessageNibblesToString(unsigned char *message,
                                   unsigned char length) {

  std::stringstream ss;
  for (int index = 0; index < length; index++) {
    unsigned char currentMessageElement = message[index];

    if (index == 8) {
      ss << +currentMessageElement;
    } else {
      unsigned char lsb = message[index] & 0x0F;
      unsigned char msb = message[index] >> 4;
      ss << +msb;
      ss << +lsb;
    }
  }
  return ss.str();
} // namespace

class TestHal : public Hal {
public:
  static const int ORDERS_COUNT_FOR_A_BYTE = 6;
  const static unsigned char EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE];
  const static unsigned char EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE];
  // 6 unsigned chars per bit, 24 times "1"
  const static int PREAMBLE_BYTE_LENGTH = 24 * 6;
  const static unsigned char EXPECTED_PREAMBLE[PREAMBLE_BYTE_LENGTH];

  const static int POSTAMBLE_BYTE_LENGTH = 2 * 6;
  const static unsigned char EXPECTED_POSTAMBLE[POSTAMBLE_BYTE_LENGTH];

  const static int SYNC_BYTE_LENGTH = 4 * 6;
  const static unsigned char EXPECTED_SYNC[SYNC_BYTE_LENGTH];

  void DelayPeriod() const override { Orders.push_back('P'); }
  void DelayHalfPeriod() const override { Orders.push_back('D'); }

  inline void GoHigh() const override { Orders.push_back('H'); }
  inline void GoLow() const override { Orders.push_back('L'); }

  unsigned char *GetOrders() { return &Orders[0]; }

  mutable std::vector<unsigned char> Orders;
};

const unsigned char TestHal::EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE] =
    {'H', 'D', 'L', 'P', 'H', 'D'};

const unsigned char TestHal::EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE] =
    {'L', 'D', 'H', 'P', 'L', 'D'};

const unsigned char
    TestHal::EXPECTED_POSTAMBLE[TestHal::POSTAMBLE_BYTE_LENGTH] = {
        // postamble : two nibbles at zero
        'H', 'D', 'L', 'P', 'H', 'D', // 0
        'H', 'D', 'L', 'P', 'H', 'D'  // 0
};

const unsigned char TestHal::EXPECTED_SYNC[TestHal::SYNC_BYTE_LENGTH] = {
    'H', 'D', 'L', 'P', 'H', 'D', // 0
    'L', 'D', 'H', 'P', 'L', 'D', // 1
    'H', 'D', 'L', 'P', 'H', 'D', // 0
    'L', 'D', 'H', 'P', 'L', 'D'  // 1
};

const unsigned char TestHal::EXPECTED_PREAMBLE[TestHal::PREAMBLE_BYTE_LENGTH] =
    {
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
        'L', 'D', 'H', 'P', 'L', 'D', // 1
};

} // namespace

/*
 * This testsuite is based on the document
 * "434MHz RF Protocol Descriptions for Wireless Weather Sensors,
 *  Oregon Scientific, AcuRite, Ambient Weather
 *  October 2015"
 *  A copy of the document should be provided along the implementation
 */
void setUp(void) {}

void tearDown(void) {}

// ---------------- Test helpers tests
void Expect_bitread_to_read_each_bit_separately() {

  std::string message = "";

  uint8_t value = 0;

  for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
    message = "Failed with value==0 for bit " + std::to_string(bitIndex);
    TEST_ASSERT_FALSE_MESSAGE(BitRead(value, bitIndex), message.c_str());
  }

  value = 1; // 0b00000001
  for (uint8_t positionOfOne = 0; positionOfOne < 8; ++positionOfOne) {

    for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
      message = "Failed when 1 is at index " + std::to_string(positionOfOne) +
                ", for bit " + std::to_string(bitIndex);
      if (positionOfOne == bitIndex) {
        TEST_ASSERT_TRUE_MESSAGE(BitRead(value, bitIndex), message.c_str());
      } else {
        TEST_ASSERT_FALSE_MESSAGE(BitRead(value, bitIndex), message.c_str());
      }
    }
    value <<= 1; // 0b00000010
  }
}

// ----------------- Oregon tests

void Expect_nibbles_to_be_sent_lsb_first() {
  uint8_t value = 0b00010001;

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));

  // Send one byte
  tinySensor.SendData(&value, 1);

  unsigned char expectedOrders[8 * TestHal::ORDERS_COUNT_FOR_A_BYTE] = {
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D'  // 0
  };
  auto actualOrdersForOneByte = testHal.GetOrders();
  TEST_ASSERT_EQUAL_INT8_ARRAY(expectedOrders, actualOrdersForOneByte,
                               8 * TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_good_hardware_orders_for_zero() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SendZero();
  auto actualOrdersForZero = testHal.GetOrders();
  TEST_ASSERT_EQUAL_INT8_ARRAY(TestHal::EXPECTED_ORDERS_FOR_ZERO,
                               actualOrdersForZero,
                               TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_good_hardware_orders_for_one() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SendOne();
  unsigned char *actualOrdersForOne = testHal.GetOrders();
  TEST_ASSERT_EQUAL_INT8_ARRAY(TestHal::EXPECTED_ORDERS_FOR_ONE,
                               actualOrdersForOne,
                               TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_messages_to_have_preamble_sync_and_postamble() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));

  tinySensor.Send();

  auto ordersCount = (6 * 4 + 4 + 2) * TestHal::ORDERS_COUNT_FOR_A_BYTE;

  unsigned char *expected = new unsigned char[ordersCount];

  memcpy(expected, TestHal::EXPECTED_PREAMBLE,
         TestHal::PREAMBLE_BYTE_LENGTH * sizeof(unsigned char));

  memcpy(expected + TestHal::PREAMBLE_BYTE_LENGTH, TestHal::EXPECTED_SYNC,
         TestHal::SYNC_BYTE_LENGTH * sizeof(unsigned char));

  memcpy(expected + TestHal::PREAMBLE_BYTE_LENGTH + TestHal::SYNC_BYTE_LENGTH,
         TestHal::EXPECTED_POSTAMBLE,
         TestHal::POSTAMBLE_BYTE_LENGTH * sizeof(unsigned char));

  unsigned char *actualOrdersForTemperatureOnly;
  actualOrdersForTemperatureOnly = testHal.GetOrders();
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualOrdersForTemperatureOnly,
                               ordersCount);
}

void Expect_right_positive_temperature_encoding()

{
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetTemperature(27.5);

  unsigned char byte4 = 5 << 4;
  unsigned char byte5 = 2 << 4 | 7;
  unsigned char byte6 = 0x0; // positive temp
  unsigned char *expected = new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

  unsigned char *actualMessage = tinySensor.Message;

  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with positive temp. with dozen");
}

void Expect_right_pressure_encoding() {

  int actualPressureInHPa = 900;
  unsigned char byte8 = actualPressureInHPa - OregonV3::PRESSURE_SCALING_VALUE;
  unsigned char byte9 = 0xC0;
  unsigned char *expected = new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, 0, 0, 0, 0, byte8, byte9};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetPressure(actualPressureInHPa);
  unsigned char *actualMessage = tinySensor.Message;

  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with possible pressure value");
}

void Expect_right_negative_temperature_encoding() {
  {

    unsigned char byte4 = 5 << 4;
    unsigned char byte5 = 2 << 4 | 7;
    unsigned char byte6 = 0x08; // negative temp
    unsigned char *expected =
        new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
            0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));
    tinySensor.SetTemperature(-27.5);
    unsigned char *actualMessage = tinySensor.Message;

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp. with dozen");
  }
  {

    unsigned char byte4 = 4 << 4;
    unsigned char byte5 = 8;
    unsigned char byte6 = 0x08; // negative temp
    unsigned char *expected =
        new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
            0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));
    tinySensor.SetTemperature(-8.4);
    unsigned char *actualMessage = tinySensor.Message;

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp without dozen");
  }
}

void Expect_right_humidity_encoding() {

  unsigned char byte6 = 2 << 4;
  unsigned char byte7 = 5;
  unsigned char *expected = new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, 0, 0, byte6, byte7, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetHumidity(52);
  unsigned char *actualMessage = tinySensor.Message;

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}
void Expect_right_channel_encoding() {

  struct Given {
    const char *label;
    unsigned char channel;
    unsigned char *expected;
    Given(const char *label, unsigned char channel, unsigned char *expected)
        : channel(channel), expected(std::move(expected)) {}
  };
  std::vector<Given> givens;

  // implemented sensors use the coding 1 << (ch –1), where ch is 1, 2 or 3.

  unsigned char channelByte = 0b00010000;
  givens.push_back(Given("channel 1", 1,
                         new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  channelByte = 0b00100000;
  givens.push_back(Given("channel 2", 2,
                         new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  channelByte = 0b01000000;
  givens.push_back(Given("channel 3", 3,
                         new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  for (auto const given : givens) {
    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));

    tinySensor.SetChannel(given.channel);
    unsigned char *actualMessage = tinySensor.Message;

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(given.expected, actualMessage,
                                         OregonV3::MESSAGE_SIZE_IN_BYTES,
                                         given.label);
  }
}

void Expect_right_rolling_code_encoding() {

  //  byte:      0    1    2    3    4     5      6      7      8      9
  // nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
  //
  // nibbles 5..6    : rolling code

  unsigned char *expected = new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, (unsigned char)8, (unsigned char)(5 << 4), 0, 0, 0, 0, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetRollingCode(85);
  unsigned char *actualMessage = tinySensor.Message;

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}

void Expect_right_low_battery_encoding() {

  //  byte:      0    1    2    3    4     5      6      7      8      9
  // nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
  //                            C
  // nibbles 7    : low batt

  unsigned char *expected = new unsigned char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0xC, 0, 0, 0, 0, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetBatteryLow();
  unsigned char *actualMessage = tinySensor.Message;

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}

void Expect_sample_message_to_be_well_encoded() {

  // This sensor is set to channel 3 (1 << (3-1)) and has a rolling ID code of
  // 0x85. The first flag nibble (0xC) contains the battery low flag bit (0x4).
  // The temperature is -8.4 ºC since nibbles 11..8 are “8084”. The first “8”
  // indicates a negative temperature and the next three (“084”) represent the
  // decimal value 8.4. Humidity is 28% and the checksum byte is 0x53 and is
  // valid.

  /*
   * byte:      0    1    2    3    4     5      6      7      8      9
   * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
   *           1 D  2 0  4 8  5 C  4 8   0  8   8  2  8        44    C  0
   *           0 0  0 0  4 1  3 3  1 2   4  0   0  8  8  8   0  2    4 4   120
   *
   * nibbles 0..3    : sensor ID
   * nibble  4       : channel
   * nibbles 5..6    : rolling code
   * nibble  7       : battery state (0x4 is low battery)
   * nibbles 8..13   : temperature
   * nibble 12,14,15 : humidity (or-ed with temperature)
   * nibbles 16..19  : pressure
   */
  std::string expectedMessage = "0000485C480882844C0";

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));

  tinySensor.SetChannel(3);
  tinySensor.SetRollingCode(0x85);

  tinySensor.SetTemperature(-8.4);
  tinySensor.SetHumidity(28);
  tinySensor.SetPressure(900);

  tinySensor.SetBatteryLow();

  unsigned char *actualMessage = tinySensor.Message;

  std::string decodedMessage =
      MessageNibblesToString(actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES);
  std::cout << "Decoded: [" << decodedMessage << "]" << std::endl;

  //TEST_ASSERT_EQUAL_STRING(expectedMessage.c_str(), decodedMessage.c_str());
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_bitread_to_read_each_bit_separately);
  RUN_TEST(Expect_nibbles_to_be_sent_lsb_first);
  RUN_TEST(Expect_good_hardware_orders_for_zero);
  RUN_TEST(Expect_good_hardware_orders_for_one);
  RUN_TEST(Expect_messages_to_have_preamble_sync_and_postamble);
  RUN_TEST(Expect_right_negative_temperature_encoding);
  RUN_TEST(Expect_right_positive_temperature_encoding);
  RUN_TEST(Expect_right_humidity_encoding);
  RUN_TEST(Expect_right_pressure_encoding);
  RUN_TEST(Expect_right_channel_encoding);
  RUN_TEST(Expect_right_rolling_code_encoding);
  RUN_TEST(Expect_sample_message_to_be_well_encoded);
  RUN_TEST(Expect_right_low_battery_encoding);
  return UNITY_END();
}
#endif