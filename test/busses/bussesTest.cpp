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
void Expect_I2C_can_be_configured()
{
  Attiny84aHal hal;
  hal.InitI2C();
  TEST_ASSERT_EQUAL(true, TestHal.I2CIsConfigured);
}
#else
#include <TestHal.h>

void Expect_I2C_can_be_configured()
{
  TestHal.I2CIsConfigured = false;
  TestHal.InitI2C();
  TEST_ASSERT_EQUAL(true, TestHal.I2CIsConfigured);
}
#endif

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(Expect_I2C_can_be_configured);
  return UNITY_END();
}
