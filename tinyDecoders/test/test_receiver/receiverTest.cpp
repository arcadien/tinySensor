/*
 * Integration tests for FUNC-DECODE-007: Receiver feeds pulses to the
 * decoders, cycles one page per source every 4 s and redraws only changed
 * characters, one per Poll. Frames come from the repository encoders through
 * OrdersToPulses into the TestReceiverHal double (TECH-DECODE-002).
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/Receiver/Receiver.h:
 *   class Receiver {
 *    public:
 *     explicit Receiver(ReceiverHal *hal);  // draws nothing
 *     void Poll();
 *   };
 * The waiting page line 1 is "Ore Lac X" from column 0, space-filled.
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

// Queues the encoder output then a radio silence, as the receiver sees it
static void receive(TestReceiverHal &hal) {
  for (const Pulse &p : OrdersToPulses(TestHal.Orders)) {
    hal.PushPulse(p.high, p.durationUs);
  }
  hal.PushPulse(false, 65535);
}

// Oregon channel 2: "23.7°C 45%  Ore2"
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

// Lacrosse address 3: "-0.5°C 64%  Lac3"
static void sendLacrosse(TestReceiverHal &hal, float temperature) {
  LacrosseWS7000 encoder(&TestHal);
  encoder.SetAddress(LacrosseWS7000::THREE);
  encoder.SetTemperature(temperature);
  encoder.SetHumidity(63.7f);
  TestHal.ClearOrders();
  encoder.Send();
  receive(hal);
}

// Polls until a Poll draws nothing; each Poll draws at most one character
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

static void assertScreen(const char *id, const char *row0, const char *row1,
                         const TestReceiverHal &hal) {
  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(row0, hal.Row(0), 16, id);
  TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(row1, hal.Row(1), 16, id);
}

struct WaitingCase {
  const char *id;
  uint32_t now;
  const char *row1;
};

static const WaitingCase waitingCases[] = {
    {"spec-12s", 12, "             12s"},
    {"boot", 0, "              0s"},
    {"one-hour", 3600, "              1h"},
};

// @req FUNC-DECODE-007
void Receiver_shows_the_waiting_page_and_uptime_before_any_frame() {
  for (const WaitingCase &c : waitingCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(c.now);

    // When
    settle(receiver, hal);

    // Then
    assertScreen(c.id, "Ore Lac X       ", c.row1, hal);
  }
}

struct DrawStepCase {
  const char *id;
  uint8_t polls;
  const char *row0;
  const char *row1;
};

// Waiting page at 12 s, from a blank screen: first differing character in
// row-major order, spaces already on screen are skipped
static const DrawStepCase drawStepCases[] = {
    {"first-char", 1, "O               ", "                "},
    {"first-word", 3, "Ore             ", "                "},
    {"skips-space", 4, "Ore L           ", "                "},
    {"row0-done", 7, "Ore Lac X       ", "                "},
    {"row1-starts", 8, "Ore Lac X       ", "             1  "},
    {"all-drawn", 10, "Ore Lac X       ", "             12s"},
};

// @req FUNC-DECODE-007
void Receiver_draws_the_first_changed_character_per_poll() {
  for (const DrawStepCase &c : drawStepCases) {
    // Given
    TestReceiverHal hal;
    Receiver receiver(&hal);
    hal.SetNowSeconds(12);

    // When
    for (uint8_t i = 0; i < c.polls; i++) {
      receiver.Poll();
    }

    // Then
    assertScreen(c.id, c.row0, c.row1, hal);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(c.polls, hal.DrawCount(), c.id);
  }
}

// @req FUNC-DECODE-007
void Receiver_draws_nothing_when_nothing_changed() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(8);
  sendOregon(hal);
  settle(receiver, hal);
  uint16_t drawn = hal.DrawCount();

  // When
  for (int i = 0; i < 5; i++) {
    receiver.Poll();
  }

  // Then
  TEST_ASSERT_EQUAL_UINT16(drawn, hal.DrawCount());
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

struct CycleCase {
  const char *id;
  uint32_t now;
  const char *row0;
  const char *row1;
};

// Both frames decoded at 8 s: Oregon is source 0, Lacrosse source 1
static const CycleCase cycleCases[] = {
    {"8s-oregon", 8, "23.7" DEG "C 45%  Ore2", "              0s"},
    {"11s-oregon-ages", 11, "23.7" DEG "C 45%  Ore2", "              3s"},
    {"12s-lacrosse", 12, "-0.5" DEG "C 64%  Lac3", "              4s"},
    {"15s-lacrosse", 15, "-0.5" DEG "C 64%  Lac3", "              7s"},
    {"16s-oregon-again", 16, "23.7" DEG "C 45%  Ore2", "              8s"},
    {"20s-lacrosse-again", 20, "-0.5" DEG "C 64%  Lac3", "             12s"},
};

// @req FUNC-DECODE-007
void Receiver_alternates_source_pages_every_4_seconds() {
  // Given
  TestReceiverHal hal;
  Receiver receiver(&hal);
  hal.SetNowSeconds(8);
  sendOregon(hal);
  sendLacrosse(hal, -0.5f);
  receiver.Poll();

  for (const CycleCase &c : cycleCases) {
    // When
    hal.SetNowSeconds(c.now);
    settle(receiver, hal);

    // Then
    assertScreen(c.id, c.row0, c.row1, hal);
  }
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
  assertScreen("lacrosse-update", "21.5" DEG "C 64%  Lac3", "              0s",
               hal);
}

void setUp(void) {
  TestHal.ClearOrders();
}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(Receiver_shows_the_waiting_page_and_uptime_before_any_frame);
  RUN_TEST(Receiver_draws_the_first_changed_character_per_poll);
  RUN_TEST(Receiver_draws_nothing_when_nothing_changed);
  RUN_TEST(Receiver_consumes_every_pending_pulse_in_one_poll);
  RUN_TEST(Receiver_alternates_source_pages_every_4_seconds);
  RUN_TEST(Receiver_shows_a_new_frame_without_waiting_for_the_next_second);
  return UNITY_END();
}
