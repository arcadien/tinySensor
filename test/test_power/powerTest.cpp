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

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unity.h>

#ifdef AVR
#include <Attiny84aHal.h>

void Expect_vcc_voltage_is_read() {
  Attiny84aHal hal;
  uint16_t vccVoltage = hal.GetVccVoltageMv();
  TEST_ASSERT_EQUAL_INT16_MESSAGE(3300, vccVoltage, "vccVoltage");
}

void Expect_battery_voltage_is_read() {
  Attiny84aHal hal;
  uint16_t batteryVoltage = hal.GetBatteryVoltageMv();
  TEST_ASSERT_EQUAL_INT16_MESSAGE(2400, batteryVoltage, "batteryVoltage");
}
void Expect_sensors_can_be_powered() {
  Attiny84aHal hal;
  hal.PowerOnSensors();
  TEST_ASSERT_EQUAL(PA2, (PORTA & PA2));
}
void Expect_sensors_can_be_unpowered() {
  Attiny84aHal hal;
  hal.PowerOffSensors();
  TEST_ASSERT_EQUAL(0, (PORTA & PA2));
}

void Expect_target_can_hibernate() {
  // hard to test, need external way of time measurement
  // because counters should be stopped during sleep
  Attiny84aHal hal;
  hal.Hibernate(0);
}
#else
#include <TestHal.h>

void Expect_vcc_voltage_is_read() {
  uint16_t vccVoltage = TestHal.GetVccVoltageMv();
  TEST_ASSERT_EQUAL_INT16_MESSAGE(3300, vccVoltage, "vccVoltage");
}

void Expect_battery_voltage_is_read() {
  uint16_t batteryVoltage = TestHal.GetBatteryVoltageMv();
  TEST_ASSERT_EQUAL_INT16_MESSAGE(2400, batteryVoltage, "batteryVoltage");
}
void Expect_sensors_can_be_powered() {
  TestHal.SensorIsPowered = false;
  TestHal.PowerOnSensors();
  TEST_ASSERT_EQUAL(true, TestHal.SensorIsPowered);
}
void Expect_sensors_can_be_unpowered() {
  TestHal.SensorIsPowered = false;
  TestHal.PowerOffSensors();
  TEST_ASSERT_EQUAL(false, TestHal.SensorIsPowered);
}
void Expect_target_can_hibernate() {
  auto begin = std::chrono::high_resolution_clock::now();
  TestHal.Hibernate(2);
  auto end = std::chrono::high_resolution_clock::now();
  TEST_ASSERT_EQUAL(2000, std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count());
}
#endif

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_vcc_voltage_is_read);
  RUN_TEST(Expect_battery_voltage_is_read);
  RUN_TEST(Expect_sensors_can_be_unpowered);
  RUN_TEST(Expect_sensors_can_be_powered);
  RUN_TEST(Expect_target_can_hibernate);
  return UNITY_END();
}
