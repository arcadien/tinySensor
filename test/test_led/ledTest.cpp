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

void tearDown() {}
void setUp() {}

#ifdef AVR
#include <Attiny84aHal.h>



void Expect_led_can_switch_on() {
  Attiny84aHal hal;
  hal.LedOn();
  TEST_ASSERT((PORTB & PB1) == PB1);
}

void Expect_led_can_switch_off() {
  Attiny84aHal hal;
  hal..LedOff();
  TEST_ASSERT((PORTB & PB1) == 0);
}
#else
#include <TestHal.h>

void Expect_led_can_switch_on() {
  TestHal.LedOn();
  TEST_ASSERT(TestHal.IsLedOn);
}

void Expect_led_can_switch_off() {
  TestHal.LedOff();
  TEST_ASSERT(!(TestHal.IsLedOn));
}
#endif

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Expect_led_can_switch_on);
  RUN_TEST(Expect_led_can_switch_off);
  return UNITY_END();
}
