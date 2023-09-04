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

void setUp(void) { TestHal.ClearOrders(); }
void tearDown(void) {}

// ----------------- Lacrosse WS7000 tests
void Expect_good_numerical_split() {

  struct TestData {
    float givenTemperature;
    bool expectedIsNegative;
    uint8_t expectedDozen;
    uint8_t expectedUnits;
    uint8_t expectedDecimal;
  };

#define TEST_COUNT 7

  // clang-format off
  TestData testDatas[TEST_COUNT] = {
    {20,   false, 2, 0, 0},
    {-20,  true,  2, 0, 0},
    {200,  false, 9, 9, 0},
    {12.5, false, 1, 2, 5},
    {-200, true,  9, 9, 0},
    {-12.7, true,  1, 2, 7},
    {200.1, false, 9, 9, 1}
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter--) {
    TestData testData = testDatas[testCounter];
    LacrosseWS7000::NumericalSplit temp;
    temp.Set(testData.givenTemperature);

    sprintf(buffer, "Temperature sign #%u", testCounter);
    TEST_ASSERT_MESSAGE(testData.expectedIsNegative == temp.isNegative, buffer);

    sprintf(buffer, "Temperature dozens #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedDozen, temp.dozens, buffer);

    sprintf(buffer, "Temperature units #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedUnits, temp.units, buffer);

    sprintf(buffer, "Temperature decimals #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedDecimal, temp.decimals,
                                   buffer);
  }
#undef TEST_COUNT
}
void Expect_NumericalSplitHundreds_to_split_well() {

  struct TestData {
    float givenValue;
    bool expectedIsNegative;
    uint8_t expectedHundreds;
    uint8_t expectedDozen;
    uint8_t expectedUnits;
    uint8_t expectedDecimal;
  };

#define TEST_COUNT 4

  // clang-format off
  TestData testDatas[TEST_COUNT] = {
    {0.,    false, 0, 0, 0, 0},
    {200.,  false, 2, 0, 0, 0},
    {-921., true,  9, 2, 1, 0},
    {1024.2, true,  9, 2, 1, 2},
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter--) {
    TestData testData = testDatas[testCounter];
    LacrosseWS7000::NumericalSplitHundreds temp;
    temp.Set(testData.givenValue);

    sprintf(buffer, "NumericalSplitHundreds sign #%u", testCounter);
    TEST_ASSERT_MESSAGE(testData.expectedIsNegative == temp.isNegative, buffer);

    sprintf(buffer, "NumericalSplitHundreds hundreds #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedHundreds, temp.hundreds,
                                   buffer);

    sprintf(buffer, "NumericalSplitHundreds dozens #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedDozen, temp.dozens, buffer);

    sprintf(buffer, "NumericalSplitHundreds units #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedUnits, temp.units, buffer);

    sprintf(buffer, "NumericalSplitHundreds decimals #%u", testCounter);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(testData.expectedDecimal, temp.decimals,
                                   buffer);
  }
#undef TEST_COUNT
}

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
      lacrosse.SetLight(1000);

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

void Expect_good_sensor_address_emission() {

  // Note: address is sent along with temperature sign flag

  struct TestData {
    LacrosseWS7000::Address givenAddress;
    bool isTemperatureNegative;
    char expectedNibble[ORDERS_PER_NIBBLE];
  };

#define TEST_COUNT 8
  // clang-format off
  TestData testDatas[8] = {
    {LacrosseWS7000::Address::ZERO,  true,  {BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ONE}  },
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

void Expect_good_temperature_emission() {

  struct TempTestData {
    float givenTemperature;
    uint8_t expectedNibble[(ORDERS_PER_NIBBLE * 3) + (3 * ORDERS_PER_BIT)];
  };

#define TEST_COUNT 6
  // clang-format off
  TempTestData testDatas[TEST_COUNT] = {
  { // #0
    0.0,
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #1
    -12.7,
    {
      BIT_ONE,  BIT_ONE,  BIT_ONE,  BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #2
    10.0,
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ONE,  BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #3
    200.1, // overlimit, set to 100
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE
    }
  },
  {// #4
    23.32,
    {
      BIT_ONE,  BIT_ONE, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ONE,  BIT_ONE, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ONE, BIT_ZERO, BIT_ZERO, SKIPPED_ONE
    } 
  },
  {// #5
    -100.1, // overlimit, set to -100
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE, 
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE
    } 
  }
  };
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TempTestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetAddress(LacrosseWS7000::Address::ZERO);
    lacrosse.SetTemperature(testData.givenTemperature);
    lacrosse.Send();

    sprintf(buffer, "Sensor temperature #%u\n", testCounter);
    uint8_t *actualOrders = TestHal.GetOrders();

    // Note: before address, we have:
    // - preamble                      (10 bits + 1)
    // - sensor type                   (4 bits  + 1)
    // - sensor address and temp. sign (4 bits  + 1)

    // so comparison shall start at order
    // ((10 + 1 + 4 + 1 + 4 + 1) * ORDERS_PER_BIT)
    auto shift = 21 * ORDERS_PER_BIT;
    uint8_t *actualTemperatureStart = actualOrders + shift;

    TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(testData.expectedNibble,
                                         actualTemperatureStart, 75, buffer);
  }
#undef TEST_COUNT
}

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
    100,
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE
    } 
  },
  {// #3
    101, // clamped to 100
    {
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,
      BIT_ZERO, BIT_ONE,  BIT_ZERO, BIT_ONE,  SKIPPED_ONE
    } 
  }
};
  // clang-format on

  for (uint8_t testCounter = 0; testCounter < TEST_COUNT; testCounter++) {
    TempTestData testData = testDatas[testCounter];
    TestHal.ClearOrders();
    LacrosseWS7000 lacrosse(&TestHal);
    lacrosse.SetTemperature(42); // unchecked
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
    lacrosse.SetTemperature(42); // unchecked
    lacrosse.SetHumidity(42);    // unchecked
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
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders, 230);
}

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
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders, 305);
}

void Expect_encoding_of_simple_temperature_and_humi_and_pressure_message(){
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

      BIT_ZERO, BIT_ZERO, BIT_ZERO, BIT_ZERO, SKIPPED_ONE,  // 205 - humi. decimals
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
  lacrosse.SetHumidity(33);
  lacrosse.SetPressure(994.8);
  lacrosse.Send();
  uint8_t *actualOrders = TestHal.GetOrders();
  TEST_ASSERT_EQUAL_CHAR_ARRAY(EXPECTED_ORDERS, actualOrders, 305);
}

int main(int, char **) {
  UNITY_BEGIN();

  RUN_TEST(Expect_good_numerical_split);
  RUN_TEST(Expect_NumericalSplitHundreds_to_split_well);
  RUN_TEST(Expect_preamble_is_10_times_0);
  RUN_TEST(Expect_good_sensor_type_emission);
  RUN_TEST(Expect_good_sensor_address_emission);
  RUN_TEST(Expect_good_temperature_emission);
  RUN_TEST(Expect_good_humidity_emission);
  RUN_TEST(Expect_good_pressure_emission);
  RUN_TEST(Expect_encoding_of_simple_temperature_message);
  RUN_TEST(Expect_encoding_of_simple_temperature_and_humi_message);
  RUN_TEST(Expect_encoding_of_simple_temperature_and_humi_and_pressure_message);

  return UNITY_END();
}
#endif