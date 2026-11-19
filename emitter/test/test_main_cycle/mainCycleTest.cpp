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
#include <algorithm>
#include <unity.h>
#include <vector>

#include <EnvironmentEncoder.h>
#include <Measurements.h>
#include <x10rf.h>

// An environment frame needs a protocol and a temperature. On native builds
// only the BMx280 stub yields one: the Ds18b20 stub reads 0 (= read error).
#if (defined(USE_OREGON) || defined(USE_LACROSSE)) && \
    (defined(USE_BMP280) || defined(USE_BME280))
#define EXPECT_ENVIRONMENT_FRAME
#define ENVIRONMENT_FRAMES 2  // FUNC-MAIN-001: environment frame sent twice
// FUNC-MAIN-001: Delay30ms added between the two copies, on top of the
// transmission's own trailing Delay30ms (9 x 30 = 270 ms of silence).
#define ENVIRONMENT_GAP_EXTRA_30MS 8
#if defined(USE_OREGON)
#define ENVIRONMENT_MIN_GAP_US 246000UL  // one Oregon frame
#else
#define ENVIRONMENT_MIN_GAP_US 98000UL  // one Lacrosse frame
#endif
#endif

#if defined(USE_BH1750)
#define LIGHT_FRAMES 2  // luminosity frame is sent twice
#endif

// Recorded by TestHal_::Init() (TECH-HAL-006).
static const unsigned char INIT = 'I';

// Extra tokens, on top of TestHal_'s radio/delay tokens.
static const unsigned char POWER_ON = '+';
static const unsigned char POWER_OFF = '-';
static const unsigned char HIBERNATE = 'Z';
static const unsigned char LED_ON = 'O';
static const unsigned char LED_OFF = 'o';
static const unsigned char DELAY_30MS = 'M';

// Plausible ADC readings: VCC ~3 V, battery ~2.3 V, both above
// LOW_BATTERY_VOLTAGE so every frame of the variant is sent.
static void SetNominalReadings(TestHal_ &hal) {
  hal.rawInternal11ref = 375;
  hal.rawBattery = 800;
  hal.rawAnalogSensor = 512;
}

// Also records power, LED, Delay30ms and hibernate calls in Orders, so the
// cycle order is visible; Delay30ms and Hibernate do not sleep.
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
  void Delay30ms() override { Orders.push_back(DELAY_30MS); }
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
#if defined(USE_BH1750)
  frames += LIGHT_FRAMES;
#endif
#if defined(EXPECT_ENVIRONMENT_FRAME)
  frames += ENVIRONMENT_FRAMES;
#endif
  return frames;
}

// Appends one TRANSMIT_WITH_LED transmission recorded on `ref` (which
// includes the encoder's own LED calls, e.g. x10rf::SendCommand).
static void AppendFrame(std::vector<unsigned char> &expected,
                        CycleSpyHal &ref) {
  expected.push_back(LED_ON);
  expected.insert(expected.end(), ref.Orders.begin(), ref.Orders.end());
  expected.push_back(LED_OFF);
  expected.push_back(DELAY_30MS);
  ref.ClearOrders();
}

// One TRANSMIT_WITH_LED ends with LedOff then Delay30ms; the BMx280 settle
// delay and x10rf's inner LedOff do not match this pair.
static int CountTransmissions(const std::vector<unsigned char> &orders) {
  int n = 0;
  for (size_t i = 1; i < orders.size(); i++) {
    n += (orders[i - 1] == LED_OFF && orders[i] == DELAY_30MS);
  }
  return n;
}

#if defined(EXPECT_ENVIRONMENT_FRAME)
// Orders of one environment encoder Send(), without the LED/delay wrapper.
static std::vector<unsigned char> EnvironmentFrameOrders(
    bool lowBattery = false) {
  CycleSpyHal ref;
  Measurements m;
  m.SetTemperature(0);  // native BMx280 stub values
#if defined(USE_BME280)
  m.SetHumidity(0);
#if !defined(OREGON_NO_PRESSURE)
  m.SetPressure(0);  // CONF-BUILD-013: without pressure -> THGR228N frame
#endif
#endif
  EnvironmentEncoder environment(&ref);
  ConfigureEncoder(environment);
  ApplyMeasurements(environment, m, lowBattery);
  environment.Send();
  return ref.Orders;
}

// Duration of a recorded token in microseconds; non-delay tokens take 0.
static unsigned long TokenDurationUs(unsigned char token) {
  switch (token) {
    case DELAY_30MS:
      return 30000;
    case 'D':
      return 512;
    case 'P':
      return 1024;
    case 'A':
      return 400;
    default:
      return 0;
  }
}
#endif

// HAL sequence of one iteration of the former while(1) loop, rebuilt with
// direct encoder calls on a separate HAL.
static std::vector<unsigned char> ReferenceCycleOrders() {
  CycleSpyHal ref;
  std::vector<unsigned char> expected;
  expected.push_back(INIT);  // TECH-HAL-006
  expected.push_back(POWER_ON);
#if defined(USE_BMP280) || defined(USE_BME280)
  expected.push_back(DELAY_30MS);  // BMx280 settle delay in ReadEnvironment
#endif

  x10rf x10(&ref, 2);
  uint16_t vccMv = ref.ComputeVccMv(INTERNAL_1v1);
#if defined(BATTERY_IS_VCC)
  uint16_t batteryMv = vccMv;
#else
  uint16_t batteryMv = ref.ConvertAnalogValueToMv(ref.GetRawBattery(), vccMv);
#if defined(BATTERY_DIVIDER)
  batteryMv *= BATTERY_DIVIDER;  // FUNC-BATTERY-002: PA1 reads battery / ratio
#endif
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

#if defined(USE_BH1750)
  LacrosseWS7000 light(&ref);
  light.SetAddress(LACROSSE_ID);
  light.SetLuminosity(0);  // native LightSensor stub value
  for (int i = 0; i < LIGHT_FRAMES; i++) {
    light.Send();
    AppendFrame(expected, ref);
  }
#endif

#if defined(EXPECT_ENVIRONMENT_FRAME)
  std::vector<unsigned char> frame = EnvironmentFrameOrders();
  for (int i = 0; i < ENVIRONMENT_FRAMES; i++) {
    if (i > 0) {
      expected.insert(expected.end(), ENVIRONMENT_GAP_EXTRA_30MS, DELAY_30MS);
    }
    ref.Orders = frame;
    AppendFrame(expected, ref);
  }
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
// @req CONF-BUILD-006 (hibernates SLEEP_TIME_IN_SECONDS)
// @req PLAT-POWER-002 (sensor power on first, off before hibernate)
void Expect_cycle_powers_sensors_first_then_powers_off_and_hibernates_last() {
  // Given
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  const std::vector<unsigned char> &orders = spy.Orders;
  TEST_ASSERT_TRUE(orders.size() >= 4);
  TEST_ASSERT_EQUAL_CHAR(POWER_ON, orders[1]);  // orders[0] is Init()
  TEST_ASSERT_EQUAL_CHAR(POWER_OFF, orders[orders.size() - 2]);
  TEST_ASSERT_EQUAL_CHAR(HIBERNATE, orders.back());
  TEST_ASSERT_EQUAL_INT(1, Count(orders, POWER_ON));
  TEST_ASSERT_EQUAL_INT(1, Count(orders, POWER_OFF));
  TEST_ASSERT_EQUAL_INT(1, spy.hibernateCalls);
  TEST_ASSERT_EQUAL_UINT16(SLEEP_TIME_IN_SECONDS, spy.hibernateSeconds);
  TEST_ASSERT_FALSE(spy.SensorIsPowered);
}

// @req TECH-HAL-006
void Expect_cycle_initialises_hal_once_before_any_other_hal_call() {
  // Given
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  TEST_ASSERT_TRUE(spy.Orders.size() > 0);
  TEST_ASSERT_EQUAL_CHAR(INIT, spy.Orders.front());
  TEST_ASSERT_EQUAL_INT(1, Count(spy.Orders, INIT));
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
// @req CONF-BUILD-001 (native_cycle_x10: no protocol, no environment frame)
// @req CONF-BUILD-002 (no temperature sensor: no environment frame)
// @req FUNC-MAIN-001 (environment frame sent twice)
void Expect_cycle_sends_one_frame_per_enabled_output_of_the_variant() {
  // Given
  CycleSpyHal spy;

  // When
  runCycle(&spy);

  // Then
  TEST_ASSERT_EQUAL_INT(ExpectedFrameCount(), CountTransmissions(spy.Orders));
}

// @req TECH-MAIN-001
// @req CONF-BUILD-002 (BMP280: temperature only; BME280: + humidity,
// pressure)
// @req FUNC-BATTERY-002 (battery meter = PA1 voltage x divider ratio)
// @req CONF-BUILD-012 (native_cycle_oregon_divider: BATTERY_DIVIDER=2)
// @req CONF-BUILD-013 (OREGON_NO_PRESSURE: THGR228N temperature+humidity)
// @req FUNC-MAIN-001 (environment frame sent twice)
// @req FUNC-X10-006 (battery meter: BCD mV at BATTERY_VOLTAGE_X10_ID)
// @req FUNC-X10-007 (analog meter: BCD mV at ANALOG1_X10_ID)
// @req FUNC-ANALOG-001 (PA0 reading converted to mV against VCC)
// @req CONF-BUILD-005 (VCC from INTERNAL_1v1)
// @req CONF-BUILD-008 (battery meter address)
// @req CONF-BUILD-009 (analog meter only with ANALOG1_X10_ID)
// @req CONF-BUILD-010 (native_cycle_oregon_vcc: battery = VCC)
// @req PLAT-POWER-004 (every frame: LED on, frame, LED off, 30 ms)
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

#if defined(EXPECT_ENVIRONMENT_FRAME)
// @req FUNC-MAIN-001
void Expect_environment_frame_sent_twice_with_a_silence_one_frame_long() {
  // Given
  CycleSpyHal spy;
  std::vector<unsigned char> frame = EnvironmentFrameOrders();

  // When
  runCycle(&spy);

  // Then
  const std::vector<unsigned char> &o = spy.Orders;
  auto first = std::search(o.begin(), o.end(), frame.begin(), frame.end());
  TEST_ASSERT_TRUE_MESSAGE(first != o.end(), "No environment frame");
  auto gapStart = first + frame.size();
  auto second = std::search(gapStart, o.end(), frame.begin(), frame.end());
  TEST_ASSERT_TRUE_MESSAGE(second != o.end(), "Only one environment frame");
  unsigned long gapUs = 0;
  for (auto it = gapStart; it != second; ++it) {
    TEST_ASSERT_NOT_EQUAL_MESSAGE('H', *it, "Radio high between the copies");
    gapUs += TokenDurationUs(*it);
  }
  TEST_ASSERT_TRUE_MESSAGE(gapUs >= ENVIRONMENT_MIN_GAP_US,
                           "Silence shorter than one frame");
}
#endif

// @req CONF-BUILD-002
void Expect_i2c_derived_when_an_i2c_sensor_is_selected() {
  // Given
#if defined(USE_BMP280) || defined(USE_BME280) || defined(USE_BH1750)
  bool needsI2c = true;
#else
  bool needsI2c = false;
#endif

  // When (USE_I2C is derived by config.h, included above)
#if defined(USE_I2C)
  bool hasI2c = true;
#else
  bool hasI2c = false;
#endif

  // Then
  TEST_ASSERT_TRUE(!needsI2c || hasI2c);
}

// @req FUNC-BATTERY-003
void Expect_low_battery_cycle_sends_only_the_environment_frame_with_its_flag() {
  // Given: VCC ~1.1 V and no battery reading, below LOW_BATTERY_VOLTAGE on
  // every variant (battery = VCC or divided PA1 reading)
  CycleSpyHal spy;
  spy.rawInternal11ref = 1023;
  spy.rawBattery = 0;

  // When
  runCycle(&spy);

  // Then: no X10 meter frame, no luminosity frame, only the environment
  // frame (with the battery-low flag on Oregon; Lacrosse has none)
#if defined(EXPECT_ENVIRONMENT_FRAME)
  TEST_ASSERT_EQUAL_INT(ENVIRONMENT_FRAMES, CountTransmissions(spy.Orders));
  std::vector<unsigned char> frame = EnvironmentFrameOrders(true);
  TEST_ASSERT_TRUE(std::search(spy.Orders.begin(), spy.Orders.end(),
                               frame.begin(), frame.end()) != spy.Orders.end());
#if defined(USE_OREGON)
  TEST_ASSERT_FALSE(frame == EnvironmentFrameOrders(false));
#endif
#else
  TEST_ASSERT_EQUAL_INT(0, CountTransmissions(spy.Orders));
#endif
}

void setUp() {}
void tearDown() {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(
      Expect_cycle_powers_sensors_first_then_powers_off_and_hibernates_last);
  RUN_TEST(Expect_cycle_initialises_hal_once_before_any_other_hal_call);
  RUN_TEST(Expect_cycle_drives_io_through_the_injected_hal_only);
  RUN_TEST(Expect_cycle_on_TestHal_records_orders_on_TestHal);
  RUN_TEST(Expect_cycle_sends_one_frame_per_enabled_output_of_the_variant);
  RUN_TEST(Expect_cycle_hal_sequence_unchanged_from_former_main_loop);
#if defined(EXPECT_ENVIRONMENT_FRAME)
  RUN_TEST(Expect_environment_frame_sent_twice_with_a_silence_one_frame_long);
#endif
  RUN_TEST(Expect_i2c_derived_when_an_i2c_sensor_is_selected);
  RUN_TEST(
      Expect_low_battery_cycle_sends_only_the_environment_frame_with_its_flag);
  return UNITY_END();
}

#endif
