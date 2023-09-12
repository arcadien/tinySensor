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

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

#include <TestHal.h>
#include <x10rf.h>

typedef struct X10SecurityTestData {
  uint8_t address;
  uint8_t id;
  uint8_t command;
  uint8_t expectedCRC;
} X10SecurityTestData;

typedef struct X10SwitchTestData {
  uint8_t houseCode;
  uint8_t unitCode;
  uint8_t command;
  uint8_t expectedMessage[4];
} X10SwitchTestData;

typedef struct X10SensorTestData {
  uint8_t address;
  uint8_t type;
  uint8_t packetType;
  uint8_t value;
  uint8_t expectedMessage[4];
} X10SensorTestData;

typedef struct X10MeterTestData {
  uint8_t address;
  uint8_t packetType;
  uint32_t value;
  uint8_t expectedMessage[6];
} X10MeterTestData;

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitWrite(value, bit, bitvalue)                                         \
  (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

void Expect_x10_switch_right_encoding() {

  char buffer[256];
  uint8_t testCount = 1;

  // clang-format off
  X10SwitchTestData testDatas[] = {
    {'b', 0x0f, OFF, {0x74,0x8B,0x68,0x97}}
  };
  // clang-format on

  for (uint8_t testIndex = 0; testIndex < testCount; testIndex++) {
    X10SwitchTestData testData = testDatas[testIndex];
    x10rf x10Encoder(&TestHal, 1);
    x10Encoder.x10Switch(testData.houseCode, testData.unitCode,
                         testData.command);

    uint8_t *actual = x10Encoder.getMessage();

    sprintf(buffer, "Checking X10 Switch message test #%u", testIndex);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(testData.expectedMessage, actual, 4,
                                         buffer);
  }
}

void Expect_x10_security_right_parity() {

  char buffer[256];
  uint8_t testCount = 5;
  X10SecurityTestData testDatas[] = {{0x40, 0x02, 0x44, 0x01},
                                     {0x40, 0x12, 0x41, 0x00},
                                     {0x43, 0x02, 0x10, 0x01},
                                     {0x21, 0x03, 0x01, 0x00},
                                     {0x0F, 0x02, 0x0F, 0x01}};

  for (uint8_t testIndex = 0; testIndex < testCount; testIndex++) {
    X10SecurityTestData testData = testDatas[testIndex];
    x10rf x10Encoder(&TestHal, 1);
    x10Encoder.x10Security(testData.address, testData.id, testData.command);

    uint8_t actual = x10Encoder.x10SecurityParity(x10Encoder.getMessage());

    sprintf(buffer, "Checking X10 Security parity test #%u", testIndex);
    TEST_ASSERT_EQUAL_MESSAGE(testData.expectedCRC, actual, buffer);
  }
}

void Expect_x10_meter_right_encoding() {

  char buffer[256];

  // clang-format off
  X10MeterTestData testDatas[] = {
    // simple value
    {0xFA, 0x00, 0x0,       {0xFA,0x0A,0x00,0x00,0x00,0x0C}}, // normal type, simple set value (0)
    {0xFB, 0x00, 0x0005DC,  {0xFB,0x0B,0x05,0xDC,0x00,0x0C}}, // normal type, simple set value 
    {0xFB, 0x00, 0xFE0102,  {0xFB,0x0B,0x01,0x02,0xFE,0x0A}}, // normal type, simple set value 
    {0xFC, 0x00, 0xFFFFFF,  {0xFC,0x0C,0xFF,0xFF,0xFF,0x0E}}, // normal type, simple set value (max, FFFFFF)
    {0xFE, 0x00, 0x1000000, {0xFE,0x0E,0x00,0x00,0x00,0x04}}, // normal type, simple set value (overflow)

    // TODO: tests for other modes
    // interval set
    // calibration in microseconds
    // new address set
    // counter reset
    // counter set to value
    // set interval within 5s
    // calibration mode within 5s
    // set address mode
  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / sizeof(X10MeterTestData);
  for (uint8_t testIndex = 0; testIndex < testCount; testIndex++) {
    x10rf x10Encoder(&TestHal, 1);

    X10MeterTestData testData = testDatas[testIndex];
    x10Encoder.RFXmeter(testData.address, testData.packetType, testData.value);

    uint8_t *actual = x10Encoder.getMessage();

    sprintf(buffer, "Checking X10 meter message test #%u", testIndex);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(testData.expectedMessage, actual, 6,
                                         buffer);
  }
}

void Expect_x10_sensor_right_encoding() {
  char buffer[256];

  // clang-format off
  X10SensorTestData testDatas[] = {
    {0xF2, 't', 't', 24,  {0xC8,0x38,0x18, 0x07}}, // temp. 24.0
    {0xF3, 't', 'T', 24,  {0xCC,0x3C,0x18, 0x87}}, // temp 24.5
    {0xF4, 'a', 'h', 42,  {0xD1,0x21,0x2A, 0x20}}, // RH 42%
    {0xF5, 'm', 'x', 100, {0xD7,0x27,0x64, 0x08}}, // message, not decoded in RFLink
    {0xF6, 'v', 'v', 18,  {0xDA,0x2A,0x12, 0x09}}, // voltage, 18
  };
  // clang-format on

  uint8_t testCount = sizeof(testDatas) / sizeof(X10SensorTestData);
  for (uint8_t testIndex = 0; testIndex < testCount; testIndex++) {
    x10rf x10Encoder(&TestHal, 1);

    X10SensorTestData testData = testDatas[testIndex];
    x10Encoder.RFXsensor(testData.address, testData.type, testData.packetType,
                         testData.value);
    uint8_t *actual = x10Encoder.getMessage();

    sprintf(buffer, "Checking X10 sensor message test #%u", testIndex);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(testData.expectedMessage, actual, 4,
                                         buffer);
  }
}

void setUp(void) { TestHal.ClearOrders(); }
void tearDown(void) {}

int main(int, char **) {
  UnityBegin("x10LibTest.cpp");

  RUN_TEST(Expect_x10_switch_right_encoding);
  RUN_TEST(Expect_x10_security_right_parity);
  RUN_TEST(Expect_x10_meter_right_encoding);
  RUN_TEST(Expect_x10_sensor_right_encoding);

  return UNITY_END();
}
#endif