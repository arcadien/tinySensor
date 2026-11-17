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
  // whole values: the rounding is checked by the FUNC-OREGON-007/008 tables
  m.SetHumidity(48.0f);
  m.SetPressure(1013.0f);

  OregonV3 reference(&TestHal);
  reference.SetChannel(1);
  reference.SetRollingCode(0x20);
  reference.SetTemperature(21.5f);
  reference.SetHumidity(48);
  reference.SetPressure(1013);
  reference.SetBatteryLow(false);

  OregonV3 encoder(&TestHal);
  encoder.SetChannel(1);
  encoder.SetRollingCode(0x20);
  TEST_ASSERT_TRUE(ApplyMeasurements(encoder, m, false));

  AssertSameOrders(SendAndCapture(reference), SendAndCapture(encoder));
}

/*
 * The sensors give float humidity and pressure; ApplyMeasurements() must send
 * them rounded to the Oregon frame resolution (whole % and whole hPa).
 */
struct OregonBytesCase {
  const char *id;
  float value;
  uint8_t bytes[2];
};

// bytes 6-7: humidity units << 4 (low nibble: sign, clear here), tens
static const OregonBytesCase oregonHumidityCases[] = {
    {"67.4-rounds-down", 67.4f, {0x70, 0x06}},
    {"67.5-half-rounds-up", 67.5f, {0x80, 0x06}},
    {"9.5-rounds-up-to-10", 9.5f, {0x00, 0x01}},
    {"99.6-clamped-to-99", 99.6f, {0x90, 0x09}},
    {"100-clamped-to-99", 100.0f, {0x90, 0x09}},
    {"-3-clamped-to-0", -3.0f, {0x00, 0x00}},
};

// @req FUNC-OREGON-007
void Expect_oregon_humidity_rounded_to_whole_percent() {
  for (const OregonBytesCase &c : oregonHumidityCases) {
    // Given
    Measurements m;
    m.SetHumidity(c.value);
    OregonV3 encoder(&TestHal);

    // When
    ApplyMeasurements(encoder, m, false);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(c.bytes, encoder.GetMessage() + 6, 2,
                                         c.id);
  }
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
// @req CONF-BUILD-002
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
  RUN_TEST(Expect_oregon_humidity_rounded_to_whole_percent);
  RUN_TEST(Expect_oregon_battery_low_flag_applied_with_temperature);
  RUN_TEST(Expect_no_environment_frame_without_temperature);
  RUN_TEST(Expect_lacrosse_frame_unchanged_for_full_measurements);
  RUN_TEST(Expect_lacrosse_temperature_only_frame_unchanged);
  RUN_TEST(Expect_measurements_track_present_fields);
  RUN_TEST(Expect_light_sensor_stub_returns_configured_lux);

  return UNITY_END();
}
#endif
