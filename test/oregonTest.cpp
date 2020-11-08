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

#include <string>
#include <unity.h>
#include <utility>
#include <vector>

namespace {
namespace Oregon {

inline bool BitRead(uint8_t value, uint8_t bit) {
  return (((value) >> (bit)) & 0x01);
}
/*
inline void sendZero(void) {
  txPinHigh();
  _delay_us(TIME);
  txPinLow();
  _delay_us(TWOTIME);
  txPinHigh();
  _delay_us(TIME);
}

inline void sendOne(void) {
  txPinLow();
  _delay_us(TIME);
  txPinHigh();
  _delay_us(TWOTIME);
  txPinLow();
  _delay_us(TIME);
}


*/
class Hal {
public:
  virtual void DelayUs(uint16_t duration) const = 0;
  virtual void TxPinHigh() const = 0;
  virtual void TxPinLow() const = 0;
};
/*
 * This class propose implementation of Oregon v3 messages
 * for environment sensors able to report temperature, humidity and pressure.
 * Given the data exposed, specific device will be emulated.
 * - Temperature only : THN132N sensor, type ID 0xEC40
 */
class v3 {

public:
  /*!
   *
   * @param hal the component responsible in nibble emission (delay and pin
   * states)
   * @param period clock period for manchester encoding
   */
  v3(Hal const &hal, uint16_t period)
      : _hal(hal), _period(period), _halfPeriod(period / 2) {}

  void SendData(uint8_t *data, uint8_t size) {
    for (uint8_t i = 0; i < size; ++i) {
      SendQuarterLSB(data[i]);
      SendQuarterMSB(data[i]);
    }
  }

private:
  void SendQuarterMSB(const uint8_t data) {
    (BitRead(data, 4)) ? SendOne() : SendZero();
    (BitRead(data, 5)) ? SendOne() : SendZero();
    (BitRead(data, 6)) ? SendOne() : SendZero();
    (BitRead(data, 7)) ? SendOne() : SendZero();
  }

  void SendQuarterLSB(const uint8_t data) {
    (BitRead(data, 0)) ? SendOne() : SendZero();
    (BitRead(data, 1)) ? SendOne() : SendZero();
    (BitRead(data, 2)) ? SendOne() : SendZero();
    (BitRead(data, 3)) ? SendOne() : SendZero();
  }

  /*!
   * off to on at the middle of a clock period
   */
  void SendZero(void) {
    _hal.TxPinHigh();
    _hal.DelayUs(_halfPeriod);
    _hal.TxPinLow();
    _hal.DelayUs(_period);
    _hal.TxPinHigh();
    _hal.DelayUs(_halfPeriod);
  }

  /*!
   * on to to off at the middle of a clock period
   */
  void SendOne(void) {
    _hal.TxPinLow();
    _hal.DelayUs(_halfPeriod);
    _hal.TxPinHigh();
    _hal.DelayUs(_period);
    _hal.TxPinLow();
    _hal.DelayUs(_halfPeriod);
  }
  uint16_t const _period;
  uint16_t const _halfPeriod;
  Hal const &_hal;
};
} // namespace Oregon

class TestHal : public Oregon::Hal {
public:
  void DelayUs(uint16_t duration) const override { Orders.push_back('D'); }
  void TxPinHigh() const override { Orders.push_back('H'); }
  void TxPinLow() const override { Orders.push_back('L'); }

  mutable std::vector<unsigned char> Orders;
};

const static std::vector<unsigned char> expectedOrderSequenceForOne({'H', 'L',
                                                                     'H'});
const static std::vector<unsigned char> expectedOrderSequenceForZero();

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
void Expect_bitread_to_read_each_nibble_separately() {

  std::string message = "";

  uint8_t value = 0;

  for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
    message = "Failed with value==0 for bit " + std::to_string(bitIndex);
    TEST_ASSERT_FALSE_MESSAGE(Oregon::BitRead(value, bitIndex),
                              message.c_str());
  }

  value = 1; // 0b00000001
  for (uint8_t positionOfOne = 0; positionOfOne < 8; ++positionOfOne) {

    for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
      message = "Failed when 1 is at index " + std::to_string(positionOfOne) +
                ", for bit " + std::to_string(bitIndex);
      if (positionOfOne == bitIndex) {
        TEST_ASSERT_TRUE_MESSAGE(Oregon::BitRead(value, bitIndex),
                                 message.c_str());
      } else {
        TEST_ASSERT_FALSE_MESSAGE(Oregon::BitRead(value, bitIndex),
                                  message.c_str());
      }
    }
    value <<= 1; // 0b00000010
  }
}

void Expect_messages_to_be_manchester_encoded() {
  uint8_t value = 0b00000000;

  TestHal testHal;
  Oregon::v3 v3(std::move(testHal), 512);

  // Send one byte
  v3.SendData(&value, 1);

  // Sending one nibble is 3 pin state changes
  // and 3 delays, to have the pin change at
  // middle of Manchester period. That is,
  // 6 orders are needed to send a nibble.
  uint16_t ordersCountNeededForANibble = 6;
  uint16_t actualSize = static_cast<uint16_t>(testHal.Orders.size());
  TEST_ASSERT_EQUAL_INT16_MESSAGE(ordersCountNeededForANibble * 8, actualSize,
                                  "Sending a byte must lead to 6x8 orders");

  // std::vector<unsigned char> expectedOne({H,L,H});
    TEST_FAIL_MESSAGE("not implemented");

}

void Expect_good_hardware_orders_for_zero() {
  TEST_FAIL_MESSAGE("not implemented");
}

void Expect_good_hardware_orders_for_one() {
    TEST_FAIL_MESSAGE("not implemented");
}

void Expect_byte_to_be_sent_lsb_first() {

  uint8_t value = 0b00000001;

  TestHal testHal;
  Oregon::v3 v3(std::move(testHal), 512);

  // Send one byte
  v3.SendData(&value, 1);

  // The byte content must be sent LSB first, so the first nibble
  // to be sent is a one, then the second is a zero.
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_bitread_to_read_each_nibble_separately);
  RUN_TEST(Expect_messages_to_be_manchester_encoded);
  RUN_TEST(Expect_byte_to_be_sent_lsb_first);
  RUN_TEST(Expect_good_hardware_orders_for_zero);
  RUN_TEST(Expect_good_hardware_orders_for_one);

  return UNITY_END();
}
