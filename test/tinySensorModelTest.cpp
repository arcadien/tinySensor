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

class Hal {
public:
  virtual void GoLow() const = 0;
  virtual void GoHigh() const = 0;
  virtual void DelayPeriod() const = 0;
  virtual void DelayHalfPeriod() const = 0;
};

class TemperatureSensor {
public:
  virtual void SetTemperature(float temperature) = 0;
};

class HumiditySensor {
public:
  virtual void SetHumidity(float humidity) = 0;
};

class PressureSensor {
public:
  virtual void SetPressure(float pressure) = 0;
};
/*
 * Emulation of environment sensors using Oregon v3 protocol
 *
 *
 *  - Temperature:                     emulates THN132N (id 0xEC, 0x40)
 *  - Temperature, humidity:           emulates THGR122NX (id 0x1D, 0x20)
 *  - Temperature, humidity, pressure: emulates BTHR968 (id 0x5D, 0x60)
 */
class OregonV3 : public TemperatureSensor {

public:
  /*!
   * @param hal the component responsible in bit emission (delay and pin
   * states)
   */
  OregonV3(Hal const &hal) : _hal(hal) {}

  void SetTemperature(float temperature) override {}

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

  void DelayPeriod() const override { Orders.push_back('P'); }
  void DelayHalfPeriod() const override { Orders.push_back('D'); }

  void GoHigh() const override { Orders.push_back('H'); }
  void GoLow() const override { Orders.push_back('L'); }

  char *GetOrders() { return &Orders[0]; }

  mutable std::vector<char> Orders;
};

const char TestHal::EXPECTED_ORDERS_FOR_ZERO[ORDERS_COUNT_FOR_A_BYTE] = {
    'H', 'D', 'L', 'P', 'H', 'D'};
const char TestHal::EXPECTED_ORDERS_FOR_ONE[ORDERS_COUNT_FOR_A_BYTE] = {
    'L', 'D', 'H', 'P', 'L', 'D'};

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
  char expected[ordersCount] = {
      // Preamble : 24 times '1'
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
      // Sync : “0101” in the order of transmission
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'L', 'D', 'H', 'P', 'L', 'D', // 1
      // postamble : two nibbles a zero
      'H', 'D', 'L', 'P', 'H', 'D', // 0
      'H', 'D', 'L', 'P', 'H', 'D'  // 0
  };
  char *actualOrdersForTemperatureOnly;
  actualOrdersForTemperatureOnly = testHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected, actualOrdersForTemperatureOnly, ordersCount);
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_bitread_to_read_each_bit_separately);
  RUN_TEST(Expect_nibbles_to_be_sent_lsb_first);
  RUN_TEST(Expect_good_hardware_orders_for_zero);
  RUN_TEST(Expect_good_hardware_orders_for_one);
  RUN_TEST(Expect_messages_to_have_preamble_sync_and_postamble);

  return UNITY_END();
}
#endif