/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019 Aurelien Labrosse
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

// This suite validates TECH-MAIN-001: extraction of the measurement-and-
// transmit cycle from main()'s while(1) loop into a free function
//
//   void runCycle(Hal* hal);
//
// declared in `include/main.h` and defined in `src/main.cpp`.
//
// PRODUCTION-SIDE SCAFFOLDING REQUIRED (not created by this test-builder
// pass, per the test-builder hard constraint of not touching production
// code):
//   - include/main.h must declare `void runCycle(Hal* hal);`
//   - src/main.cpp must define `runCycle`, containing the full former
//     while(1) loop body plus the environmentEncoder construction/
//     configuration that currently precedes the loop (OregonV3 construction,
//     SetChannel/SetRollingCode for USE_OREGON, SetAddress for
//     USE_LACROSSE). main() must be reduced to hal.Init() followed by
//     while (1) { runCycle(&hal); }.
//   - src/main.cpp's `int main()` must remain guarded so that linking this
//     test target (which supplies its own Unity `main()`) does not collide
//     with it, OR the build must otherwise make `runCycle` callable from a
//     native test binary. How that is achieved (e.g. excluding main.cpp's
//     `main()` symbol under a NATIVE-aware guard, or relying on PlatformIO's
//     default test src filtering plus a compile unit exposing runCycle) is
//     an implementation decision left to code-builder; this test only
//     requires that `#include <main.h>` resolves and `runCycle(Hal*)` is
//     linkable from this translation unit.
//
// This is the TDD Red phase: these tests are expected to fail to compile
// until include/main.h and runCycle() exist.

#include <Hal.h>
#include <TestHal.h>
#include <main.h>
#include <unity.h>

void setUp() {}
void tearDown() {}

// @req TECH-MAIN-001
void runCycle_does_not_crash() {
  // Given
  // TestHal is the singleton native HAL stub; USE_OREGON +
  // BATTERY_IS_VCC build flags select a code path with no AVR-only
  // hardware sensor reads.

  // When
  runCycle(&TestHal);

  // Then
  // Reaching this point without crashing confirms runCycle() was
  // successfully extracted and is callable as a standalone function.
  TEST_ASSERT_TRUE(true);
}

// @req TECH-MAIN-001
void runCycle_produces_radio_orders() {
  // Given
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  TEST_ASSERT_GREATER_THAN_UINT32(0, TestHal.Orders.size());
}

// @req TECH-MAIN-001
void runCycle_calls_hal_via_pointer() {
  // Given
  TestHal.ClearOrders();
  runCycle(&TestHal);
  size_t ordersAfterFirstCall = TestHal.Orders.size();

  // When
  runCycle(&TestHal);

  // Then
  TEST_ASSERT_GREATER_THAN_UINT32(ordersAfterFirstCall, TestHal.Orders.size());
}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(runCycle_does_not_crash);
  RUN_TEST(runCycle_produces_radio_orders);
  RUN_TEST(runCycle_calls_hal_via_pointer);
  return UNITY_END();
}
