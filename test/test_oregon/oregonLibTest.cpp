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
        'H', 'D', 'L', 'P', 'H', 'D',  // 0
        'H', 'D', 'L', 'P', 'H', 'D'   // 0
};

static const unsigned char EXPECTED_PREAMBLE[OregonV3::PREAMBLE_BYTE_LENGTH] = {
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
    'L', 'D', 'H', 'P', 'L', 'D',  // 1
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
void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

// ---------------- Test helpers tests
// @req FUNC-OREGON-003
void Expect_bitread_to_read_each_bit_separately() {
  std::string message = "";

  uint8_t value = 0;

  for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
    message = "Failed with value==0 for bit " + std::to_string(bitIndex);
    TEST_ASSERT_FALSE_MESSAGE(OregonV3::BitRead(value, bitIndex),
                              message.c_str());
  }

  value = 1;  // 0b00000001
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
    value <<= 1;  // 0b00000010
  }
}

// ----------------- Oregon tests

// @req FUNC-OREGON-001
void Expect_good_hardware_orders_for_zero() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SendZero();
  auto actualOrdersForZero = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS_FOR_ZERO, actualOrdersForZero,
                               OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
// @req FUNC-OREGON-002
void Expect_good_hardware_orders_for_one() {
  OregonV3 oregonv3(&TestHal);
  oregonv3.SendOne();
  unsigned char *actualOrdersForOne = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS_FOR_ONE, actualOrdersForOne,
                               OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
// @req FUNC-OREGON-003
void Expect_nibbles_to_be_sent_lsb_first() {
  OregonV3 oregonv3(&TestHal);

  // Send one byte
  uint8_t value = 0b00010010;
  oregonv3.SendData(&value, 1);

  unsigned char expectedOrders[8 * OregonV3::ORDERS_COUNT_FOR_A_BIT] = {
      'H', 'D', 'L', 'P', 'H', 'D',  // 0
      'L', 'D', 'H', 'P', 'L', 'D',  // 1
      'H', 'D', 'L', 'P', 'H', 'D',  // 0
      'H', 'D', 'L', 'P', 'H', 'D',  // 0
      'L', 'D', 'H', 'P', 'L', 'D',  // 1
      'H', 'D', 'L', 'P', 'H', 'D',  // 0
      'H', 'D', 'L', 'P', 'H', 'D',  // 0
      'H', 'D', 'L', 'P', 'H', 'D'   // 0
  };
  unsigned char *actualOrdersForOneByte = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expectedOrders, actualOrdersForOneByte,
                               8 * OregonV3::ORDERS_COUNT_FOR_A_BIT);
}
// @req FUNC-OREGON-004
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
/*
 * Oregon v2.1 layout as decoded by RFLink Plugins/Plugin_048.c (couin3),
 * nibble 2i being the low nibble of byte i (nibble 0 is the sync nibble 0xA):
 *
 * byte:      0    1    2    3    4     5      6      7      8      9    10
 * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19][20 21]
 *
 * byte 0-1 : device ID, sync nibble included (EA4C, 1A2D, 5A5D)
 * byte 2   : channel in the high nibble
 * byte 3   : rolling code
 * byte 4   : flags (bit 2 battery low) | temperature tenths << 4
 * byte 5   : temperature units | tens << 4
 * byte 6   : sign (bit 3) | humidity units << 4
 * byte 7   : humidity tens (comfort nibble 0)
 * byte 8   : pressure hPa - 795, clamped 0-255 (BTHR918)
 * byte 9   : weather prediction (BTHR918)
 * checksum : nibbles 13-14 (THN132N), byte 8 (THGR228N), byte 10 (BTHR918)
 */
struct TemperatureCase {
  const char *id;
  float temperature;
  uint8_t byte4, byte5, byte6;
};

static const TemperatureCase positiveTemperatureCases[] = {
    {"27.5", 27.5f, 0x50, 0x27, 0x00},
    {"21.5", 21.5f, 0x50, 0x21, 0x00},
    {"8.4-no-tens", 8.4f, 0x40, 0x08, 0x00},
    {"zero", 0.0f, 0x00, 0x00, 0x00},
};

// @req FUNC-OREGON-005
void Expect_right_positive_temperature_encoding() {
  for (const TemperatureCase &c : positiveTemperatureCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    expected[4] = c.byte4;
    expected[5] = c.byte5;
    expected[6] = c.byte6;

    // When
    oregonv3.SetTemperature(c.temperature);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}

static const TemperatureCase negativeTemperatureCases[] = {
    {"-27.5", -27.5f, 0x50, 0x27, 0x08},
    {"-8.4-no-tens", -8.4f, 0x40, 0x08, 0x08},
    {"-10.2", -10.2f, 0x20, 0x10, 0x08},
    {"-0.5", -0.5f, 0x50, 0x00, 0x08},
};

// @req FUNC-OREGON-006
void Expect_right_negative_temperature_encoding() {
  for (const TemperatureCase &c : negativeTemperatureCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    expected[4] = c.byte4;
    expected[5] = c.byte5;
    expected[6] = c.byte6;

    // When
    oregonv3.SetTemperature(c.temperature);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}

struct SignAndHumidityCase {
  const char *id;
  bool humidityFirst;
};

static const SignAndHumidityCase signAndHumidityCases[] = {
    {"temperature-then-humidity", false},
    {"humidity-then-temperature", true},
};

// @req FUNC-OREGON-006
void Expect_negative_sign_to_share_byte_6_with_humidity_units() {
  for (const SignAndHumidityCase &c : signAndHumidityCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    const uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {
        0, 0, 0, 0, 0x20, 0x10, 0x78, 0x06, 0, 0, 0};

    // When
    if (c.humidityFirst) {
      oregonv3.SetHumidity(67);
      oregonv3.SetTemperature(-10.2f);
    } else {
      oregonv3.SetTemperature(-10.2f);
      oregonv3.SetHumidity(67);
    }

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}

struct HumidityCase {
  const char *id;
  uint8_t humidity;
  uint8_t byte6, byte7;
};

static const HumidityCase humidityCases[] = {
    {"52", 52, 0x20, 0x05},       {"67", 67, 0x70, 0x06},
    {"7-no-tens", 7, 0x70, 0x00}, {"zero", 0, 0x00, 0x00},
    {"99", 99, 0x90, 0x09},
};

// @req FUNC-OREGON-007
void Expect_right_humidity_encoding() {
  for (const HumidityCase &c : humidityCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    expected[6] = c.byte6;
    expected[7] = c.byte7;

    // When
    oregonv3.SetHumidity(c.humidity);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}

struct PressureCase {
  const char *id;
  uint16_t pressure;
  uint8_t byte8, byte9;
};

static const PressureCase pressureCases[] = {
    // Old-firmware tinySensor on the bench: RFLink showed 975 hPa while the
    // receiver captured byte 8 = 0xB4 in the same frame (FUNC-OREGON-008)
    {"bench-old-firmware-975-rain", 975, 0xB4, 0x30},
    {"offset-795-is-zero", 795, 0x00, 0x30},
    {"1005-cloudy", 1005, 0xD2, 0x20},
    {"1013-partly-cloudy", 1013, 0xDA, 0x60},
    {"1025-sunny", 1025, 0xEE, 0xC0},
    {"1050-is-255", 1050, 0xFF, 0xC0},
    {"1200-clamped-to-255", 1200, 0xFF, 0xC0},
    {"700-clamped-to-0", 700, 0x00, 0x30},
};

// @req FUNC-OREGON-008
void Expect_right_pressure_encoding() {
  for (const PressureCase &c : pressureCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    expected[8] = c.byte8;
    expected[9] = c.byte9;

    // When
    oregonv3.SetPressure(c.pressure);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}
// @req FUNC-OREGON-009
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
struct RollingCodeCase {
  const char *id;
  uint8_t channel;
  uint8_t rollingCode;
  uint8_t byte2;
};

static const RollingCodeCase rollingCodeCases[] = {
    {"0x85", 0, 0x85, 0x00},
    {"0x00", 0, 0x00, 0x00},
    {"0xFF-keeps-channel-3", 3, 0xFF, 0x40},
};

// @req FUNC-OREGON-010
void Expect_right_rolling_code_encoding() {
  for (const RollingCodeCase &c : rollingCodeCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    oregonv3.SetChannel(c.channel);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    expected[2] = c.byte2;
    expected[3] = c.rollingCode;

    // When
    oregonv3.SetRollingCode(c.rollingCode);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}

struct BatteryCase {
  const char *id;
  bool withTemperature;
  bool batteryLow;
  uint8_t byte4;
};

// Each case first sets the opposite flag, so the call under test changes it
static const BatteryCase batteryCases[] = {
    {"low-sets-bit-2", false, true, 0x04},
    {"ok-clears-bit-2", false, false, 0x00},
    {"low-keeps-temperature-tenths", true, true, 0x54},
    {"ok-keeps-temperature-tenths", true, false, 0x50},
};

// @req FUNC-OREGON-011
void Expect_right_battery_flag_encoding() {
  for (const BatteryCase &c : batteryCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    uint8_t expected[OregonV3::MESSAGE_SIZE_IN_BYTES] = {0};
    if (c.withTemperature) {
      oregonv3.SetTemperature(27.5f);
      expected[5] = 0x27;
    }
    oregonv3.SetBatteryLow(!c.batteryLow);
    expected[4] = c.byte4;

    // When
    oregonv3.SetBatteryLow(c.batteryLow);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(),
                                         OregonV3::MESSAGE_SIZE_IN_BYTES, c.id);
  }
}
// @req FUNC-OREGON-013
void Expect_temperature_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetTemperature(-8.4);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x1) == 0x1);
}
// @req FUNC-OREGON-013
void Expect_humidity_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetHumidity(84);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x2) == 0x2);
}
// @req FUNC-OREGON-013
void Expect_pressure_set_to_change_message_status() {
  OregonV3 oregonv3(&TestHal);

  uint8_t messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_EQUAL(0, messageStatus);

  oregonv3.SetPressure(1000);
  messageStatus = oregonv3.GetMessageStatus();
  TEST_ASSERT_TRUE((messageStatus & 0x4) == 0x4);
}
// @req FUNC-OREGON-013
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
enum Field : uint8_t { TEMP = 1, HUM = 2, PRES = 4 };

struct DeviceCase {
  const char *id;
  uint8_t fields;
  uint8_t byte0, byte1;
};

static const DeviceCase deviceCases[] = {
    {"thn132n-temperature", TEMP, 0xEA, 0x4C},
    {"thgr228n-temperature-humidity", TEMP | HUM, 0x1A, 0x2D},
    {"bthr918-temperature-humidity-pressure", TEMP | HUM | PRES, 0x5A, 0x5D},
};

// @req FUNC-OREGON-012
void Expect_fields_set_to_select_device_id_with_sync_nibble() {
  for (const DeviceCase &c : deviceCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    if (c.fields & TEMP)
      oregonv3.SetTemperature(10.f);
    if (c.fields & HUM)
      oregonv3.SetHumidity(50);
    if (c.fields & PRES)
      oregonv3.SetPressure(1000);
    const uint8_t expected[2] = {c.byte0, c.byte1};

    // When
    oregonv3.FinalizeMessage();

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(expected, oregonv3.GetMessage(), 2,
                                         c.id);
  }
}

struct ReferenceCase {
  const char *id;
  uint8_t channel, rollingCode;
  bool batteryLow;
  uint8_t fields;
  float temperature;
  uint8_t humidity;
  uint16_t pressure;
  const char *payload;
};

static const ReferenceCase referenceCases[] = {
    {"thn132n", 1, 0x3C, true, TEMP, -10.2f, 0, 0, "EA4C103C2410D803000000"},
    {"thgr228n", 3, 0xC4, false, TEMP | HUM, 21.5f, 67, 0,
     "1A2D40C450217006390000"},
    {"bthr918", 3, 0x7E, true, TEMP | HUM | PRES, 21.5f, 60, 1013,
     "5A5D407E54210006DA605F"},
};

// @req FUNC-OREGON-014
void Expect_payload_to_match_rflink_reference() {
  for (const ReferenceCase &c : referenceCases) {
    // Given
    OregonV3 oregonv3(&TestHal);
    oregonv3.SetChannel(c.channel);
    oregonv3.SetRollingCode(c.rollingCode);
    oregonv3.SetBatteryLow(c.batteryLow);
    if (c.fields & TEMP)
      oregonv3.SetTemperature(c.temperature);
    if (c.fields & HUM)
      oregonv3.SetHumidity(c.humidity);
    if (c.fields & PRES)
      oregonv3.SetPressure(c.pressure);

    // When
    oregonv3.FinalizeMessage();

    // Then
    std::string payload = MessageNibblesToString(
        oregonv3.GetMessage(), OregonV3::MESSAGE_SIZE_IN_BYTES);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(c.payload, payload.c_str(), c.id);
  }
}

int main(int, char **) {
  UNITY_BEGIN();

  RUN_TEST(Expect_good_hardware_orders_for_zero);
  RUN_TEST(Expect_good_hardware_orders_for_one);
  RUN_TEST(Expect_bitread_to_read_each_bit_separately);
  RUN_TEST(Expect_nibbles_to_be_sent_lsb_first);
  RUN_TEST(Expect_messages_to_have_preamble_and_postamble);

  RUN_TEST(Expect_right_negative_temperature_encoding);
  RUN_TEST(Expect_negative_sign_to_share_byte_6_with_humidity_units);
  RUN_TEST(Expect_right_positive_temperature_encoding);
  RUN_TEST(Expect_right_humidity_encoding);
  RUN_TEST(Expect_right_pressure_encoding);
  RUN_TEST(Expect_right_channel_encoding);
  RUN_TEST(Expect_right_rolling_code_encoding);
  RUN_TEST(Expect_right_battery_flag_encoding);

  RUN_TEST(Expect_temperature_set_to_change_message_status);
  RUN_TEST(Expect_humidity_set_to_change_message_status);
  RUN_TEST(Expect_pressure_set_to_change_message_status);
  RUN_TEST(Expect_all_values_set_to_change_message_status);

  RUN_TEST(Expect_fields_set_to_select_device_id_with_sync_nibble);

  RUN_TEST(Expect_payload_to_match_rflink_reference);

  return UNITY_END();
}
#endif
