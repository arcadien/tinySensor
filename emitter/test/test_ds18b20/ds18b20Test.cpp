/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019-2024 Aurélien Labrosse
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

/*
 * Integration tests for TECH-SENSOR-001.
 *
 * Production side must provide (code-builder handles this):
 *
 * In include/Ds18b20.h:
 *   class Ds18b20 {
 *    public:
 *     Ds18b20(Hal* hal);
 *     void    Begin();
 *     void    Convert();
 *     int16_t Read();
 *   };
 *
 * On non-AVR (native) targets all methods must be stubs:
 *   - Begin()   — no-op, must not call any Hal method
 *   - Convert() — no-op, must not call any Hal method
 *   - Read()    — returns 0
 *
 * On AVR targets (not tested here — native stub path only):
 *   - Convert() calls ds18b20convert() then hal->Delay1s() then ds18b20read()
 *     internally; main.cpp must never call those ds18b20 library functions
 *     or reference PORTA/DDRA/PINA directly.
 */

#if defined(NATIVE)

#include <stdint.h>
#include <unity.h>

#include <Ds18b20.h>
#include <TestHal.h>

// ---------------------------------------------------------------------------
// setUp / tearDown
// ---------------------------------------------------------------------------

void setUp(void) {
  TestHal.ClearOrders();
}

void tearDown(void) {}

// ---------------------------------------------------------------------------
// TECH-SENSOR-001 — Ds18b20 native stub contract
// ---------------------------------------------------------------------------

// @req TECH-SENSOR-001
void Ds18b20_constructs_without_error() {
  // Given
  // A valid Hal pointer (TestHal singleton)

  // When
  Ds18b20 sensor(&TestHal);

  // Then
  // No crash, no exception — construction succeeds silently
  TEST_PASS_MESSAGE("Ds18b20 must construct without error when given a Hal*");
}

// @req TECH-SENSOR-001
void Ds18b20_Begin_emits_no_hal_tokens() {
  // Given
  Ds18b20 sensor(&TestHal);
  TestHal.ClearOrders();

  // When
  sensor.Begin();

  // Then
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, (int)TestHal.Orders.size(),
      "Begin() must be a no-op on native: no HAL tokens must be emitted");
}

// @req TECH-SENSOR-001
void Ds18b20_Convert_emits_no_hal_tokens() {
  // Given
  Ds18b20 sensor(&TestHal);
  TestHal.ClearOrders();

  // When
  sensor.Convert();

  // Then
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, (int)TestHal.Orders.size(),
      "Convert() must be a no-op on native: no HAL tokens must be emitted");
}

// @req TECH-SENSOR-001
void Ds18b20_Read_returns_zero_on_native() {
  // Given
  Ds18b20 sensor(&TestHal);

  // When
  int16_t result = sensor.Read();

  // Then
  TEST_ASSERT_EQUAL_INT16_MESSAGE(
      0, result, "Read() must return 0 on native (stub return value)");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int, char **) {
  UnityBegin("ds18b20Test.cpp");

  RUN_TEST(Ds18b20_constructs_without_error);
  RUN_TEST(Ds18b20_Begin_emits_no_hal_tokens);
  RUN_TEST(Ds18b20_Convert_emits_no_hal_tokens);
  RUN_TEST(Ds18b20_Read_returns_zero_on_native);

  return UNITY_END();
}

#endif
