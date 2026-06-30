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

// ---------------------------------------------------------------------------
// Helper: count occurrences of a token in the Orders vector
// ---------------------------------------------------------------------------
static int countToken(const std::vector<unsigned char> &orders,
                      unsigned char token) {
  int n = 0;
  for (unsigned char c : orders) {
    if (c == token)
      n++;
  }
  return n;
}

// Default rawInternal11ref that TestHal_ constructor initialises (1095).
// With INTERNAL_1v1=1100 this yields VCC = 1100*1024/1095 ≈ 1028 mV,
// which is above LOW_BATTERY_VOLTAGE=500 → normal (non-suppressed) path.
static const uint16_t DEFAULT_RAW_INTERNAL_11REF = 1095;

// A raw ADC value that drives VCC below LOW_BATTERY_VOLTAGE=500:
//   VCC = 1100 * 1024 / 2253 ≈ 499 mV
static const uint16_t LOW_BAT_RAW_INTERNAL_11REF = 2253;

// Token emitted by TestHal_::DelayX10PreambleHigh — produced on every
// x10rf::SendCommand call (and therefore on every RFXmeter call).
static const unsigned char TOK_X10_PREAMBLE_HIGH = 'Q';

void setUp() {
  TestHal.rawInternal11ref = DEFAULT_RAW_INTERNAL_11REF;
}
void tearDown() {
  TestHal.rawInternal11ref = DEFAULT_RAW_INTERNAL_11REF;
}

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

// ---------------------------------------------------------------------------
// FUNC-X10-006: Battery voltage RFXmeter transmission and low-battery
// suppression
// ---------------------------------------------------------------------------

// @req FUNC-X10-006
void runCycle_transmits_battery_voltage_via_x10_rfxmeter() {
  // Given
  // TestHal.rawInternal11ref = 1095 (default, restored by setUp).
  // VCC = 1100 * 1024 / 1095 ≈ 1028 mV, which is above LOW_BATTERY_VOLTAGE
  // (500 mV).  The BATTERY_VOLTAGE_X10_ID=0x01 and BATTERY_IS_VCC build
  // flags (from native_main_oregon) ensure runCycle reaches the RFXmeter
  // call path.  Any successful x10rf::RFXmeter invocation emits at least one
  // DelayX10PreambleHigh token ('Q') in TestHal.Orders.
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  int preambleCount = countToken(TestHal.Orders, TOK_X10_PREAMBLE_HIGH);
  TEST_ASSERT_GREATER_THAN_INT_MESSAGE(
      0, preambleCount,
      "runCycle must emit at least one X10 preamble-high token ('Q') when "
      "battery is not low, indicating RFXmeter was called");
}

// @req FUNC-X10-006
void runCycle_suppresses_battery_x10_when_low_battery() {
  // Given
  // Raise rawInternal11ref so ComputeVccMv returns < LOW_BATTERY_VOLTAGE.
  // With INTERNAL_1v1=1100 and raw=2253:
  //   VCC = 1100 * 1024 / 2253 ≈ 499 mV < LOW_BATTERY_VOLTAGE (500 mV).
  // setUp already restored the default; override it now.
  TestHal.rawInternal11ref = LOW_BAT_RAW_INTERNAL_11REF;
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  // The X10 RFXmeter call for the battery voltage must be suppressed: no
  // preamble-high ('Q') token should appear in Orders.
  int preambleCount = countToken(TestHal.Orders, TOK_X10_PREAMBLE_HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, preambleCount,
      "runCycle must NOT emit any X10 preamble-high token ('Q') when "
      "lowBattery is true (battery RFXmeter call must be suppressed)");
}

// ---------------------------------------------------------------------------
// FUNC-X10-007: Auxiliary analog voltage RFXmeter transmission and
// low-battery suppression
//
// These tests are only compiled when ANALOG1_X10_ID is defined (i.e. in the
// native_main_oregon_analog env).  In native_main_oregon (where ANALOG1_X10_ID
// is absent) the guard prevents compilation, so this suite stays green there.
//
// PRODUCTION-SIDE NOTE: The ANALOG1_X10_ID block in src/main.cpp uses `vccMv`
// declared inside the BATTERY_VOLTAGE_X10_ID block.  Both macros must be
// defined together; the native_main_oregon_analog env guarantees this.
// ---------------------------------------------------------------------------

#ifdef ANALOG1_X10_ID

// Expected number of DelayX10PreambleHigh ('Q') tokens per RFXmeter call:
// x10rf is constructed with rf_repeats=2, so SendCommand loops twice,
// emitting one 'Q' per iteration → 2 tokens per RFXmeter call.
// With BATTERY_VOLTAGE_X10_ID + ANALOG1_X10_ID both active (healthy battery):
//   battery RFXmeter → 2 'Q' tokens
//   analog  RFXmeter → 2 'Q' tokens
//   total             → 4 'Q' tokens
static const int EXPECTED_Q_PER_RFXMETER = 2;
static const int EXPECTED_Q_BOTH_ACTIVE = 2 * EXPECTED_Q_PER_RFXMETER;

// @req FUNC-X10-007
void runCycle_transmits_analog_voltage_via_x10_rfxmeter() {
  // Given
  // rawInternal11ref = 1095 (default, restored by setUp).
  // VCC = 1100 * 1024 / 1095 ≈ 1028 mV > LOW_BATTERY_VOLTAGE (500 mV) →
  // lowBattery = false.  Both BATTERY_VOLTAGE_X10_ID=0x01 and
  // ANALOG1_X10_ID=0x02 are defined (native_main_oregon_analog env), so
  // runCycle must call RFXmeter twice, emitting EXPECTED_Q_BOTH_ACTIVE 'Q'
  // tokens in TestHal.Orders.
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  int preambleCount = countToken(TestHal.Orders, TOK_X10_PREAMBLE_HIGH);
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(
      EXPECTED_Q_BOTH_ACTIVE, preambleCount,
      "runCycle must emit at least 4 X10 preamble-high tokens ('Q') when "
      "both BATTERY_VOLTAGE_X10_ID and ANALOG1_X10_ID are defined and "
      "battery is not low (2 per RFXmeter call × 2 calls)");
}

// @req FUNC-X10-007
void runCycle_suppresses_analog_x10_when_low_battery() {
  // Given
  // Raise rawInternal11ref so ComputeVccMv returns < LOW_BATTERY_VOLTAGE.
  // VCC = 1100 * 1024 / 2253 ≈ 499 mV < LOW_BATTERY_VOLTAGE (500 mV) →
  // lowBattery = true.  Both the battery and analog RFXmeter calls must be
  // suppressed.
  TestHal.rawInternal11ref = LOW_BAT_RAW_INTERNAL_11REF;
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  // Neither RFXmeter call should fire: zero 'Q' tokens expected.
  int preambleCount = countToken(TestHal.Orders, TOK_X10_PREAMBLE_HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, preambleCount,
      "runCycle must NOT emit any X10 preamble-high token ('Q') when "
      "lowBattery is true (both battery and analog RFXmeter calls must be "
      "suppressed)");
}

#endif  // ANALOG1_X10_ID

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(runCycle_does_not_crash);
  RUN_TEST(runCycle_produces_radio_orders);
  RUN_TEST(runCycle_calls_hal_via_pointer);
  RUN_TEST(runCycle_transmits_battery_voltage_via_x10_rfxmeter);
  RUN_TEST(runCycle_suppresses_battery_x10_when_low_battery);
#ifdef ANALOG1_X10_ID
  RUN_TEST(runCycle_transmits_analog_voltage_via_x10_rfxmeter);
  RUN_TEST(runCycle_suppresses_analog_x10_when_low_battery);
#endif
  return UNITY_END();
}
