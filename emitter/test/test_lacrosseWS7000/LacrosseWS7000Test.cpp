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

#include <LacrosseWS7000.h>
#include <TestHal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

// for test messages
static char buffer[512];

// clang-format off
#define BIT_ONE     'H', 'A', 'L', 'A', 'A'
#define SKIPPED_ONE 'H', 'A', 'L', 'A', 'A'
#define BIT_ZERO    'H', 'A', 'A', 'L', 'A'
// clang-format on

#define ORDERS_PER_BIT 5
#define ORDERS_PER_NIBBLE (4 * ORDERS_PER_BIT)

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

// ----------------- Lacrosse WS7000 tests
// @req FUNC-LACROSSE-004
void Expect_good_numerical_split() {
  struct TestData {
    float givenValue;
    LacrosseWS7000::NumericalValueSplitter::Result expected;
  };

#define TEST_COUNT 6

  // clang-format off
  TestData testDatas[TEST_COUNT] = {
    {0.,     {false, 0, 0, 0, 0}},
    {200.2,   {false, 2, 0, 0, 2}},
    {-921.,  {true,  9, 2, 1, 0}},
    {1024.2, {false, 10,2, 4, 2}},
    {26.1,   {false, 0, 2, 6, 1}},
    {96.8,   {false, 0, 9, 6, 8}}
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TestData testData = testDatas[testCounter];

    LacrosseWS7000::NumericalValueSplitter::Result actual;
    LacrosseWS7000::NumericalValueSplitter::Split(testData.givenValue, &actual);

    sprintf(buffer, "NumericalValueSplitter sign #%u", testCounter);
    TEST_ASSERT_MESSAGE((testData.expected.isNegative == actual.isNegative),
                        buffer);

    sprintf(buffer, "NumericalValueSplitter hundreds #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expected.hundreds, actual.hundreds,
                                   buffer);

    sprintf(buffer, "NumericalValueSplitter dozens #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expected.dozens, actual.dozens,
                                   buffer);

    sprintf(buffer, "NumericalValueSplitter units #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expected.units, actual.units,
                                   buffer);

    sprintf(buffer, "NumericalValueSplitter decimals #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expected.decimals, actual.decimals,
                                   buffer);
  }
#undef TEST_COUNT
}

// @req FUNC-LACROSSE-001
void Expect_preamble_is_10_times_0() {
  static const uint8_t PREAMBLE_IS_TEN_ZEROS[] = {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO};

  LacrosseWS7000 lacrosse(&TestHal);
  lacrosse.Send();
  uint8_t *actualOrdersForPreamble = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(PREAMBLE_IS_TEN_ZEROS, actualOrdersForPreamble,
                               sizeof(PREAMBLE_IS_TEN_ZEROS));
}

// @req FUNC-LACROSSE-002
void Expect_good_sensor_type_emission() {
  struct TestData {
    bool givenHasLux;
    bool givenHasTemperature;
    bool givenHasHumidity;
    bool givenHasPressure;
    char expectedNibble[ORDERS_PER_NIBBLE];
  };

#define TEST_COUNT 5
  // clang-format off
  TestData testDatas[TEST_COUNT] = {
    {false, true,  false, false, {BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO}  }, // temp only (0)
    {false, true,  true,  false, {BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO}  }, // temp + rh (1)
    {false, true,  true,  true,  {BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO } }, // temp + rh + baro (4)
    {true,  false, false, false, {BIT_ONE,  BIT_ZERO, BIT_ONE,  BIT_ZERO}  }, // lux       (5)
    {true,  true,  true,  true,  {BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ONE }  }  // nothing (error, F)
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);

    if (testData.givenHasLux)
      lacrosse.SetLuminosity(1000);

    if (testData.givenHasTemperature)
      lacrosse.SetTemperature(24.0);

    if (testData.givenHasHumidity)
      lacrosse.SetHumidity(42);

    if (testData.givenHasPressure)
      lacrosse.SetPressure(900);

    lacrosse.Send();

    sprintf(buffer, "Test case #%u\n", testCounter);
    uint8_t *actualOrders = TestHal.GetOrders();

    // Note: before address, we have preamble (10 bits + 1 dead bit)
    // so comparison shall start at order ((10 + 1) * ORDERS_PER_BIT)
    auto shift = ((10 + 1) * ORDERS_PER_BIT);
    uint8_t *actualSensorTypeStart = actualOrders + shift;

    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(testData.expectedNibble,
                                         actualSensorTypeStart,
                                         ORDERS_PER_NIBBLE, buffer);
  }
#undef TEST_COUNT
}

// @req FUNC-LACROSSE-003
void Expect_good_sensor_address_emission() {
  // Note: address is sent along with temperature sign flag

  struct TestData {
    LacrosseWS7000::Address givenAddress;
    bool isTemperatureNegative;
    char expectedNibble[ORDERS_PER_NIBBLE];
  };

#define TEST_COUNT 8
  // clang-format off
  TestData testDatas[9] = {
    {LacrosseWS7000::Address::ZERO,  true,  {BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ONE}  },
    {LacrosseWS7000::Address::ONE,   true,  {BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE}  },
    {LacrosseWS7000::Address::ONE,   false, {BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO} },
    {LacrosseWS7000::Address::TWO,   false, {BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO} },
    {LacrosseWS7000::Address::THREE, false, {BIT_ONE,  BIT_ONE,  BIT_ZERO, BIT_ZERO} },
    {LacrosseWS7000::Address::FOUR,  true,  {BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ONE}  },
    {LacrosseWS7000::Address::FIVE,  false, {BIT_ONE,  BIT_ZERO, BIT_ONE,  BIT_ZERO} },
    {LacrosseWS7000::Address::SIX,   true,  {BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ONE}  },
    {LacrosseWS7000::Address::SEVEN, false, {BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO} },
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetAddress(testData.givenAddress);
    if (testData.isTemperatureNegative) {
      lacrosse.SetTemperature(-10.2);
    } else {
      lacrosse.SetTemperature(10.2);
    }
    lacrosse.Send();

    sprintf(buffer, "sensor address #%u\n", testCounter);
    uint8_t *actualOrders = TestHal.GetOrders();

    // Note: before address, we have:
    // - preamble    (10 bits + 1 dead bit)
    // - sensor type (4 bits  + 1 dead bit)
    // so comparison shall start at order ((10 + 1 + 4 + 1) * ORDERS_PER_BIT)
    auto shift = ((10 + 1 + 4 + 1) * ORDERS_PER_BIT);
    uint8_t *actualSensorAddressStart = actualOrders + shift;

    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(testData.expectedNibble,
                                         actualSensorAddressStart,
                                         4 * ORDERS_PER_BIT, buffer);
  }
#undef TEST_COUNT
}

// Orders of nibbles as sent: four bits LSB first, each nibble followed by a
// one bit separator (FUNC-LACROSSE-009)
static std::vector<unsigned char> NibbleOrders(
    std::initializer_list<uint8_t> nibbles) {
  static const unsigned char one[] = {BIT_ONE};
  static const unsigned char zero[] = {BIT_ZERO};
  std::vector<unsigned char> orders;
  for (uint8_t nibble : nibbles) {
    for (uint8_t bit = 0; bit < 4; bit++) {
      const unsigned char *b = ((nibble >> bit) & 1) ? one : zero;
      orders.insert(orders.end(), b, b + ORDERS_PER_BIT);
    }
    orders.insert(orders.end(), one, one + ORDERS_PER_BIT);
  }
  return orders;
}

// Compares the sent orders from bit `bitOffset` on with `nibbles`
static void AssertNibblesSentAt(uint16_t bitOffset,
                                std::initializer_list<uint8_t> nibbles,
                                const char *id) {
  const std::vector<unsigned char> expected = NibbleOrders(nibbles);
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(
      bitOffset * ORDERS_PER_BIT + expected.size(), TestHal.Orders.size(), id);
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(
      expected.data(), TestHal.GetOrders() + bitOffset * ORDERS_PER_BIT,
      expected.size(), id);
}

struct TemperatureCase {
  const char *id;
  float temperature;
  bool negative;
  uint8_t tenths, units, tens;
};

static const TemperatureCase temperatureCases[] = {
    {"zero", 0.0f, false, 0, 0, 0},
    {"-12.7", -12.7f, true, 7, 2, 1},
    {"10.0", 10.0f, false, 0, 0, 1},
    {"23.32", 23.32f, false, 3, 3, 2},
    {"21.97-rounds-up-to-22.0", 21.97f, false, 0, 2, 2},
    {"21.94-rounds-down-to-21.9", 21.94f, false, 9, 1, 2},
    {"21.25-half-rounds-up", 21.25f, false, 3, 1, 2},
    {"-10.25-half-rounds-away-from-zero", -10.25f, true, 3, 0, 1},
    {"99.96-clamped-to-99.9", 99.96f, false, 9, 9, 9},
    {"200.1-clamped-to-99.9", 200.1f, false, 9, 9, 9},
    {"-100.1-clamped-to--99.9", -100.1f, true, 9, 9, 9},
    // rounds to 0.0, which is not negative
    {"-0.04-sends-0.0-without-sign", -0.04f, false, 0, 0, 0},
};

// @req FUNC-LACROSSE-004
void Expect_good_temperature_emission() {
  for (const TemperatureCase &c : temperatureCases) {
    // Given
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetAddress(LacrosseWS7000::Address::ZERO);
    lacrosse.SetTemperature(c.temperature);

    // When
    lacrosse.Send();

    // Then: preamble (10 + 1 bits) and type (5 bits) come first; address
    // ZERO leaves only the sign bit in the address nibble
    const uint8_t addressAndSign = c.negative ? 0x8 : 0x0;
    AssertNibblesSentAt(16, {addressAndSign, c.tenths, c.units, c.tens}, c.id);
  }
}

// @req FUNC-LACROSSE-005
void Expect_good_humidity_emission() {
  struct TempTestData {
    uint8_t givenHumidity;
    uint8_t expectedNibble[(ORDERS_PER_NIBBLE * 3) + (3 * ORDERS_PER_BIT)];
  };

#define TEST_COUNT 4
  // clang-format off
  TempTestData testDatas[TEST_COUNT] = {
  { // #0
    0,
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #1
    12,
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #2
    100, // clamped to 99.9
    {
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE, // decimals
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE, // units
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE  // dozens
    } 
  },
  {// #3
    101, // clamped to 99.9
    {
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE,
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE,
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE
    } 
  }
};
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TempTestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetTemperature(42);  // unchecked
    lacrosse.SetHumidity(testData.givenHumidity);
    lacrosse.Send();

    sprintf(buffer, "Sensor humidity #%u", testCounter);
    uint8_t *actualOrders = TestHal.GetOrders();

    // Note: before address, we have:
    // - preamble                      (11 bits)
    // - sensor type                   (1 nibble)
    // - sensor address and temp. sign (1 nibble)
    // - sensor temp.                  (3 nibbles)

    // so comparison shall start at order
    // (11 * ORDERS_PER_BIT) + (5 * ORDERS_PER_NIBBLE) + (5 * ORDERS_PER_BIT)
    auto shift = ((11 * ORDERS_PER_BIT) + (5 * ORDERS_PER_NIBBLE) +
                  (5 * ORDERS_PER_BIT));
    uint8_t *actualHumidityStart = actualOrders + shift;

    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(testData.expectedNibble,
                                         actualHumidityStart, 75, buffer);
  }
#undef TEST_COUNT
}

// @req FUNC-LACROSSE-006
void Expect_good_pressure_emission() {
  struct TempTestData {
    float givenPressure;
    uint8_t expectedNibble[(ORDERS_PER_NIBBLE * 4) + (4 * ORDERS_PER_BIT)];
  };

#define TEST_COUNT 3
  // clang-format off
  TempTestData testDatas[TEST_COUNT] = {
  { // #0 
    0, // should be raised to 850-200 by implementation
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // units
      BIT_ONE,  BIT_ZERO, BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // dozen
      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // hundreds
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE  // decimals
    } 
  },
  {// #1
    794.1+200,
    {
      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // units
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE, // dozen
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // hundreds
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE  // decimals 
    } 
  },
  {// #2
    794.8+200,
    {
      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // units
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE, // dozen
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // hundreds
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE  // decimals
    } 
  }
};
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TempTestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetTemperature(42);  // unchecked
    lacrosse.SetHumidity(42);     // unchecked
    lacrosse.SetPressure(testData.givenPressure);
    lacrosse.Send();

    sprintf(buffer, "Sensor pressure #%u", testCounter);
    uint8_t *actualOrders = TestHal.GetOrders();

    // Note: before address, we have:
    // - preamble                      (11 bits)
    // - sensor type                   (1 nibble)
    // - sensor address and temp. sign (1 nibble)
    // - sensor temp.                  (3 nibbles)
    // - sensor humi.                  (3 nibbles)

    // so comparison shall start at order
    // (11 * ORDERS_PER_BIT) + (8 * ORDERS_PER_NIBBLE) + (8 * ORDERS_PER_BIT)
    auto shift = ((11 * ORDERS_PER_BIT) + (8 * ORDERS_PER_NIBBLE) +
                  (8 * ORDERS_PER_BIT));
    uint8_t *actualPressureStart = actualOrders + shift;

    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(testData.expectedNibble,
                                         actualPressureStart, 100, buffer);
  }
#undef TEST_COUNT
}

// @req FUNC-LACROSSE-008
void Expect_encoding_of_simple_temperature_message() {
  // clang-format off
  // 0 0 0 0  1 1 1 0  0 1 1 1  0 0 1 0  0 0 0 1  1 0 1 0  0 1 1 1
  //    0        E        7        2        1         A       7
  //
  // Sensor type 0, address 6, 12.7 (negative) checkXor A, checksum 7
  //
  // complete message:
  // 0 0 0 0 0 0 0 0 0 0 1  0 0 0 0 1  0 1 1 1 1  1 1 1 0 1  0 1 0 0 1  1 0 0 0 1  0 1 0 1 1  1 1 1 0 1
  static const  uint8_t EXPECTED_ORDERS[230] = {
      
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 25  - preamble 1/2
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 50  - preamble 2/2
      SKIPPED_ONE,                                          // 55  - check bit
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 80  - type
      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ONE,  SKIPPED_ONE,  // 105 - address and sign
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 130 - temp. decimals
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 155 - temp. unit
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 180 - temp. dozen
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE,  // 205 - checkXor
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE   // 230 - checkSum
  };
  // clang-format on

  LacrosseWS7000 lacrosse(&TestHal);
  lacrosse.SetAddress(LacrosseWS7000::Address::SIX);
  lacrosse.SetTemperature(-12.7);
  lacrosse.Send();
  uint8_t *actualOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders,
                               sizeof(EXPECTED_ORDERS));
}

// @req FUNC-LACROSSE-008
void Expect_encoding_of_simple_temperature_and_humi_message() {
  // clang-format off
  static const  uint8_t EXPECTED_ORDERS[305] = {
      
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 25  - preamble 1/2
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 50  - preamble 2/2
      SKIPPED_ONE,                                          // 55  - check bit
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 80  - type
      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ONE,  SKIPPED_ONE,  // 105 - address and sign
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 130 - temp. decimals
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 155 - temp. unit
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 180 - temp. dozen
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 205 - humi. decimals
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 230 - humi. unit
      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 255 - humi. dozen
      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ONE,  SKIPPED_ONE,  // 280 - checkXor
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE   // 305 - checkSum
  };
  // clang-format on

  LacrosseWS7000 lacrosse(&TestHal);
  lacrosse.SetAddress(LacrosseWS7000::Address::SIX);
  lacrosse.SetTemperature(-12.7);
  lacrosse.SetHumidity(61);
  lacrosse.Send();
  uint8_t *actualOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders,
                               sizeof(EXPECTED_ORDERS));
}

// @req FUNC-LACROSSE-009
void Expect_encoding_of_simple_temperature_and_humi_and_pressure_message() {
  // clang-format off
  static const  uint8_t EXPECTED_ORDERS[405] = {
      
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 25  - preamble 1/2
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,     // 50  - preamble 2/2
      SKIPPED_ONE,                                          // 55  - check bit

      BIT_ZERO,  BIT_ZERO, BIT_ONE, BIT_ZERO, SKIPPED_ONE,  // 80  - type

      BIT_ONE, BIT_ONE,  BIT_ZERO,  BIT_ZERO,  SKIPPED_ONE,  // 105 - address and sign

      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 130 - temp. decimals

      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 155 - temp. unit
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 180 - temp. dozen
      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 205 - humi. decimals
      BIT_ONE,  BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 230 - humi. unit
      BIT_ONE,  BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 255 - humi. dozen

      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 280 - press. units
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE,  // 305 - press. dozen
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE,  // 330 - press. hundreds
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE,  // 355 - press. decimals

      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ZERO,  SKIPPED_ONE, // 380 - checkXor
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ONE,   BIT_ONE      // 405 - checkSum
  };
  // clang-format on

  LacrosseWS7000 lacrosse(&TestHal);
  lacrosse.SetAddress(LacrosseWS7000::Address::THREE);
  lacrosse.SetTemperature(26.1);
  lacrosse.SetHumidity(33.4);
  lacrosse.SetPressure(994.8);
  lacrosse.Send();
  uint8_t *actualOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders,
                               sizeof(EXPECTED_ORDERS));
}

// @req FUNC-LACROSSE-007
void Expect_encoding_of_simple_luminosity_message() {
  // clang-format off
  static const  uint8_t EXPECTED_ORDERS[] = {
      
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,    // 25  - preamble 1/2
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO,    // 50  - preamble 2/2
      SKIPPED_ONE,                                         // 55  - check bit
      BIT_ONE,  BIT_ZERO,  BIT_ONE, BIT_ZERO, SKIPPED_ONE, // 80  - type
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // 105 - address and sign
      BIT_ZERO, BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, // 130 - lum. unit
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ONE,  SKIPPED_ONE, // 155 - lum. dozen
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // 180 - lum. hundreds
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // 205 - lum. power of 10
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // 230 - lum. expo nibble 1
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // 255 - lum. expo nibble 2
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, // 280 - lum. expo nibble 3
      BIT_ZERO, BIT_ZERO, BIT_ONE,  BIT_ONE,  SKIPPED_ONE, // 380 - checkXor
      BIT_ONE,  BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE  // 405 - checkSum
  };
  // clang-format on

  LacrosseWS7000 lacrosse(&TestHal);
  lacrosse.SetAddress(LacrosseWS7000::Address::SEVEN);
  lacrosse.SetLuminosity(86.0);
  lacrosse.Send();
  uint8_t *actualOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders,
                               sizeof(EXPECTED_ORDERS));
}

int main(int, char **) {
  UNITY_BEGIN();

  RUN_TEST(Expect_good_numerical_split);
  RUN_TEST(Expect_preamble_is_10_times_0);
  RUN_TEST(Expect_good_sensor_type_emission);
  RUN_TEST(Expect_good_sensor_address_emission);
  RUN_TEST(Expect_good_temperature_emission);
  RUN_TEST(Expect_good_humidity_emission);
  RUN_TEST(Expect_good_pressure_emission);
  RUN_TEST(Expect_encoding_of_simple_temperature_message);
  RUN_TEST(Expect_encoding_of_simple_temperature_and_humi_message);
  RUN_TEST(Expect_encoding_of_simple_temperature_and_humi_and_pressure_message);
  RUN_TEST(Expect_encoding_of_simple_luminosity_message);

  return UNITY_END();
}
#endif