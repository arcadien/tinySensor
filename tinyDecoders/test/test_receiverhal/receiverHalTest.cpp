/*
 * Integration tests for TECH-DECODE-002: the receiver hardware is a
 * compile-time ReceiverHal alias; on native it is the TestReceiverHal double,
 * whose contract the Receiver tests (FUNC-DECODE-007) rely on.
 *
 * Production side must provide (code-builder handles this):
 *   tinyDecoders/lib/ReceiverHal/ReceiverHal.h:
 *     typedef AtmegaReceiverHal ReceiverHal;  // __AVR_ATmega328P__ or
 *                                             // __AVR_ATmega32U4__
 *     typedef TestReceiverHal ReceiverHal;    // when AVR is not defined
 *     #error                                  // any other MCU
 *   tinyDecoders/lib/ReceiverHal/TestReceiverHal.h (native only):
 *     class TestReceiverHal {
 *      public:
 *       bool PopPulse(bool &high, uint16_t &durationUs);  // oldest, FIFO
 *       uint32_t NowSeconds() const;
 *       void DrawChar(uint8_t page, uint8_t col, char c);  // start page 0-2,
 *                                                          // col 0-15
 *       void DrawBigChar(uint8_t col, char c);             // col 0-5
 *       void PushPulse(bool high, uint16_t durationUs);
 *       void SetNowSeconds(uint32_t s);
 *       const char *Row(uint8_t page) const;  // page 0-2, 16 chars,
 *                                             // initially spaces
 *       const char *Big() const;     // 6 chars, initially spaces
 *       uint16_t DrawCount() const;  // DrawChar + DrawBigChar calls so far
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

// big: DrawBigChar(col, c), page unused; else DrawChar(page, col, c)
struct Draw {
  bool big;
  uint8_t page, col;
  char c;
};

struct DrawCase {
  const char *id;
  Draw draws[4];
  uint8_t count;
  const char *page0;
  const char *page1;
  const char *page2;
  const char *big;
};

static const DrawCase drawCases[] = {
    {"blank-screen",
     {},
     0,
     "                ",
     "                ",
     "                ",
     "      "},
    {"page0-left",
     {{false, 0, 0, 'O'}},
     1,
     "O               ",
     "                ",
     "                ",
     "      "},
    {"page1-centre",
     {{false, 1, 7, 'W'}},
     1,
     "                ",
     "       W        ",
     "                ",
     "      "},
    {"page2-right",
     {{false, 2, 15, 's'}},
     1,
     "                ",
     "                ",
     "               s",
     "      "},
    {"big-col0",
     {{true, 0, 0, '2'}},
     1,
     "                ",
     "                ",
     "                ",
     "2     "},
    {"big-col5",
     {{true, 0, 5, '%'}},
     1,
     "                ",
     "                ",
     "                ",
     "     %"},
    {"one-of-each",
     {{false, 2, 12, 'h'}, {true, 0, 3, '-'}},
     2,
     "                ",
     "                ",
     "            h   ",
     "   -  "},
    {"overwrite",
     {{false, 0, 3, 'a'},
      {false, 0, 3, 'b'},
      {true, 0, 0, '1'},
      {true, 0, 0, '\x7F'}},
     4,
     "   b            ",
     "                ",
     "                ",
     "\x7F     "},
};

// @req TECH-DECODE-002
void TestReceiverHal_keeps_the_drawn_screen() {
  for (const DrawCase &c : drawCases) {
    // Given
    TestReceiverHal hal;

    // When
    for (uint8_t i = 0; i < c.count; i++) {
      const Draw &d = c.draws[i];
      if (d.big)
        hal.DrawBigChar(d.col, d.c);
      else
        hal.DrawChar(d.page, d.col, d.c);
    }

    // Then
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.page0, hal.Row(0), 16, c.id);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.page1, hal.Row(1), 16, c.id);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.page2, hal.Row(2), 16, c.id);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(c.big, hal.Big(), 6, c.id);
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
