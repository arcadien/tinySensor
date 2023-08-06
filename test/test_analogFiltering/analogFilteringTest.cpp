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
#include <AnalogFilter.h>

void tearDown() {}
void setUp() {}

void Expected_exclusion_to_exclude_value()
{
  AnalogFilter filter(1, 3);
  filter.Push(10);
  filter.Push(0);
  filter.Push(0);
  filter.Push(0);
  TEST_ASSERT_EQUAL(0, filter.Get());
}

void Expected_zero_exclusion_to_not_exclude()
{
  AnalogFilter filter(0, 3);
  filter.Push(2);
  filter.Push(3);
  filter.Push(4);
  TEST_ASSERT_EQUAL(3, filter.Get());
}

void Expected_filter_to_mean_actual_values()
{
  AnalogFilter filter(0, 3);
  filter.Push(2);
  filter.Push(4);
  TEST_ASSERT_EQUAL(3, filter.Get());
}

void Expected_filter_to_mean_values_and_floor_result()
{
  AnalogFilter filter(0, 4);
  filter.Push(1);
  filter.Push(2);
  filter.Push(3);
  filter.Push(4);
  TEST_ASSERT_EQUAL(2, filter.Get());
}
void Expected_filter_to_ignore_supernumerary_values()
{
  AnalogFilter filter(0, 2);
  filter.Push(2);
  filter.Push(2);
  filter.Push(3); // supernumerary
  filter.Push(4); // supernumerary
  TEST_ASSERT_EQUAL(2, filter.Get());
}

int main(int, char **)
{
  UNITY_BEGIN();
  RUN_TEST(Expected_exclusion_to_exclude_value);
  RUN_TEST(Expected_zero_exclusion_to_not_exclude);
  RUN_TEST(Expected_filter_to_mean_values_and_floor_result);
  RUN_TEST(Expected_filter_to_mean_actual_values);
  RUN_TEST(Expected_filter_to_ignore_supernumerary_values);
  return UNITY_END();
}
