/*
 * Integration tests for FUNC-DECODE-006: SourceTable keeps the last decoded
 * record of each radio source (kind, id, code), fed with loopback frames of the
 * repository encoders through the three decoders.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/SourceTable/SourceTable.h (<stdint.h> and the three
 * decoder headers only, no dynamic allocation):
 *   struct SourceRecord {
 *     enum Kind : uint8_t { Oregon, Lacrosse, LacrosseLux, X10 };
 *     enum : uint8_t { Temperature = 1, Humidity = 2, Pressure = 4, Lux = 8,
 *                      Millivolts = 16 };  // bits of `fields`
 *     Kind kind; uint8_t id; uint8_t code; uint8_t fields;
 *     int16_t temperatureTenths;
 *     uint16_t humidityTenths; uint16_t pressureTenths; uint16_t lux;
 *     uint16_t millivolts; uint32_t lastSeenSeconds;
 *   };  // aggregate, members in this order
 *   class SourceTable {
 *    public:
 *     // 16 when __AVR_ATmega32U4__ is defined, else 8
 *     static const uint8_t kCapacity = 8;
 *     void Update(const OregonDecoder &, uint32_t nowSeconds);
 *     void Update(const LacrosseDecoder &, uint32_t nowSeconds);
 *     void Update(const X10Decoder &, uint32_t nowSeconds);
 *     uint8_t Count() const;
 *     const SourceRecord &At(uint8_t index) const;
 *   };
 * Kind and field bits are nested in SourceRecord (SourceRecord::Oregon,
 * SourceRecord::Temperature...).
 */

#include <stdint.h>
#include <unity.h>

#include <LacrosseDecoder.h>
#include <LacrosseWS7000.h>
#include <Loopback.h>
#include <OregonDecoder.h>
#include <Oregon_v3.h>
#include <SourceTable.h>
#include <TestHal.h>
#include <X10Decoder.h>
#include <x10rf.h>

static const uint8_t T = SourceRecord::Temperature;
static const uint8_t H = SourceRecord::Humidity;
static const uint8_t P = SourceRecord::Pressure;
static const uint8_t LX = SourceRecord::Lux;
static const uint8_t MV = SourceRecord::Millivolts;

enum Protocol : uint8_t { OREGON_V3, WS7000, RFXMETER };

// One frame sent by a repository encoder. `id` is the Oregon channel, the
// Lacrosse address or the X10 address; `code` the Oregon rolling code (0 for
// the others); `value` is the lux or the X10 value.
struct Transmission {
  Protocol protocol;
  uint8_t id;
  uint8_t code;
  uint8_t fields;
  float temperature, humidity, pressure;
  uint32_t value;
};

// Feeds the encoder output to a decoder, updating the table on each frame
template <typename Decoder>
static void receive(SourceTable &table, uint32_t nowSeconds) {
  Decoder decoder;
  for (const Pulse &p : OrdersToPulses(TestHal.Orders)) {
    if (decoder.Feed(p.high, p.durationUs)) {
      table.Update(decoder, nowSeconds);
    }
  }
}

static void transmit(SourceTable &table, const Transmission &t,
                     uint32_t nowSeconds) {
  if (t.protocol == OREGON_V3) {
    OregonV3 encoder(&TestHal);
    encoder.SetChannel(t.id);
    encoder.SetRollingCode(t.code);
    if (t.fields & T)
      encoder.SetTemperature(t.temperature);
    if (t.fields & H)
      encoder.SetHumidity(t.humidity);
    if (t.fields & P)
      encoder.SetPressure(t.pressure);
    TestHal.ClearOrders();
    encoder.Send();
    receive<OregonDecoder>(table, nowSeconds);
  } else if (t.protocol == WS7000) {
    LacrosseWS7000 encoder(&TestHal);
    encoder.SetAddress(static_cast<LacrosseWS7000::Address>(t.id));
    if (t.fields & T)
      encoder.SetTemperature(t.temperature);
    if (t.fields & H)
      encoder.SetHumidity(t.humidity);
    if (t.fields & P)
      encoder.SetPressure(t.pressure);
    if (t.fields & LX)
      encoder.SetLuminosity(t.value);
    TestHal.ClearOrders();
    encoder.Send();
    receive<LacrosseDecoder>(table, nowSeconds);
  } else {
    x10rf encoder(&TestHal, 1);
    TestHal.ClearOrders();
    encoder.RFXmeter(t.id, 0, t.value);
    receive<X10Decoder>(table, nowSeconds);
  }
}

static void assertRecord(const char *id, const SourceRecord &expected,
                         const SourceRecord &actual) {
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected.kind, actual.kind, id);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(expected.id, actual.id, id);
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected.code, actual.code, id);
  TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected.fields, actual.fields, id);
  TEST_ASSERT_EQUAL_INT16_MESSAGE(expected.temperatureTenths,
                                  actual.temperatureTenths, id);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected.humidityTenths,
                                   actual.humidityTenths, id);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected.pressureTenths,
                                   actual.pressureTenths, id);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected.lux, actual.lux, id);
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected.millivolts, actual.millivolts, id);
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected.lastSeenSeconds,
                                   actual.lastSeenSeconds, id);
}

// @req FUNC-DECODE-006
void SourceTable_starts_empty() {
  // Given
  SourceTable table;

  // When
  uint8_t count = table.Count();

  // Then
  TEST_ASSERT_EQUAL_UINT8(0, count);
  TEST_ASSERT_EQUAL_UINT8(8, SourceTable::kCapacity);  // native build
}

struct MappingCase {
  const char *id;
  Transmission sent;
  SourceRecord expected;
};

static const MappingCase mappingCases[] = {
    {"oregon-thn132n",
     {OREGON_V3, 1, 0x3C, T, -10.2f, 0, 0, 0},
     {SourceRecord::Oregon, 1, 0x3C, T, -102, 0, 0, 0, 0, 100}},
    {"oregon-thgr228n",
     {OREGON_V3, 3, 0xC4, T | H, 21.5f, 67, 0, 0},
     {SourceRecord::Oregon, 3, 0xC4, T | H, 215, 670, 0, 0, 0, 100}},
    {"oregon-bthr918",
     {OREGON_V3, 3, 0x7E, T | H | P, 21.5f, 60, 1013, 0},
     {SourceRecord::Oregon, 3, 0x7E, T | H | P, 215, 600, 10130, 0, 0, 100}},
    {"lacrosse-type0",
     {WS7000, 0, 0, T, 21.5f, 0, 0, 0},
     {SourceRecord::Lacrosse, 0, 0, T, 215, 0, 0, 0, 0, 100}},
    {"lacrosse-type1",
     {WS7000, 3, 0, T | H, -0.5f, 63.7f, 0, 0},
     {SourceRecord::Lacrosse, 3, 0, T | H, -5, 637, 0, 0, 0, 100}},
    {"lacrosse-type4",
     {WS7000, 5, 0, T | H | P, -10.2f, 42.5f, 1013.4f, 0},
     {SourceRecord::Lacrosse, 5, 0, T | H | P, -102, 425, 10134, 0, 0, 100}},
    {"lacrosse-type5-lux",
     {WS7000, 7, 0, LX, 0, 0, 0, 1000},
     {SourceRecord::LacrosseLux, 7, 0, LX, 0, 0, 0, 1000, 0, 100}},
    {"x10-bcd-millivolts",
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x2950},
     {SourceRecord::X10, 10, 0, MV, 0, 0, 0, 0, 2950, 100}},
    {"x10-zero-millivolts",
     {RFXMETER, 0, 0, 0, 0, 0, 0, 0x0000},
     {SourceRecord::X10, 0, 0, MV, 0, 0, 0, 0, 0, 100}},
    {"x10-max-bcd",
     {RFXMETER, 255, 0, 0, 0, 0, 0, 0x9999},
     {SourceRecord::X10, 255, 0, MV, 0, 0, 0, 0, 9999, 100}},
    {"x10-hex-hundreds-nibble",
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x2A50},
     {SourceRecord::X10, 10, 0, 0, 0, 0, 0, 0, 0, 100}},
    {"x10-hex-units-nibble",
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x295A},
     {SourceRecord::X10, 10, 0, 0, 0, 0, 0, 0, 0, 100}},
    {"x10-above-9999",
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x10000},
     {SourceRecord::X10, 10, 0, 0, 0, 0, 0, 0, 0, 100}},
};

// @req FUNC-DECODE-006
void SourceTable_maps_each_decoded_frame_to_a_record() {
  for (const MappingCase &c : mappingCases) {
    // Given
    SourceTable table;

    // When
    transmit(table, c.sent, 100);

    // Then
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, table.Count(), c.id);
    assertRecord(c.id, c.expected, table.At(0));
  }
}

struct UpdateCase {
  const char *id;
  Transmission first, second;
  SourceRecord expected;
};

// The whole record is overwritten: fields absent from the second frame clear
static const UpdateCase updateCases[] = {
    {"oregon-device-change",
     {OREGON_V3, 1, 0x3C, T | H | P, 21.5f, 60, 1013, 0},
     {OREGON_V3, 1, 0x3C, T, -10.2f, 0, 0, 0},
     {SourceRecord::Oregon, 1, 0x3C, T, -102, 0, 0, 0, 0, 40}},
    {"lacrosse-new-values",
     {WS7000, 3, 0, T | H, 21.5f, 63.7f, 0, 0},
     {WS7000, 3, 0, T | H, 22.0f, 50, 0, 0},
     {SourceRecord::Lacrosse, 3, 0, T | H, 220, 500, 0, 0, 0, 40}},
    {"lux-new-value",
     {WS7000, 3, 0, LX, 0, 0, 0, 1000},
     {WS7000, 3, 0, LX, 0, 0, 0, 12},
     {SourceRecord::LacrosseLux, 3, 0, LX, 0, 0, 0, 12, 0, 40}},
    {"x10-value-loses-field",
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x2950},
     {RFXMETER, 10, 0, 0, 0, 0, 0, 0x2A50},
     {SourceRecord::X10, 10, 0, 0, 0, 0, 0, 0, 0, 40}},
};

// @req FUNC-DECODE-006
void SourceTable_updates_a_known_source_in_place() {
  for (const UpdateCase &c : updateCases) {
    // Given
    SourceTable table;
    transmit(table, {WS7000, 6, 0, T, 5.0f, 0, 0, 0}, 10);
    transmit(table, c.first, 20);
    transmit(table, {RFXMETER, 99, 0, 0, 0, 0, 0, 0x3300}, 30);

    // When
    transmit(table, c.second, 40);

    // Then
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, table.Count(), c.id);
    assertRecord(c.id, {SourceRecord::Lacrosse, 6, 0, T, 50, 0, 0, 0, 0, 10},
                 table.At(0));
    assertRecord(c.id, c.expected, table.At(1));
    assertRecord(c.id, {SourceRecord::X10, 99, 0, MV, 0, 0, 0, 0, 3300, 30},
                 table.At(2));
  }
}

struct DistinctCase {
  const char *id;
  Transmission first, second;
  SourceRecord::Kind firstKind, secondKind;
  uint8_t firstCode, secondCode;
};

static const DistinctCase distinctCases[] = {
    {"lacrosse-environment-and-lux-same-address",
     {WS7000, 4, 0, T | H | P, 21.5f, 50, 1013, 0},
     {WS7000, 4, 0, LX, 0, 0, 0, 1000},
     SourceRecord::Lacrosse,
     SourceRecord::LacrosseLux,
     0,
     0},
    {"oregon-and-lacrosse-same-id",
     {OREGON_V3, 1, 0x3C, T, 21.5f, 0, 0, 0},
     {WS7000, 1, 0, T, 21.5f, 0, 0, 0},
     SourceRecord::Oregon,
     SourceRecord::Lacrosse,
     0x3C,
     0},
    {"lacrosse-and-x10-same-id",
     {WS7000, 1, 0, T, 21.5f, 0, 0, 0},
     {RFXMETER, 1, 0, 0, 0, 0, 0, 0x2950},
     SourceRecord::Lacrosse,
     SourceRecord::X10,
     0,
     0},
    {"oregon-same-channel-other-rolling-code",
     {OREGON_V3, 2, 0xB8, T, 21.5f, 0, 0, 0},
     {OREGON_V3, 2, 0x36, T, 21.5f, 0, 0, 0},
     SourceRecord::Oregon,
     SourceRecord::Oregon,
     0xB8,
     0x36},
};

// @req FUNC-DECODE-006
void SourceTable_keeps_other_kind_or_code_as_another_source() {
  for (const DistinctCase &c : distinctCases) {
    // Given
    SourceTable table;
    transmit(table, c.first, 10);

    // When
    transmit(table, c.second, 20);

    // Then
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, table.Count(), c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.firstKind, table.At(0).kind, c.id);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(c.firstCode, table.At(0).code, c.id);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, table.At(0).lastSeenSeconds, c.id);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.secondKind, table.At(1).kind, c.id);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(c.secondCode, table.At(1).code, c.id);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(20, table.At(1).lastSeenSeconds, c.id);
  }
}

static const int8_t NONE = -1;

struct FullCase {
  const char *id;
  uint32_t seen[8];  // time each X10 source (address index + 1) is sent at
  int8_t refreshed;  // index sent again at t=400 before the 9th source
  uint8_t replaced;
};

static const FullCase fullCases[] = {
    {"oldest-first", {100, 101, 102, 103, 104, 105, 106, 107}, NONE, 0},
    {"oldest-in-middle", {107, 106, 100, 101, 102, 103, 104, 105}, NONE, 2},
    {"oldest-last", {107, 106, 105, 104, 103, 102, 101, 100}, NONE, 7},
    {"oldest-refreshed-then-next-oldest",
     {100, 101, 102, 103, 104, 105, 106, 107},
     0,
     1},
};

// @req FUNC-DECODE-006
void SourceTable_replaces_least_recently_seen_source_when_full() {
  for (const FullCase &c : fullCases) {
    // Given
    SourceTable table;
    uint32_t seen[8];
    for (uint8_t i = 0; i < 8; i++) {
      seen[i] = c.seen[i];
      transmit(table, {RFXMETER, uint8_t(i + 1), 0, 0, 0, 0, 0, 0x3300},
               seen[i]);
    }
    if (c.refreshed != NONE) {
      seen[c.refreshed] = 400;
      transmit(table,
               {RFXMETER, uint8_t(c.refreshed + 1), 0, 0, 0, 0, 0, 0x3300},
               400);
    }

    // When
    transmit(table, {OREGON_V3, 2, 0x3C, T, 21.5f, 0, 0, 0}, 500);

    // Then
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(8, table.Count(), c.id);
    for (uint8_t i = 0; i < 8; i++) {
      if (i == c.replaced) {
        assertRecord(c.id,
                     {SourceRecord::Oregon, 2, 0x3C, T, 215, 0, 0, 0, 0, 500},
                     table.At(i));
      } else {
        assertRecord(c.id,
                     {SourceRecord::X10, uint8_t(i + 1), 0, MV, 0, 0, 0, 0,
                      3300, seen[i]},
                     table.At(i));
      }
    }
  }
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(SourceTable_starts_empty);
  RUN_TEST(SourceTable_maps_each_decoded_frame_to_a_record);
  RUN_TEST(SourceTable_updates_a_known_source_in_place);
  RUN_TEST(SourceTable_keeps_other_kind_or_code_as_another_source);
  RUN_TEST(SourceTable_replaces_least_recently_seen_source_when_full);
  return UNITY_END();
}
