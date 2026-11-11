/*
 * Integration tests for FUNC-DECODE-008: FormatRflink renders the frame a
 * decoder just completed as one RFLink text line. Frames come from the
 * repository encoders through OrdersToPulses (TECH-DECODE-001).
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/RflinkFormatter/RflinkFormatter.h (<stdint.h> and the
 * decoder headers only, no printf, no dynamic allocation):
 *   const uint8_t RflinkLineSize = 64;
 *   uint8_t FormatRflink(const X10Decoder &, uint8_t counter,
 *                        char line[RflinkLineSize]);
 *   uint8_t FormatRflink(const LacrosseDecoder &, uint8_t counter,
 *                        char line[RflinkLineSize]);
 *   uint8_t FormatRflink(const OregonDecoder &, uint8_t counter,
 *                        char line[RflinkLineSize]);
 * Each writes the line plus '\0' (no "\r\n") and returns its length.
 */

#include <stdint.h>
#include <string.h>
#include <unity.h>

#include <LacrosseDecoder.h>
#include <LacrosseWS7000.h>
#include <Loopback.h>
#include <OregonDecoder.h>
#include <Oregon_v3.h>
#include <RflinkFormatter.h>
#include <TestHal.h>
#include <X10Decoder.h>
#include <vector>
#include <x10rf.h>

// Feeds the pulses until the decoder completes a frame
template <class Decoder>
static Decoder decoded(const std::vector<Pulse> &pulses) {
  Decoder decoder;
  for (const Pulse &p : pulses) {
    if (decoder.Feed(p.high, p.durationUs))
      return decoder;
  }
  TEST_FAIL_MESSAGE("loopback frame not decoded");
  return decoder;
}

enum Field : uint8_t { TEMP = 1, HUM = 2, PRES = 4, LUX = 8 };

static OregonDecoder oregon(uint8_t channel, uint8_t rollingCode,
                            bool batteryLow, uint8_t fields, float temperature,
                            uint8_t humidity, uint16_t pressure) {
  OregonV3 encoder(&TestHal);
  encoder.SetChannel(channel);
  encoder.SetRollingCode(rollingCode);
  encoder.SetBatteryLow(batteryLow);
  if (fields & TEMP)
    encoder.SetTemperature(temperature);
  if (fields & HUM)
    encoder.SetHumidity(humidity);
  if (fields & PRES)
    encoder.SetPressure(pressure);
  TestHal.ClearOrders();
  encoder.Send();
  return decoded<OregonDecoder>(OrdersToPulses(TestHal.Orders));
}

static LacrosseDecoder lacrosse(uint8_t address, uint8_t fields,
                                float temperature, float humidity,
                                float pressure, uint16_t lux) {
  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(static_cast<LacrosseWS7000::Address>(address));
  if (fields & TEMP)
    encoder.SetTemperature(temperature);
  if (fields & HUM)
    encoder.SetHumidity(humidity);
  if (fields & PRES)
    encoder.SetPressure(pressure);
  if (fields & LUX)
    encoder.SetLuminosity(lux);
  TestHal.ClearOrders();
  encoder.Send();
  return decoded<LacrosseDecoder>(OrdersToPulses(TestHal.Orders));
}

static X10Decoder x10(uint8_t address, uint32_t value) {
  x10rf encoder(&TestHal, 1);
  TestHal.ClearOrders();
  encoder.RFXmeter(address, 0, value);
  return decoded<X10Decoder>(OrdersToPulses(TestHal.Orders));
}

struct FormatCase {
  const char *id;
  // Decodes the frame then calls the matching FormatRflink overload
  uint8_t (*format)(uint8_t counter, char *line);
  uint8_t counter;
  const char *line;
};

static const FormatCase formatCases[] = {
    {"oregon-thgn123n",
     [](uint8_t n, char *l) {
       return FormatRflink(oregon(2, 0xA5, false, TEMP | HUM, 23.7f, 45, 0), n,
                           l);
     },
     0x0A, "20;0A;Oregon TempHygro;ID=A502;TEMP=00ed;HUM=45;BAT=OK;"},
    {"oregon-thn132n-negative-battery-low",
     [](uint8_t n, char *l) {
       return FormatRflink(oregon(1, 0x3C, true, TEMP, -10.2f, 0, 0), n, l);
     },
     0x00, "20;00;Oregon Temp;ID=3C01;TEMP=8066;BAT=LOW;"},
    {"oregon-bthr918",
     [](uint8_t n, char *l) {
       return FormatRflink(
           oregon(3, 0x7E, true, TEMP | HUM | PRES, 21.5f, 60, 1013), n, l);
     },
     0xFF, "20;FF;Oregon BTHR;ID=7E03;TEMP=00d7;HUM=60;BARO=03f5;BAT=LOW;"},
    {"lacrosse-type4-rounded",
     [](uint8_t n, char *l) {
       return FormatRflink(
           lacrosse(3, TEMP | HUM | PRES, -10.2f, 42.5f, 1013.4f, 0), n, l);
     },
     0x01, "20;01;LaCrosse WS7000;ID=0403;TEMP=8066;HUM=43;BARO=03f5;"},
    {"lacrosse-type5-lux",
     [](uint8_t n, char *l) {
       return FormatRflink(lacrosse(2, LUX, 0, 0, 0, 1000), n, l);
     },
     0x01, "20;01;LaCrosse WS7000;ID=0502;LUX=03e8;"},
    {"x10-bcd-meter",
     [](uint8_t n, char *l) { return FormatRflink(x10(10, 0x2950), n, l); },
     0x02, "20;02;X10Meter;ID=000A;METER=2950;"},
    {"x10-not-bcd-no-meter",
     [](uint8_t n, char *l) { return FormatRflink(x10(10, 0x2A50), n, l); },
     0x03, "20;03;X10Meter;ID=000A;"},
};

// @req FUNC-DECODE-008
void FormatRflink_renders_one_rflink_line_per_decoded_frame() {
  for (const FormatCase &c : formatCases) {
    // Given
    char line[RflinkLineSize];
    memset(line, '#', sizeof line);  // dirty, so a missing '\0' shows up

    // When
    uint8_t length = c.format(c.counter, line);

    // Then
    TEST_ASSERT_EQUAL_STRING_MESSAGE(c.line, line, c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(strlen(c.line), length, c.id);
  }
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(FormatRflink_renders_one_rflink_line_per_decoded_frame);
  return UNITY_END();
}
