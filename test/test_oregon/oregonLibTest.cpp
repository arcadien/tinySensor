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
#if defined(NATIVE)

#include <cstdarg>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <Oregon_v3.h>
#include <TestHal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

static const unsigned char
    EXPECTED_ORDERS_FOR_ZERO[OregonV3::ORDERS_COUNT_FOR_A_BIT] = {
        'H', 'D', 'L', 'P', 'H', 'D'};

static const unsigned char
    EXPECTED_ORDERS_FOR_ONE[OregonV3::ORDERS_COUNT_FOR_A_BIT] = {'L', 'D', 'H',
                                                                 'P', 'L', 'D'};

static const unsigned char
    EXPECTED_POSTAMBLE[2 * OregonV3::ORDERS_COUNT_FOR_A_BIT] = {
        // postamble : two nibbles at zero
        'H', 'D', 'L', 'P', 'H', 'D', // 0
        'H', 'D', 'L', 'P', 'H', 'D'  // 0
};

static const unsigned char EXPECTED_PREAMBLE[OregonV3::PREAMBLE_BYTE_LENGTH] = {
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

static std::string MessageNibblesToString(const unsigned char *message,
                                          unsigned char length) {
  std::stringstream ss;
  for (int index = 0; index < length; index++) {
    ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
       << ((int)(message[index]));
  }
  return ss.str();
}

/*
 * This testsuite is based on the document
 * "434MHz RF Protocol Descriptions for Wireless Weather Sensors,
 *  Oregon Scientific, AcuRite, Ambient Weather
 *  October 2015"
 *  A copy of the document should be provided along the implementation
 */
void setUp(void) { TestHal.ClearOrders(); }
void tearDown(void) {}

// ---------------- Test helpers tests
void Expect_bitread_to_read_each_bit_separately() {

  std::string message = "";

  uint8_t value = 0;

  for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
    message = "Failed with value==0 for bit " + std::to_string(bitIndex);
    TEST_ASSERT_FALSE_MESSAGE(OregonV3::BitRead(value, bitIndex),
                              message.c_str());
  }

  value = 1; // 0b00000001
  for (uint8_t positionOfOne = 0; positionOfOne < 8; ++positionOfOne) {

    for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
      message = "Failed when 1 is at index " + std::to_string(positionOfOne) +
                ", for bit " + std::to_string(bitIndex);
      if (positionOfOne == bitIndex) {
        TEST_ASSERT_TRUE_MESSAGE(OregonV3::BitRead(value, bitIndex),
                                 message.c_str());
      } else {
        TEST_ASSERT_FALSE_MESSAGE(OregonV3::BitRead(value, bitIndex),
                                  message.c_str());
      }
    }
    value <<= 1; // 0b00000010
  }
}

// ----------------- Oregon tests

void Expect_good_hardware_orders_for_zero() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SendZero();
  auto actualOrdersForZero = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS_FOR_ZERO, actualOrdersForZero,
                               OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
void Expect_good_hardware_orders_for_one() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SendOne();
  unsigned char *actualOrdersForOne = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS_FOR_ONE, actualOrdersForOne,
                               OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
void Expect_nibbles_to_be_sent_lsb_first() {
  OregonV3 oregonv3(&TestHal);

  // Send one byte
  uint8_t value = 0b00010010;
  oregonv3.SendData(&value, 1);

  unsigned char expectedOrders[8 * OregonV3::ORDERS_COUNT_FOR_A_BIT] = {
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D'  // 0
  };
  unsigned char *actualOrdersForOneByte = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expectedOrders, actualOrdersForOneByte,
                               8 * OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
void Expect_messages_to_have_preamble_and_postamble() {

  // The specification document says that the SYNC must be sent.
  // With the RFLINK decoder, which is the reference for this library,
  // the SYNC is not needed.
  OregonV3 oregonv3(&TestHal);

  oregonv3.Send();

  auto ordersCount = (6 * 4 + /* 4 +*/ 1) * OregonV3::ORDERS_COUNT_FOR_A_BIT;

  unsigned char expected[ordersCount];

  memcpy(expected, EXPECTED_PREAMBLE,
         OregonV3::PREAMBLE_BYTE_LENGTH * sizeof(unsigned char));

  memcpy(expected + OregonV3::PREAMBLE_BYTE_LENGTH, EXPECTED_POSTAMBLE,
         OregonV3::POSTAMBLE_BYTE_LENGTH * sizeof(unsigned char));

  unsigned char *actualEmptyMessageOrders;
  actualEmptyMessageOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actualEmptyMessageOrders, ordersCount);
}
void Expect_right_positive_temperature_encoding()

{
  OregonV3 oregonv3(&TestHal);
  oregonv3.SetTemperature(27.5);

  // clang-format off
  unsigned char byte4 = 5 << 4; // 0x50
                byte4 |= 7;     // 0x57
  unsigned char byte5 = 2 << 4; // 0x20
  // clang-format on

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, byte4, byte5, 0, 0, 0, 0, 0};

  const unsigned char *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with positive temp. with dozen");
}
void Expect_right_negative_temperature_encoding() {
  {

    // clang-format off
    unsigned char byte4 = 5 << 4; // 0x50
                  byte4 |= 7;     // 0x57
    unsigned char byte5 = 2 << 4; // 0x20
                  byte5 |= 0x08;  // 0x08 (negative temp)
    // clang-format on

    unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
        0, 0, 0, 0, byte4, byte5, 0, 0, 0, 0, 0};

    OregonV3 oregonv3(&TestHal);
    oregonv3.SetTemperature(-27.5);
    const unsigned char *actualMessage = oregonv3.GetMessage();

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp. with dozen");
  }
  {

    // clang-format off
    unsigned char byte4 = 4 << 4; // 0x40
                  byte4 |= 8;     // 0x48
    unsigned char byte5 = 0x00;   // 0x00
                  byte5 |= 0x08;   // 0x08 (negative temp)
    // clang-format on

    unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
        0, 0, 0, 0, byte4, byte5, 0, 0, 0, 0, 0};

    OregonV3 oregonv3(&TestHal);
    oregonv3.SetTemperature(-8.4);
    const unsigned char *actualMessage = oregonv3.GetMessage();

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp without dozen");
  }
}
void Expect_right_pressure_encoding() {

  const int actualPressureInHPa = 1200;
  const int pressureScalingValue = 795; // //OregonV3::PRESSURE_SCALING_VALUE;
  uint16_t pressureValue =
      (uint16_t)(actualPressureInHPa - pressureScalingValue); // 405, 0x195

  uint8_t byte7 = 0x05;
  uint8_t byte8 = 0x91;
  uint8_t byte9 = 0xC0; // sunny

  uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, 0, 0, 0, byte7, byte8, byte9, 0};

  OregonV3 oregonv3(&TestHal);
  oregonv3.SetPressure(actualPressureInHPa);
  uint8_t actualMessage[OregonV3::MESSAGE_SIZE_IN_BYTES];
  memcpy(&actualMessage, oregonv3.GetMessage(),
         OregonV3::MESSAGE_SIZE_IN_BYTES);

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with possible pressure value");
}
void Expect_right_humidity_encoding() {

  uint8_t byte6 = 0x25;

  uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{0,     0, 0, 0, 0, 0,
                                                    byte6, 0, 0, 0, 0};

  OregonV3 oregonv3(&TestHal);
  oregonv3.SetHumidity(52);
  const uint8_t *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actualMessage,
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
  unsigned char msg1[OregonV3::MESSAGE_SIZE_IN_BYTES]{0, 0, channelByte, 0, 0,
                                                      0, 0, 0,           0, 0};
  givens.push_back(Given("channel 1", 1, msg1));

  channelByte = 0b00100000;
  unsigned char msg2[OregonV3::MESSAGE_SIZE_IN_BYTES]{0, 0, channelByte, 0, 0,
                                                      0, 0, 0,           0, 0};
  givens.push_back(Given("channel 2", 2, msg2));

  channelByte = 0b01000000;
  unsigned char msg3[OregonV3::MESSAGE_SIZE_IN_BYTES]{0, 0, channelByte, 0, 0,
                                                      0, 0, 0,           0, 0};
  givens.push_back(Given("channel 3", 3, msg3));

  for (auto const given : givens) {

    OregonV3 oregonv3(&TestHal);

    oregonv3.SetChannel(given.channel);
    const unsigned char *actualMessage = oregonv3.GetMessage();

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

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{0, 0, 0x08, 0x50, 0,
                                                          0, 0, 0,    0,    0};

  OregonV3 oregonv3(&TestHal);
  oregonv3.SetRollingCode(0x85);
  const unsigned char *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}
void Expect_right_battery_ok_encoding() {

  //  byte:      0    1    2    3    4     5      6      7      8      9
  // nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
  //                            4
  // nibble 7    : low batt

  OregonV3 oregonv3(&TestHal);
  oregonv3.SetBatteryLow(false);

  const unsigned char *actualMessage = oregonv3.GetMessage();

  uint8_t thirdByte = actualMessage[3];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0b00000000, (thirdByte & 0x00000100),
                           "Testing battery ok flag");
}
void Expect_right_low_battery_encoding() {

  //  byte:      0    1    2    3    4     5      6      7      8      9
  // nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
  //                            4
  // nibble 7    : low batt

  OregonV3 oregonv3(&TestHal);
  oregonv3.SetBatteryLow(true);
  const unsigned char *actualMessage = oregonv3.GetMessage();

  uint8_t thirdByte = actualMessage[3];

  TEST_ASSERT_EQUAL_HEX8_MESSAGE(0x04, (thirdByte & 0x04),
                           "Testing battery low flag");

}
void Expect_temperature_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetTemperature(-8.4);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x1) == 0x1);
}
void Expect_humidity_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetHumidity(84);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x2) == 0x2);
}
void Expect_pressure_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetPressure(1000);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x4) == 0x4);
}
void Expect_all_values_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetTemperature(12.2);
  oregonv3.SetHumidity(40);
  oregonv3.SetPressure(1000);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x7) == 0x7);
}
void Expect_temperature_set_to_change_sensor_type() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SetTemperature(10.f);

  static const uint16_t THN132 = 0xEC40;

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      (THN132 >> 8), (THN132 & 0xFF), 0, 0, 0, 0, 0, 0, 0, 0, 0};

  oregonv3.FinalizeMessage();
  const unsigned char *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(
      expected, actualMessage, 2,
      "Expect THN132 (0xEC40) with temperature only");
}
void Expect_humidity_set_to_change_sensor_type() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SetHumidity(10.f);
  static const uint16_t THGN123N = 0x1D20;

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      (THGN123N >> 8), (THGN123N & 0xFF), 0, 0, 0, 0, 0, 0, 0, 0, 0};

  oregonv3.FinalizeMessage();
  const unsigned char *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(
      expected, actualMessage, 2, "Expect THGN123N (0x1D20) with humidity");
}
void Expect_pressure_set_to_change_sensor_type() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SetPressure(900);
  static const uint16_t BTHR918 = 0x5A5D;

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      (BTHR918 >> 8), (BTHR918 & 0xFF), 0, 0, 0, 0, 0, 0, 0, 0, 0};

  oregonv3.FinalizeMessage();
  const unsigned char *actualMessage = oregonv3.GetMessage();

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, actualMessage, 2,
                                       "Expect BTHR918 (0x5A5D) with pressure");
}
void Expect_sample_message_to_be_well_encoded() {

  // nibbles 0..3 are sensor ID. All measurement, shall be 0x5A, 0x5D
  // nibble 4 is channel 3 (1 << (3-1))
  // nibbles 5 and 6 are rolling code, 0x85
  // nibble 7 contains the battery low flag bit (0x4)
  // nibbles 8..11 are temperature. 0x48, 0x08 (-8.4 °c)
  // nibbles 12..13 are humidity, 28%
  // nibbles 16..17 are pressure, 900 hpa. (900-795 = 105)
  // nibbles 18..19 are the checksum byte, 0x53

  /*
   * byte:      0    1    2    3    4     5      6      7      8      9
   * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
   *           5 A  5 D  4 8  5 4  4 8   0  8   8  2  8        105   5 3
   *
   * nibbles 0..3    : sensor ID
   * nibble  4       : channel
   * nibbles 5..6    : rolling code
   * nibble  7       : battery state (0x4 is low battery)
   * nibbles 8..13   : temperature
   * nibble 12,14,15 : humidity (or-ed with temperature)
   * nibbles 16..19  : pressure
   */
  std::string expectedMessage = "5A5D423448088209603054";

  OregonV3 oregonv3(&TestHal);

  oregonv3.SetChannel(3);
  oregonv3.SetRollingCode(0x23);

  oregonv3.SetTemperature(-8.4);
  oregonv3.SetHumidity(28);
  oregonv3.SetPressure(900);

  oregonv3.SetBatteryLow(true);

  oregonv3.FinalizeMessage();

  const unsigned char *actualMessage = oregonv3.GetMessage();

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0x5A, 0x5D, 0x42, 0x34, 0x48, 0x08, 0x82, 0x09, 0x60, 0x30, 0x54};
  //   0     1      2     3     4    5     6     7     8     9     10

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(
      expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
      "Expect right encoding for full message");

  std::string decodedMessage =
      MessageNibblesToString(actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES);
  std::cout << "Decoded: [" << decodedMessage << "]" << std::endl;

  TEST_ASSERT_EQUAL_STRING(expectedMessage.c_str(), decodedMessage.c_str());
}
void Expect_implementation_follows_samples_1() {

  /*

  This sensor is set to channel 3 (1 << (3-1)) and has a rolling ID code of
  0x85. The first flag nibble contains the battery low flag bit (0x4). The
  temperature is -8.4 ºC since nibbles 11..8 are “8084”. The first “8” indicates
  a negative temperature and the next three (“084”) represent the decimal value
  8.4. Humidity is 28% and the checksum byte is 0x53 and is valid.

  NOTE: as byte 7 has been set to 0, checksum changes to 0x93

  */

  std::string expectedMessage = "1D20485448088200390000";

  OregonV3 oregonv3(&TestHal);

  oregonv3.SetChannel(3);
  oregonv3.SetRollingCode(0x85);
  oregonv3.SetBatteryLow(true);
  oregonv3.SetTemperature(-8.4);
  oregonv3.SetHumidity(28);

  oregonv3.FinalizeMessage();

  const unsigned char *actualMessage = oregonv3.GetMessage();

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0x1D, 0x20, 0x48, 0x54, 0x48, 0x08, 0x82, 0x00, 0x39};

  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(
      expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
      "Expect right encoding for full message");

  std::string decodedMessage =
      MessageNibblesToString(actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES);
  std::cout << "Decoded: [" << decodedMessage << "]" << std::endl;

  TEST_ASSERT_EQUAL_STRING(expectedMessage.c_str(), decodedMessage.c_str());
}
void Expect_implementation_follows_samples_2() {

  /*
  This sensor is set to channel 1 (1 << (1-1)) and has a rolling ID code of
  0x6B, and the battery low bit is not set in the flag nibble. Temperature
  and humidity are 19.0 ºC and 37%. Checksum is 0x41 and is valid.

  NOTE: as byte 7 has been set to 0, checksum changes to 0xD2

  */

  std::string expectedMessage = "1D2016B0091073002C0000";

  OregonV3 oregonv3(&TestHal);

  oregonv3.SetChannel(1);
  oregonv3.SetRollingCode(0x6B);

  oregonv3.SetTemperature(19);
  oregonv3.SetHumidity(37);

  oregonv3.FinalizeMessage();

  const unsigned char *actualMessage = oregonv3.GetMessage();

  unsigned char expected[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0x1D, 0x20, 0x16, 0xB0, 0x09, 0x10, 0x73, 0x00, 0x2C};
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(
      expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
      "Expect right encoding for full message");

  std::string decodedMessage =
      MessageNibblesToString(actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES);
  std::cout << "Decoded: [" << decodedMessage << "]" << std::endl;

  TEST_ASSERT_EQUAL_STRING(expectedMessage.c_str(), decodedMessage.c_str());
}

int main(int, char **) {
  UNITY_BEGIN();

  RUN_TEST(Expect_good_hardware_orders_for_zero);
  RUN_TEST(Expect_good_hardware_orders_for_one);
  RUN_TEST(Expect_bitread_to_read_each_bit_separately);
  RUN_TEST(Expect_nibbles_to_be_sent_lsb_first);
  RUN_TEST(Expect_messages_to_have_preamble_and_postamble);

  RUN_TEST(Expect_right_negative_temperature_encoding);
  RUN_TEST(Expect_right_positive_temperature_encoding);
  RUN_TEST(Expect_right_humidity_encoding);
  RUN_TEST(Expect_right_pressure_encoding);
  RUN_TEST(Expect_right_channel_encoding);
  RUN_TEST(Expect_right_rolling_code_encoding);
  RUN_TEST(Expect_right_battery_ok_encoding);
  RUN_TEST(Expect_right_low_battery_encoding);

  RUN_TEST(Expect_temperature_set_to_change_message_status);
  RUN_TEST(Expect_humidity_set_to_change_message_status);
  RUN_TEST(Expect_pressure_set_to_change_message_status);
  RUN_TEST(Expect_all_values_set_to_change_message_status);

  RUN_TEST(Expect_temperature_set_to_change_sensor_type);
  RUN_TEST(Expect_humidity_set_to_change_sensor_type);
  RUN_TEST(Expect_pressure_set_to_change_sensor_type);

  RUN_TEST(Expect_sample_message_to_be_well_encoded);
  RUN_TEST(Expect_implementation_follows_samples_1);
  RUN_TEST(Expect_implementation_follows_samples_2);

  return UNITY_END();
}
#endif