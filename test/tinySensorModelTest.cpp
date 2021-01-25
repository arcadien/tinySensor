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

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <unity.h>

namespace {

inline bool BitRead(uint8_t value, uint8_t bit) {
  return (((value) >> (bit)) & 0x01);
}

std::string MessageNibblesToString(char *message, char length) {

  return "1D20485C480882835";
}

/*
 * Interface definition for lower level hardware function
 *
 */
class Hal {
public:
  virtual void GoLow() const = 0;
  virtual void GoHigh() const = 0;
  virtual void DelayPeriod() const = 0;
  virtual void DelayHalfPeriod() const = 0;
};

/*
 * Emulation of environment sensors using Oregon v3 protocol
 *
 *
 *  - Temperature:                     emulates THN132N (id 0xEC, 0x40)
 *  - Temperature, humidity:           emulates THGR122NX (id 0x1D, 0x20)
 *  - Temperature, humidity, pressure: emulates BTHR968 (id 0x5D, 0x60)
 *
 * Payload data is stored in the `_message` property. Payload is made of
 * 'nibbles' which are 4 bits long.
 *
 * byte:      0    1    2    3    4     5      6      7      8      9
 * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
 *           1 D  2 0  4 8  5 C 4  8  0  8   8  2   8  3  5
 *
 * nibbles 0..3    : sensor ID
 * nibble  4       : channel
 * nibbles 5..6    : rolling code
 * nibble  7       : battery state (0x4 is low battery)
 * nibbles 8..13   : temperature
 * nibble 12,14,15 : humidity (or-ed with temperature)
 * nibbles 16..19  : pressure
 *
 *
 */
class OregonV3 {

public:
  // pressure is a value expressed in hPa, in interval ]850, 1100[
  // To allow storage in a char, PRESSURE_SCALING_VALUE is removed from actual
  // pressure value. It is added when decoding the message
  static const int PRESSURE_SCALING_VALUE = 856;

  // As rolling code is spread on two bytes, then its
  // max value is 15 tens and 15 units, because 15 aka 0xF is the max value on 4
  // bits
  static const char MAX_ROLLING_CODE_VALUE = 165;

  /*!
   * @param hal the component responsible in bit emission (delay and pin
   * states)
   */
  OregonV3(Hal const &hal) : _hal(hal) {

    for (uint index = 0; index < MESSAGE_SIZE_IN_BYTES; index++) {
      _message[index] = 0;
    }
  }

  void SetChannel(char channel) { _message[2] = 1 << 4 + (channel - 1); }

  /*
   * \param rollingCode must be less than MAX_ROLLING_CODE_VALUE
   */
  void SetRollingCode(char rollingCode) {
    char rollingCodeTens = (char)rollingCode / 10;
    _message[2] = rollingCodeTens;                            // nibble 4
    _message[3] |= (rollingCode - rollingCodeTens * 10) << 4; // nibble 5
  }

  void SetBatteryLow() { _message[3] |= 0xC; }

  void SetTemperature(float temperature) {

    if (temperature < 0) {
      _message[6] = 0x08;
      temperature *= -1;
    } else {
      _message[6] = 0x00;
    }

    // Determine decimal and float part

    // ex. for 27.4 degrees C
    int tempIntegerPart = (int)temperature; // 27

    // (int)(27/10) = 2
    int temperatureDozen = (int)(tempIntegerPart / 10);

    // 2.7 - 20 = (int)7.4=7
    int temperatureUnit = (int)round(
        (float)((float)tempIntegerPart / 10.0 - (float)temperatureDozen) *
        10.0);

    int temperatureDecimal = (int)round(
        (float)(temperature - (float)tempIntegerPart) // 27.4 - 27 = 0.4
        * 10                                          // 0.4 * 10 = 4
    );

    _message[4] |= (temperatureDecimal << 4);
    _message[5] = (temperatureDozen << 4);
    _message[5] |= temperatureUnit;
  }
  void SetHumidity(int humidity) {
    // humidity nibbles are spread on bytes 6 and 7
    // bit 6 also contains the negative temp flag (lsb)
    _message[7] = (humidity / 10);
    _message[6] |= (humidity - _message[7] * 10) << 4;
  }
  /*!
   *
   * Presure is set using hPa
   *
   * Values out of possible ones on earth (less that 850, more than 1099) are
   * ignored.
   */
  void SetPressure(int pressure) {
    if ((pressure > 850) && (pressure < 1100)) {
      _message[8] = pressure - PRESSURE_SCALING_VALUE;
      _message[9] = 0xC0;
    }
  }

  /*!
   * Emit message according to available data
   */
  void Send() {
    SendData(PREAMBLE, 3);
    SendLSB(SYNC);
    SendData(POSTAMBLE, 1);
  }

  void SendData(uint8_t *data, uint8_t size) {
    for (uint8_t i = 0; i < size; ++i) {
      SendLSB(data[i]);
      SendMSB(data[i]);
    }
  }

  void SendZero(void) {
    _hal.GoHigh();
    _hal.DelayHalfPeriod();
    _hal.GoLow();
    _hal.DelayPeriod();
    _hal.GoHigh();
    _hal.DelayHalfPeriod();
  }

  void SendOne(void) {
    _hal.GoLow();
    _hal.DelayHalfPeriod();
    _hal.GoHigh();
    _hal.DelayPeriod();
    _hal.GoLow();
    _hal.DelayHalfPeriod();
  }

  auto Message() { return _message; }
  static const char MESSAGE_SIZE_IN_BYTES = 10;

private:
  void SendMSB(const uint8_t data) {
    (BitRead(data, 4)) ? SendOne() : SendZero();
    (BitRead(data, 5)) ? SendOne() : SendZero();
    (BitRead(data, 6)) ? SendOne() : SendZero();
    (BitRead(data, 7)) ? SendOne() : SendZero();
  }

  void SendLSB(const uint8_t data) {
    (BitRead(data, 0)) ? SendOne() : SendZero();
    (BitRead(data, 1)) ? SendOne() : SendZero();
    (BitRead(data, 2)) ? SendOne() : SendZero();
    (BitRead(data, 3)) ? SendOne() : SendZero();
  }

  char _message[MESSAGE_SIZE_IN_BYTES];

  // v3, preamble is 6 nibbles of 1, 24 times '1'
  // see "Message Layout" section
  static uint8_t PREAMBLE[3];

  // one nibble as 0101, so 0b00001010
  // see "Message Layout" section
  static const uint8_t SYNC = {0b00001010};

  // v3, two nibbles with unknown format
  // see "Message Layout" section
  static uint8_t POSTAMBLE[1];

  Hal const &_hal;
};
uint8_t OregonV3::PREAMBLE[] = {0xFF, 0xFF, 0xFF};
uint8_t OregonV3::POSTAMBLE[] = {0x0};

class TestHal : public Hal {
public:
  static const int ORDERS_COUNT_FOR_A_BYTE = 6;
  const static char EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE];
  const static char EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE];
  // 6 chars per bit, 24 times "1"
  const static int PREAMBLE_BYTE_LENGTH = 24 * 6;
  const static char EXPECTED_PREAMBLE[PREAMBLE_BYTE_LENGTH];

  const static int POSTAMBLE_BYTE_LENGTH = 2 * 6;
  const static char EXPECTED_POSTAMBLE[POSTAMBLE_BYTE_LENGTH];

  const static int SYNC_BYTE_LENGTH = 4 * 6;
  const static char EXPECTED_SYNC[SYNC_BYTE_LENGTH];

  void DelayPeriod() const override { Orders.push_back('P'); }
  void DelayHalfPeriod() const override { Orders.push_back('D'); }

  inline void GoHigh() const override { Orders.push_back('H'); }
  inline void GoLow() const override { Orders.push_back('L'); }

  char *GetOrders() { return &Orders[0]; }

  mutable std::vector<char> Orders;
};

const char TestHal::EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE] = {
    'H', 'D', 'L', 'P', 'H', 'D'};

const char TestHal::EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE] = {
    'L', 'D', 'H', 'P', 'L', 'D'};

const char TestHal::EXPECTED_POSTAMBLE[TestHal::POSTAMBLE_BYTE_LENGTH] = {
    // postamble : two nibbles at zero
    'H', 'D', 'L', 'P', 'H', 'D', // 0
    'H', 'D', 'L', 'P', 'H', 'D'  // 0
};

const char TestHal::EXPECTED_SYNC[TestHal::SYNC_BYTE_LENGTH] = {
    'H', 'D', 'L', 'P', 'H', 'D', // 0
    'L', 'D', 'H', 'P', 'L', 'D', // 1
    'H', 'D', 'L', 'P', 'H', 'D', // 0
    'L', 'D', 'H', 'P', 'L', 'D'  // 1
};

const char TestHal::EXPECTED_PREAMBLE[TestHal::PREAMBLE_BYTE_LENGTH] = {
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

  char expectedOrders[8 * TestHal::ORDERS_COUNT_FOR_A_BYTE] = {
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
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expectedOrders, actualOrdersForOneByte,
                               8 * TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_good_hardware_orders_for_zero() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SendZero();
  auto actualOrdersForZero = testHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(TestHal::EXPECTED_ORDERS_FOR_ZERO,
                               actualOrdersForZero,
                               TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_good_hardware_orders_for_one() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SendOne();
  char *actualOrdersForOne = testHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(TestHal::EXPECTED_ORDERS_FOR_ONE,
                               actualOrdersForOne,
                               TestHal::ORDERS_COUNT_FOR_A_BYTE);
}

void Expect_messages_to_have_preamble_sync_and_postamble() {
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));

  tinySensor.Send();

  auto ordersCount = (6 * 4 + 4 + 2) * TestHal::ORDERS_COUNT_FOR_A_BYTE;

  char *expected = new char[ordersCount + 1];

  expected[ordersCount + 1] = '\0';

  memcpy(expected, TestHal::EXPECTED_PREAMBLE,
         TestHal::PREAMBLE_BYTE_LENGTH * sizeof(char));

  memcpy(expected + TestHal::PREAMBLE_BYTE_LENGTH, TestHal::EXPECTED_SYNC,
         TestHal::SYNC_BYTE_LENGTH * sizeof(char));

  memcpy(expected + TestHal::PREAMBLE_BYTE_LENGTH + TestHal::SYNC_BYTE_LENGTH,
         TestHal::EXPECTED_POSTAMBLE,
         TestHal::POSTAMBLE_BYTE_LENGTH * sizeof(char));

  char *actualOrdersForTemperatureOnly;
  actualOrdersForTemperatureOnly = testHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actualOrdersForTemperatureOnly,
                               ordersCount);

  delete expected;
}

void Expect_right_positive_temperature_encoding()

{
  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetTemperature(27.5);

  char byte4 = 5 << 4;
  char byte5 = 2 << 4 | 7;
  char byte6 = 0x0; // positive temp
  char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

  char *actualMessage = tinySensor.Message();

  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with positive temp. with dozen");
}

void Expect_right_pressure_encoding() {

  int actualPressureInHPa = 900;
  char byte8 = actualPressureInHPa - OregonV3::PRESSURE_SCALING_VALUE;
  char byte9 = 0xC0;
  char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, 0, 0, 0, 0, byte8, byte9};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetPressure(actualPressureInHPa);
  char *actualMessage = tinySensor.Message();

  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(expected, actualMessage,
                                       OregonV3::MESSAGE_SIZE_IN_BYTES,
                                       "Failed with possible pressure value");
}

void Expect_right_negative_temperature_encoding() {
  {

    char byte4 = 5 << 4;
    char byte5 = 2 << 4 | 7;
    char byte6 = 0x08; // negative temp
    char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
        0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));
    tinySensor.SetTemperature(-27.5);
    char *actualMessage = tinySensor.Message();

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp. with dozen");
  }
  {

    char byte4 = 4 << 4;
    char byte5 = 8;
    char byte6 = 0x08; // negative temp
    char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
        0, 0, 0, 0, byte4, byte5, byte6, 0, 0, 0};

    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));
    tinySensor.SetTemperature(-8.4);
    char *actualMessage = tinySensor.Message();

    TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
        expected, actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES,
        "Failed with negative temp without dozen");
  }
}

void Expect_right_humidity_encoding() {

  char byte6 = 2 << 4;
  char byte7 = 5;
  char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, 0, 0, 0, 0, byte6, byte7, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetHumidity(52);
  char *actualMessage = tinySensor.Message();

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}
void Expect_right_channel_encoding() {

  struct Given {
    const char *label;
    char channel;
    char *expected;
    Given(const char *label, char channel, char *expected)
        : channel(channel), expected(std::move(expected)) {}
  };
  std::vector<Given> givens;

  // implemented sensors use the coding 1 << (ch –1), where ch is 1, 2 or 3.

  char channelByte = 0b00010000;
  givens.push_back(Given("channel 1", 1,
                         new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  channelByte = 0b00100000;
  givens.push_back(Given("channel 2", 2,
                         new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  channelByte = 0b01000000;
  givens.push_back(Given("channel 3", 3,
                         new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
                             0, 0, channelByte, 0, 0, 0, 0, 0, 0, 0}));

  for (auto const given : givens) {
    TestHal testHal;
    OregonV3 tinySensor(std::move(testHal));

    tinySensor.SetChannel(given.channel);
    char *actualMessage = tinySensor.Message();

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

  char *expected = new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{
      0, 0, (char)8, (char)(5 << 4), 0, 0, 0, 0, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetRollingCode(85);
  char *actualMessage = tinySensor.Message();

  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actualMessage,
                               OregonV3::MESSAGE_SIZE_IN_BYTES);
}

void Expect_right_low_battery_encoding() {

  //  byte:      0    1    2    3    4     5      6      7      8      9
  // nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19]
  //                            C
  // nibbles 7    : low batt

  char *expected =
      new char[OregonV3::MESSAGE_SIZE_IN_BYTES]{0, 0, 0, 0xC, 0, 0, 0, 0, 0, 0};

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));
  tinySensor.SetBatteryLow();
  char *actualMessage = tinySensor.Message();

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

  // message byte index          0   1  2  3  4  5  6  7  8
  // std::string expectedMessage = "1D 20 48 5C 48 08 .8 28 35";
  std::string expectedMessage = "1D20485C480882835";

  TestHal testHal;
  OregonV3 tinySensor(std::move(testHal));

  tinySensor.SetChannel(3);
  tinySensor.SetRollingCode(0x85);

  tinySensor.SetTemperature(-8.4);
  tinySensor.SetHumidity(28);
  tinySensor.SetBatteryLow();

  char *actualMessage = tinySensor.Message();

  std::string decodedMessage =
      MessageNibblesToString(actualMessage, OregonV3::MESSAGE_SIZE_IN_BYTES);
  std::cout << decodedMessage << std::endl;

  TEST_ASSERT_EQUAL_STRING(expectedMessage.c_str(), decodedMessage.c_str());
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