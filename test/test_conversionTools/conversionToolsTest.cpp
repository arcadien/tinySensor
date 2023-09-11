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

#include <conversionTools.h>
#include <unity.h>

void tearDown() {}
void setUp() {}

void Test_dec16ToHex() {
  struct TestData {
    uint16_t given;
    uint16_t expected;
  };

  // clang-format off
  TestData testDatas[] = {
    {0, 0},
    {5, 5}, 
    {4200,      0x4200}, 
    {9999,      0x9999},
    {10000,     0xA000}, // overflow
  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / sizeof(TestData);
  for (uint8_t testCounter = 0; testCounter < testCount; testCounter++) {
    TestData testData = testDatas[testCounter];
    uint16_t actual = ConversionTools::dec16ToHex(testData.given);
    TEST_ASSERT_EQUAL_UINT16(testData.expected, actual);
  }
}
void Test_dec32ToHex() {
  struct TestData {
    uint32_t given;
    uint32_t expected;
  };

  // clang-format off
  TestData testDatas[] = {
    {0, 0},
    {5, 5}, 
    {4200,      0x4200}, 
    {999999,     0x999999},
    {99999999,   0x99999999}, // max
    {99999999+1, 0xA0000000}, // overflow

  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / sizeof(TestData);
  for (uint8_t testCounter = 0; testCounter < testCount; testCounter++) {
    TestData testData = testDatas[testCounter];
    uint32_t actual = ConversionTools::dec32ToHex(testData.given);
    TEST_ASSERT_EQUAL_UINT32(testData.expected, actual);
  }
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Test_dec16ToHex);
  RUN_TEST(Test_dec32ToHex);
  return UNITY_END();
}
