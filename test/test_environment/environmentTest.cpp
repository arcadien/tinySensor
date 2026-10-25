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

#include <EnvironmentEncoder.h>
#include <LightSensor.h>
#include <Measurements.h>
#include <TestHal.h>
#include <unity.h>
#include <vector>

/*
 * ApplyMeasurements() must drive the encoders exactly as main.cpp did before
 * the refactor. Each test builds a reference encoder with the former direct
 * calls, sends both frames and compares the recorded HAL orders.
 */

static std::vector<unsigned char> SendAndCapture(OregonV3 &encoder) {
  TestHal.ClearOrders();
  encoder.Send();
  return TestHal.Orders;
}
static std::vector<unsigned char> SendAndCapture(LacrosseWS7000 &encoder) {
  TestHal.ClearOrders();
  encoder.Send();
  return TestHal.Orders;
}

static void AssertSameOrders(const std::vector<unsigned char> &expected,
                             const std::vector<unsigned char> &actual) {
  TEST_ASSERT_EQUAL_MESSAGE(expected.size(), actual.size(),
                            "Frames differ in length");
  TEST_ASSERT_EQUAL_CHAR_ARRAY(expected.data(), actual.data(), expected.size());
}

// @req FUNC-OREGON-012
void Expect_oregon_frame_unchanged_for_full_measurements() {
  Measurements m;
  m.SetTemperature(21.5f);
  m.SetHumidity(48.7f);
  m.SetPressure(1013.4f);

  OregonV3 reference(&TestHal);
  reference.SetChannel(1);
  reference.SetRollingCode(0x20);
  reference.SetTemperature(21.5f);
  reference.SetHumidity(48.7f);  // former implicit float -> uint8_t
  reference.SetPressure(1013.4f);
  reference.SetBatteryLow(false);

  OregonV3 encoder(&TestHal);
  encoder.SetChannel(1);
  encoder.SetRollingCode(0x20);
  TEST_ASSERT_TRUE(ApplyMeasurements(encoder, m, false));

  AssertSameOrders(SendAndCapture(reference), SendAndCapture(encoder));
}

// @req FUNC-BATTERY-003
void Expect_oregon_battery_low_flag_applied_with_temperature() {
  Measurements m;
  m.SetTemperature(-3.2f);

  OregonV3 reference(&TestHal);
  reference.SetTemperature(-3.2f);
  reference.SetBatteryLow(true);

  OregonV3 encoder(&TestHal);
  TEST_ASSERT_TRUE(ApplyMeasurements(encoder, m, true));

  AssertSameOrders(SendAndCapture(reference), SendAndCapture(encoder));
}

// @req FUNC-OREGON-012
void Expect_no_environment_frame_without_temperature() {
  Measurements m;
  m.SetHumidity(50.0f);

  OregonV3 oregon(&TestHal);
  LacrosseWS7000 lacrosse(&TestHal);
  TEST_ASSERT_FALSE(ApplyMeasurements(oregon, m, false));
  TEST_ASSERT_FALSE(ApplyMeasurements(lacrosse, m, false));
}

// @req FUNC-LACROSSE-002
void Expect_lacrosse_frame_unchanged_for_full_measurements() {
  Measurements m;
  m.SetTemperature(18.3f);
  m.SetHumidity(62.1f);
  m.SetPressure(998.6f);

  LacrosseWS7000 reference(&TestHal);
  reference.SetAddress(LacrosseWS7000::Address::THREE);
  reference.SetTemperature(18.3f);
  reference.SetHumidity(62.1f);
  reference.SetPressure(998.6f);

  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(LacrosseWS7000::Address::THREE);
  TEST_ASSERT_TRUE(ApplyMeasurements(encoder, m, true));

  AssertSameOrders(SendAndCapture(reference), SendAndCapture(encoder));
}

// @req FUNC-LACROSSE-002
void Expect_lacrosse_temperature_only_frame_unchanged() {
  Measurements m;
  m.SetTemperature(5.0f);

  LacrosseWS7000 reference(&TestHal);
  reference.SetAddress(LacrosseWS7000::Address::ONE);
  reference.SetTemperature(5.0f);

  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(LacrosseWS7000::Address::ONE);
  TEST_ASSERT_TRUE(ApplyMeasurements(encoder, m, false));

  AssertSameOrders(SendAndCapture(reference), SendAndCapture(encoder));
}

// @req FUNC-SENSOR-003
void Expect_measurements_track_present_fields() {
  Measurements m;
  TEST_ASSERT_FALSE(m.Has(Measurements::TEMPERATURE));
  TEST_ASSERT_FALSE(m.Has(Measurements::LUMINOSITY));

  m.SetLuminosity(1234);
  TEST_ASSERT_TRUE(m.Has(Measurements::LUMINOSITY));
  TEST_ASSERT_FALSE(m.Has(Measurements::TEMPERATURE));
  TEST_ASSERT_EQUAL_UINT16(1234, m.luminosity);
}

// @req FUNC-SENSOR-003
void Expect_light_sensor_stub_returns_configured_lux() {
  LightSensor sensor(&TestHal);
  sensor.stubLux = 742;
  TEST_ASSERT_EQUAL_UINT16(742, sensor.Read());
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UnityBegin("environmentTest.cpp");

  RUN_TEST(Expect_oregon_frame_unchanged_for_full_measurements);
  RUN_TEST(Expect_oregon_battery_low_flag_applied_with_temperature);
  RUN_TEST(Expect_no_environment_frame_without_temperature);
  RUN_TEST(Expect_lacrosse_frame_unchanged_for_full_measurements);
  RUN_TEST(Expect_lacrosse_temperature_only_frame_unchanged);
  RUN_TEST(Expect_measurements_track_present_fields);
  RUN_TEST(Expect_light_sensor_stub_returns_configured_lux);

  return UNITY_END();
}
#endif
