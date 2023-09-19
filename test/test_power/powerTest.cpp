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

#include <TestHal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

static char buffer[512];

void Expect_vcc_is_computed_using_internal_1v1_ref() {

  struct TestData {
    uint16_t raw1v1Measurement;
    uint16_t known1v1value;
    uint16_t expectedVccMv;
  };

  // clang-format off
  TestData testDatas[] = {
    {0,   1100, 0},
    {226, 1100, 4984},
    {765, 1055, 1412},
    {1024,1100, 1100},
    {1,   1100, 12288},
  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / 6;

  for (uint8_t testCounter = 0; testCounter < testCount; testCounter++) {
    TestData testData = testDatas[testCounter];
    TestHal.rawInternal11ref = testData.raw1v1Measurement;
    uint16_t actual = TestHal.ComputeVccMv(testData.known1v1value);

    sprintf(buffer, "Test #%u", testCounter);

    TEST_ASSERT_EQUAL_UINT16_MESSAGE(testData.expectedVccMv, actual, buffer);
  }
}

void Expect_convertion_from_adc_to_mv_is_right() {

  struct TestData {
    uint16_t vccMv;
    uint16_t rawBatteryMeasurement;
    uint16_t expectedVBatt;
  };

  // clang-format off
  TestData testDatas[] = {
    {0,    0,   0},
    {5000, 512, 2500},
    {5000, 1,   4},
  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / 6;

  for (uint8_t testCounter = 0; testCounter < testCount; testCounter++) {
    TestData testData = testDatas[testCounter];

    uint16_t vBatt = TestHal.ConvertAnalogValueToMv(
        testData.rawBatteryMeasurement, testData.vccMv);

    sprintf(buffer, "Test #%u", testCounter);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(testData.expectedVBatt, vBatt, buffer);
  }
}

void tearDown() {}
void setUp() {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_vcc_is_computed_using_internal_1v1_ref);
  RUN_TEST(Expect_convertion_from_adc_to_mv_is_right);
  return UNITY_END();
}
