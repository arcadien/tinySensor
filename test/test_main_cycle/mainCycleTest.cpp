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
#if defined(NATIVE)

/*
 * TECH-MAIN-001: one cycle is `void runCycle(Hal *hal)`.
 * Runs only in the native_cycle_* envs (one per build variant family); the
 * #ifdef blocks below mirror each env's build_flags.
 *
 * Production side must provide (not created here):
 *   - include/main.h declaring `void runCycle(Hal *hal);`
 *   - src/main.cpp defining runCycle, with main() compiled out under UNIT_TEST
 */

#include <TestHal.h>
#include <config.h>
#include <conversionTools.h>
#include <main.h>
#include <unity.h>
#include <vector>

#include <EnvironmentEncoder.h>
#include <Measurements.h>
#include <x10rf.h>

// Extra tokens, on top of TestHal_'s radio/delay tokens.
static const unsigned char POWER_ON = '+';
static const unsigned char POWER_OFF = '-';
static const unsigned char HIBERNATE = 'Z';
static const unsigned char LED_ON = 'O';
static const unsigned char LED_OFF = 'o';

// Plausible ADC readings: VCC ~3 V, battery ~2.3 V, both above
// LOW_BATTERY_VOLTAGE so every frame of the variant is sent.
static void SetNominalReadings(TestHal_ &hal) {
  hal.rawInternal11ref = 375;
  hal.rawBattery = 800;
  hal.rawAnalogSensor = 512;
}

// Also records power, LED and hibernate calls in Orders, so the cycle order
// is visible; Hibernate does not sleep.
class CycleSpyHal : public TestHal_ {
 public:
  int hibernateCalls = 0;
  uint16_t hibernateSeconds = 0xFFFF;

  CycleSpyHal() { SetNominalReadings(*this); }

  void PowerOnSensors() override {
    TestHal_::PowerOnSensors();
    Orders.push_back(POWER_ON);
  }
  void PowerOffSensors() override {
    TestHal_::PowerOffSensors();
    Orders.push_back(POWER_OFF);
  }
  void LedOn() override {
    TestHal_::LedOn();
    Orders.push_back(LED_ON);
  }
  void LedOff() override {
    TestHal_::LedOff();
    Orders.push_back(LED_OFF);
  }
  void Hibernate(uint16_t seconds) override {
    hibernateCalls++;
    hibernateSeconds = seconds;
    Orders.push_back(HIBERNATE);
  }
};

// Frames one cycle must send for this build variant, in order.
static int ExpectedFrameCount() {
  int frames = 1;  // BATTERY_VOLTAGE_X10_ID is set in every native_cycle env
#if defined(ANALOG1_X10_ID)
  frames++;
#endif
#if defined(USE_OREGON) || defined(USE_LACROSSE)
  frames++;  // native BMx280 stub reports a temperature
#endif
  return frames;
}

// Appends one LED-wrapped transmission recorded on `ref`.
static void AppendFrame(std::vector<unsigned char> &expected, TestHal_ &ref) {
  expected.push_back(LED_ON);
  expected.insert(expected.end(), ref.Orders.begin(), ref.Orders.end());
  expected.push_back(LED_OFF);
  ref.ClearOrders();
}

// HAL sequence of one iteration of the former while(1) loop, rebuilt with
// direct encoder calls on a separate HAL.
static std::vector<unsigned char> ReferenceCycleOrders() {
  TestHal_ ref;
  SetNominalReadings(ref);
  std::vector<unsigned char> expected;
  expected.push_back(POWER_ON);

  x10rf x10(&ref, 2);
  uint16_t vccMv = ref.ComputeVccMv(INTERNAL_1v1);
#if defined(BATTERY_IS_VCC)
  uint16_t batteryMv = vccMv;
#else
  uint16_t batteryMv = ref.ConvertAnalogValueToMv(ref.GetRawBattery(), vccMv);
#endif
  x10.RFXmeter(BATTERY_VOLTAGE_X10_ID, 0x00,
               ConversionTools::dec16ToHex(batteryMv));
  AppendFrame(expected, ref);

#if defined(ANALOG1_X10_ID)
  uint16_t analogMv =
      ref.ConvertAnalogValueToMv(ref.GetRawAnalogSensor(), vccMv);
  x10.RFXmeter(ANALOG1_X10_ID, 0x00, ConversionTools::dec16ToHex(analogMv));
  AppendFrame(expected, ref);
#endif

#if defined(USE_OREGON) || defined(USE_LACROSSE)
  Measurements m;
  m.SetTemperature(0);  // native BMx280 stub values
#if defined(USE_BME280)
  m.SetHumidity(0);
  m.SetPressure(0);
#endif
  EnvironmentEncoder environment(&ref);
  ConfigureEncoder(environment);
  ApplyMeasurements(environment, m, false);
  environment.Send();
  AppendFrame(expected, ref);
#endif

  expected.push_back(POWER_OFF);
  expected.push_back(HIBERNATE);
  return expected;
}

static int Count(const std::vector<unsigned char> &orders, unsigned char c) {
  int n = 0;
  for (unsigned char o : orders) {
    n += (o == c);
  }
  return n;
}

// @req TECH-MAIN-001
void Expect_cycle_powers_sensors_first_then_powers_off_and_hibernates_last() {
  // Given
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  const std::vector<unsigned char> &orders = spy.Orders;
  TEST_ASSERT_TRUE(orders.size() >= 3);
  TEST_ASSERT_EQUAL_CHAR(POWER_ON, orders.front());
  TEST_ASSERT_EQUAL_CHAR(POWER_OFF, orders[orders.size() - 2]);
  TEST_ASSERT_EQUAL_CHAR(HIBERNATE, orders.back());
  TEST_ASSERT_EQUAL_INT(1, Count(orders, POWER_ON));
  TEST_ASSERT_EQUAL_INT(1, Count(orders, POWER_OFF));
  TEST_ASSERT_EQUAL_INT(1, spy.hibernateCalls);
  TEST_ASSERT_EQUAL_UINT16(SLEEP_TIME_IN_SECONDS, spy.hibernateSeconds);
  TEST_ASSERT_FALSE(spy.SensorIsPowered);
}

// @req TECH-MAIN-001
void Expect_cycle_drives_io_through_the_injected_hal_only() {
  // Given
  TestHal.ClearOrders();
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  TEST_ASSERT_TRUE(spy.Orders.size() > 3);
  TEST_ASSERT_EQUAL_size_t(0, TestHal.Orders.size());
}

// @req TECH-MAIN-001
void Expect_cycle_on_TestHal_records_orders_on_TestHal() {
  // Given
  SetNominalReadings(TestHal);
  TestHal.ClearOrders();

  // When
  runCycle(&TestHal);

  // Then
  TEST_ASSERT_TRUE(TestHal.Orders.size() > 0);
  TEST_ASSERT_FALSE(TestHal.SensorIsPowered);
  TEST_ASSERT_FALSE(TestHal.IsLedOn);
}

// @req TECH-MAIN-001
void Expect_cycle_sends_one_frame_per_enabled_output_of_the_variant() {
  // Given
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  TEST_ASSERT_EQUAL_INT(ExpectedFrameCount(), Count(spy.Orders, LED_ON));
}

// @req TECH-MAIN-001
void Expect_cycle_hal_sequence_unchanged_from_former_main_loop() {
  // Given
  CycleSpyHal spy;
  std::vector<unsigned char> expected = ReferenceCycleOrders();

  // When
  runCycle(&spy);

  // Then
  TEST_ASSERT_EQUAL_size_t_MESSAGE(expected.size(), spy.Orders.size(),
                                   "Cycle sequence differs in length");
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected.data(), spy.Orders.data(),
                               expected.size());
}

void setUp() {}
void tearDown() {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(
      Expect_cycle_powers_sensors_first_then_powers_off_and_hibernates_last);
  RUN_TEST(Expect_cycle_drives_io_through_the_injected_hal_only);
  RUN_TEST(Expect_cycle_on_TestHal_records_orders_on_TestHal);
  RUN_TEST(Expect_cycle_sends_one_frame_per_enabled_output_of_the_variant);
  RUN_TEST(Expect_cycle_hal_sequence_unchanged_from_former_main_loop);
  return UNITY_END();
}

#endif
