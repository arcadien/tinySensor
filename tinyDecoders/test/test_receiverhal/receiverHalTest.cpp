/*
 * Integration tests for TECH-DECODE-002: the receiver hardware is a
 * compile-time ReceiverHal alias; on native it is the TestReceiverHal double,
 * whose contract the Receiver tests (FUNC-DECODE-007) rely on.
 *
 * Production side must provide (code-builder handles this):
 *   tinyDecoders/lib/ReceiverHal/ReceiverHal.h:
 *     typedef Attiny84aReceiverHal ReceiverHal;  // when AVR is defined
 *     typedef TestReceiverHal ReceiverHal;       // otherwise
 *   tinyDecoders/lib/ReceiverHal/TestReceiverHal.h (native only):
 *     class TestReceiverHal {
 *      public:
 *       bool PopPulse(bool &high, uint16_t &durationUs);  // oldest, FIFO
 *       uint32_t NowSeconds() const;
 *       void DrawChar(uint8_t row, uint8_t col, char c);  // row 0-1, col 0-15
 *       void PushPulse(bool high, uint16_t durationUs);
 *       void SetNowSeconds(uint32_t s);
 *       const char *Row(uint8_t row) const;  // 16 chars, initially spaces
 *       uint16_t DrawCount() const;          // DrawChar calls so far
 *       static const bool HasLineOutput;     // true
 *       void WriteLine(const char *line);    // appends to Lines()
 *       const std::vector<std::string> &Lines() const;
 *     };  // default-constructible, no virtual
 */

#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>
#include <unity.h>

#include <ReceiverHal.h>
#include <TestReceiverHal.h>

static_assert(std::is_same<ReceiverHal, TestReceiverHal>::value,
              "ReceiverHal is TestReceiverHal on native");
static_assert(TestReceiverHal::HasLineOutput,
              "the native double records RFLink lines");

struct PulseCase {
  bool high;
  uint16_t durationUs;
};

static const PulseCase pulses[] = {
    {true, 512}, {false, 1024}, {true, 0}, {false, 65535}, {true, 400},
};

// @req TECH-DECODE-002
void TestReceiverHal_pops_pushed_pulses_in_order_then_none() {
  // Given
  TestReceiverHal hal;
  for (const PulseCase &p : pulses) {
    hal.PushPulse(p.high, p.durationUs);
  }

  // When
  const size_t n = sizeof pulses / sizeof pulses[0];
  PulseCase popped[n + 1] = {};
  bool available[n + 1];
  for (size_t i = 0; i <= n; i++) {
    available[i] = hal.PopPulse(popped[i].high, popped[i].durationUs);
  }

  // Then
  for (size_t i = 0; i < n; i++) {
    TEST_ASSERT_TRUE(available[i]);
    TEST_ASSERT_EQUAL(pulses[i].high, popped[i].high);
    TEST_ASSERT_EQUAL_UINT16(pulses[i].durationUs, popped[i].durationUs);
  }
  TEST_ASSERT_FALSE(available[n]);
}

// @req TECH-DECODE-002
void TestReceiverHal_reports_the_set_clock() {
  // Given
  TestReceiverHal hal;

  // When
  hal.SetNowSeconds(86400);

  // Then
  TEST_ASSERT_EQUAL_UINT32(86400, hal.NowSeconds());
}

struct Draw {
  uint8_t row, col;
  char c;
};

struct DrawCase {
  const char *id;
  Draw draws[3];
  uint8_t count;
  const char *row0;
  const char *row1;
};

static const DrawCase drawCases[] = {
    {"blank-screen", {}, 0, "                ", "                "},
    {"top-left", {{0, 0, 'O'}}, 1, "O               ", "                "},
    {"bottom-right", {{1, 15, 's'}}, 1, "                ", "               s"},
    {"overwrite",
     {{0, 3, 'a'}, {0, 3, 'b'}, {1, 0, '\x7F'}},
     3,
     "   b            ",
     "\x7F               "},
};

// @req TECH-DECODE-002
void TestReceiverHal_keeps_the_drawn_screen() {
  for (const DrawCase &c : drawCases) {
    // Given
    TestReceiverHal hal;

    // When
    for (uint8_t i = 0; i < c.count; i++) {
      hal.DrawChar(c.draws[i].row, c.draws[i].col, c.draws[i].c);
    }

    // Then
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.row0, hal.Row(0), 16, c.id);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.row1, hal.Row(1), 16, c.id);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(c.count, hal.DrawCount(), c.id);
  }
}

// @req TECH-DECODE-002
void TestReceiverHal_records_written_lines_in_order() {
  // Given
  TestReceiverHal hal;

  // When
  hal.WriteLine("20;00;X10Meter;ID=000A;");
  hal.WriteLine("20;01;LaCrosse WS7000;ID=0502;LUX=03e8;");

  // Then
  const std::vector<std::string> &lines = hal.Lines();
  TEST_ASSERT_EQUAL_INT(2, lines.size());
  TEST_ASSERT_EQUAL_STRING("20;00;X10Meter;ID=000A;", lines[0].c_str());
  TEST_ASSERT_EQUAL_STRING("20;01;LaCrosse WS7000;ID=0502;LUX=03e8;",
                           lines[1].c_str());
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(TestReceiverHal_pops_pushed_pulses_in_order_then_none);
  RUN_TEST(TestReceiverHal_reports_the_set_clock);
  RUN_TEST(TestReceiverHal_keeps_the_drawn_screen);
  RUN_TEST(TestReceiverHal_records_written_lines_in_order);
  return UNITY_END();
}
