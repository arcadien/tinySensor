/*
 * Integration tests for FUNC-DECODE-007: Receiver feeds pulses to the
 * decoders, cycles one page per source every 10 s, alternates humidity and
 * pressure every 2 s, shows a centred waiting page only while the table is
 * empty and draws at most one pending cell per Poll. Frames come from the
 * repository encoders through OrdersToPulses into the TestReceiverHal double
 * (TECH-DECODE-002).
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/Receiver/Receiver.h:
 *   class Receiver {
 *    public:
 *     explicit Receiver(ReceiverHal *hal);  // draws nothing
 *     void Poll();
 *   };
 * A source page is FormatPage (UI-DECODE-001): `big` on DrawBigChar cols
 * 0-5, `top` on DrawChar page 0 cols 12-15, `bottom` on DrawChar page 2 cols
 * 12-15. The waiting page is "Waiting for data" on DrawChar page 1 cols 0-15.
 *
 * FUNC-DECODE-009: with ReceiverHal::HasLineOutput, each decoded frame also
 * goes out as one RFLink line (FormatRflink, FUNC-DECODE-008) through
 * hal->WriteLine, with a uint8_t counter starting at 0, no deduplication.
 */

#include <stdint.h>
#include <unity.h>

#include <LacrosseWS7000.h>
#include <Loopback.h>
#include <Oregon_v3.h>
#include <Receiver.h>
#include <ReceiverHal.h>
#include <TestHal.h>
#include <TestReceiverHal.h>

// Degree glyph; kept a separate literal so "\x7F" never eats a hex digit
#define DEG "\x7F"

static const char kWaiting[] = "Waiting for data";
static const char kBlank16[] = "                ";

// Queues the encoder output then a radio silence, as the receiver sees it
static void receive(TestReceiverHal &hal) {
  for (const Pulse &p : OrdersToPulses(TestHal.Orders)) {
    hal.PushPulse(p.high, p.durationUs);
  }
  hal.PushPulse(false, 65535);
}

// Oregon THGN123N channel 2, rolling code 0x3C: "23.7° " / "2-3c" / " 45%"
static void sendOregon(TestReceiverHal &hal) {
  OregonV3 encoder(&TestHal);
  encoder.SetChannel(2);
  encoder.SetRollingCode(0x3C);
  encoder.SetTemperature(23.7f);
  encoder.SetHumidity(45);
  TestHal.ClearOrders();
  encoder.Send();
  receive(hal);
}

// Oregon BTHR918 channel 1, rolling code 0xB8: "21.5° " / "1-b8" /
// " 60%" or "1013"
static void sendBthr918(TestReceiverHal &hal) {
  OregonV3 encoder(&TestHal);
  encoder.SetChannel(1);
  encoder.SetRollingCode(0xB8);
  encoder.SetTemperature(21.5f);
  encoder.SetHumidity(60);
  encoder.SetPressure(1013);
  TestHal.ClearOrders();
  encoder.Send();
  receive(hal);
}

// Lacrosse type 1 address 3: "-0.5° " / "  L3" / " 64%" at -0.5
static void sendLacrosse(TestReceiverHal &hal, float temperature) {
  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(LacrosseWS7000::THREE);
  encoder.SetTemperature(temperature);
  encoder.SetHumidity(63.7f);
  TestHal.ClearOrders();
  encoder.Send();
  receive(hal);
}

// Polls until a Poll draws nothing; each Poll draws at most one cell
static void settle(Receiver &receiver, TestReceiverHal &hal) {
  for (int i = 0; i < 64; i++) {
    uint16_t before = hal.DrawCount();
    receiver.Poll();
    uint16_t drawn = hal.DrawCount() - before;
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(1, drawn);
    if (drawn == 0)
      return;
  }
  TEST_FAIL_MESSAGE("screen never settled");
}

static void polls(Receiver &receiver, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    receiver.Poll();
  }
}

// Source page cells: big 0-5, top at page 0 cols 12-15, bottom at page 2
static void assertSourcePage(const char *id, const char *big, const char *top,
                             const char *bottom, const TestReceiverHal &hal) {
  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(big, hal.Big(), 6, id);
  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(top, hal.Row(0) + 12, 4, id);
  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(bottom, hal.Row(2) + 12, 4, id);
}

// @req FUNC-DECODE-007
void Receiver_shows_the_centred_waiting_page_while_the_table_is_empty() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(12);

  // When
  settle(receiver, hal);

  // Then
  TEST_ASSERT_EQUAL_STRING_LEN(kWaiting, hal.Row(1), 16);
  TEST_ASSERT_EQUAL_UINT16(16, hal.DrawCount());
  TEST_ASSERT_EQUAL_STRING_LEN(kBlank16, hal.Row(0), 16);
  TEST_ASSERT_EQUAL_STRING_LEN(kBlank16, hal.Row(2), 16);
  TEST_ASSERT_EQUAL_STRING_LEN("      ", hal.Big(), 6);
}

struct WaitingStepCase {
  const char *id;
  uint8_t polls;
  const char *row1;
};

// From a blank screen: one waiting text cell per Poll, cols 0-15, spaces
// included
static const WaitingStepCase waitingStepCases[] = {
    {"first-cell", 1, "W               "},
    {"first-word", 7, "Waiting         "},
    {"space-counts", 8, "Waiting         "},
    {"second-word", 11, "Waiting for     "},
    {"all-drawn", 16, "Waiting for data"},
};

// @req FUNC-DECODE-007
void Receiver_draws_the_waiting_text_one_cell_per_poll() {
  for (const WaitingStepCase &c : waitingStepCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(12);

    // When
    polls(receiver, c.polls);

    // Then
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.row1, hal.Row(1), 16, c.id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(c.polls, hal.DrawCount(), c.id);
  }
}

struct SourceStepCase {
  const char *id;
  uint8_t polls;
  const char *big;
  const char *top;
  const char *bottom;
};

// Oregon page entered at 12 s from the settled waiting page: all 14 cells are
// drawn, spaces included, big 0-5 then top 12-15 then bottom 12-15
static const SourceStepCase sourceStepCases[] = {
    {"big-first", 1, "2     ", "    ", "    "},
    {"big-degree", 5, "23.7" DEG " ", "    ", "    "},
    {"big-trailing-space", 6, "23.7" DEG " ", "    ", "    "},
    {"top-first", 7, "23.7" DEG " ", "2   ", "    "},
    {"top-done", 10, "23.7" DEG " ", "2-3c", "    "},
    {"bottom-leading-space", 11, "23.7" DEG " ", "2-3c", "    "},
    {"bottom-tens", 12, "23.7" DEG " ", "2-3c", " 4  "},
    {"all-14-drawn", 14, "23.7" DEG " ", "2-3c", " 45%"},
};

// @req FUNC-DECODE-007
void Receiver_draws_every_source_cell_in_order_on_the_first_frame() {
  for (const SourceStepCase &c : sourceStepCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(12);
    settle(receiver, hal);
    sendOregon(hal);

    // When
    polls(receiver, c.polls);

    // Then
    assertSourcePage(c.id, c.big, c.top, c.bottom, hal);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(16 + c.polls, hal.DrawCount(), c.id);
  }
}

// @req FUNC-DECODE-007
void Receiver_draws_exactly_14_cells_when_entering_a_source_page() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(12);
  settle(receiver, hal);
  sendOregon(hal);

  // When
  settle(receiver, hal);

  // Then
  TEST_ASSERT_EQUAL_UINT16(16 + 14, hal.DrawCount());
}

struct CycleCase {
  const char *id;
  uint32_t now;
  const char *big;
  const char *top;
  const char *bottom;
};

// Both frames decoded at 20 s: Oregon is source 0, Lacrosse source 1;
// page index (now / 10) % 2
static const CycleCase cycleCases[] = {
    {"20s-oregon", 20, "23.7" DEG " ", "2-3c", " 45%"},
    {"29s-oregon", 29, "23.7" DEG " ", "2-3c", " 45%"},
    {"30s-lacrosse", 30, "-0.5" DEG " ", "  L3", " 64%"},
    {"39s-lacrosse", 39, "-0.5" DEG " ", "  L3", " 64%"},
    {"40s-oregon-again", 40, "23.7" DEG " ", "2-3c", " 45%"},
    {"50s-lacrosse-again", 50, "-0.5" DEG " ", "  L3", " 64%"},
};

// @req FUNC-DECODE-007
void Receiver_alternates_source_pages_every_10_seconds() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(20);
  sendOregon(hal);
  sendLacrosse(hal, -0.5f);
  receiver.Poll();

  for (const CycleCase &c : cycleCases) {
    // When
    hal.SetNowSeconds(c.now);
    settle(receiver, hal);

    // Then
    assertSourcePage(c.id, c.big, c.top, c.bottom, hal);
  }
}

struct AlternateCase {
  const char *id;
  uint32_t now;
  const char *bottom;
};

// BTHR918 decoded at 0 s: humidity when (now / 2) % 2 == 0, else pressure
static const AlternateCase alternateCases[] = {
    {"0s-humidity", 0, " 60%"}, {"1s-humidity", 1, " 60%"},
    {"2s-pressure", 2, "1013"}, {"3s-pressure", 3, "1013"},
    {"4s-humidity", 4, " 60%"}, {"6s-pressure", 6, "1013"},
};

// @req FUNC-DECODE-007
void Receiver_alternates_humidity_and_pressure_every_2_seconds() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(0);
  sendBthr918(hal);
  receiver.Poll();

  for (const AlternateCase &c : alternateCases) {
    // When
    hal.SetNowSeconds(c.now);
    settle(receiver, hal);

    // Then
    assertSourcePage(c.id, "21.5" DEG " ", "1-b8", c.bottom, hal);
  }
}

struct IdleCase {
  const char *id;
  uint32_t now;
};

// Oregon (humidity only) settled at 20 s: same page, same values
static const IdleCase idleCases[] = {
    {"same-second", 20},
    {"next-second", 21},
    {"alternate-flips-without-pressure", 22},
    {"end-of-page", 29},
};

// @req FUNC-DECODE-007
void Receiver_draws_nothing_when_nothing_changed() {
  for (const IdleCase &c : idleCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(20);
    sendOregon(hal);
    settle(receiver, hal);
    uint16_t drawn = hal.DrawCount();
    hal.SetNowSeconds(c.now);

    // When
    polls(receiver, 5);

    // Then
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(drawn, hal.DrawCount(), c.id);
  }
}

// @req FUNC-DECODE-007
void Receiver_consumes_every_pending_pulse_in_one_poll() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  sendOregon(hal);
  sendLacrosse(hal, -0.5f);

  // When
  receiver.Poll();

  // Then
  bool high;
  uint16_t durationUs;
  TEST_ASSERT_FALSE(hal.PopPulse(high, durationUs));
}

// @req FUNC-DECODE-007
void Receiver_shows_a_new_frame_without_waiting_for_the_next_second() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(12);
  sendLacrosse(hal, -0.5f);
  settle(receiver, hal);

  // When
  sendLacrosse(hal, 21.5f);
  settle(receiver, hal);

  // Then
  assertSourcePage("lacrosse-update", "21.5" DEG " ", "  L3", " 64%", hal);
}

// @req FUNC-DECODE-007
void Receiver_keeps_a_source_last_seen_1799_seconds_ago() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(100);
  sendLacrosse(hal, -0.5f);
  settle(receiver, hal);
  uint16_t drawn = hal.DrawCount();

  // When
  hal.SetNowSeconds(1899);
  polls(receiver, 5);

  // Then
  TEST_ASSERT_EQUAL_UINT16(drawn, hal.DrawCount());
  assertSourcePage("kept", "-0.5" DEG " ", "  L3", " 64%", hal);
  TEST_ASSERT_EQUAL_STRING_LEN(kBlank16, hal.Row(1), 16);
}

struct ExpireStepCase {
  const char *id;
  uint8_t polls;
  const char *big;
  const char *top;
  const char *bottom;
  const char *row1;
};

// Lacrosse seen at 100 s (14 cells drawn from the blank screen), expired at
// 1900 s: the 14 source cells go blank first, then the waiting text
static const ExpireStepCase expireStepCases[] = {
    {"big-blanking", 3, "   5" DEG " ", "  L3", " 64%", kBlank16},
    {"big-blank", 6, "      ", "  L3", " 64%", kBlank16},
    {"top-blank", 10, "      ", "    ", " 64%", kBlank16},
    {"source-blank", 14, "      ", "    ", "    ", kBlank16},
    {"text-starts", 15, "      ", "    ", "    ", "W               "},
    {"waiting-page", 30, "      ", "    ", "    ", kWaiting},
};

// @req FUNC-DECODE-007
void Receiver_blanks_the_source_page_then_waits_when_the_last_source_expires() {
  for (const ExpireStepCase &c : expireStepCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(100);
    sendLacrosse(hal, -0.5f);
    settle(receiver, hal);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(14, hal.DrawCount(), c.id);
    hal.SetNowSeconds(1900);

    // When
    polls(receiver, c.polls);

    // Then
    assertSourcePage(c.id, c.big, c.top, c.bottom, hal);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.row1, hal.Row(1), 16, c.id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(14 + c.polls, hal.DrawCount(), c.id);
  }
}

// @req FUNC-DECODE-009
void Receiver_writes_one_rflink_line_per_decoded_frame() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  sendOregon(hal);
  sendOregon(hal);
  sendLacrosse(hal, -0.5f);
  sendLacrosse(hal, -0.5f);

  // When
  receiver.Poll();

  // Then
  static const char *const expected[] = {
      "20;00;Oregon TempHygro;ID=3C02;TEMP=00ed;HUM=45;BAT=OK;",
      "20;01;Oregon TempHygro;ID=3C02;TEMP=00ed;HUM=45;BAT=OK;",
      "20;02;LaCrosse WS7000;ID=0103;TEMP=8005;HUM=64;",
      "20;03;LaCrosse WS7000;ID=0103;TEMP=8005;HUM=64;",
  };
  TEST_ASSERT_EQUAL_INT(4, hal.Lines().size());
  for (size_t i = 0; i < 4; i++) {
    TEST_ASSERT_EQUAL_STRING(expected[i], hal.Lines()[i].c_str());
  }
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Receiver_shows_the_centred_waiting_page_while_the_table_is_empty);
  RUN_TEST(Receiver_draws_the_waiting_text_one_cell_per_poll);
  RUN_TEST(Receiver_draws_every_source_cell_in_order_on_the_first_frame);
  RUN_TEST(Receiver_draws_exactly_14_cells_when_entering_a_source_page);
  RUN_TEST(Receiver_alternates_source_pages_every_10_seconds);
  RUN_TEST(Receiver_alternates_humidity_and_pressure_every_2_seconds);
  RUN_TEST(Receiver_draws_nothing_when_nothing_changed);
  RUN_TEST(Receiver_consumes_every_pending_pulse_in_one_poll);
  RUN_TEST(Receiver_shows_a_new_frame_without_waiting_for_the_next_second);
  RUN_TEST(Receiver_keeps_a_source_last_seen_1799_seconds_ago);
  RUN_TEST(
      Receiver_blanks_the_source_page_then_waits_when_the_last_source_expires);
  RUN_TEST(Receiver_writes_one_rflink_line_per_decoded_frame);
  return UNITY_END();
}
