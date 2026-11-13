/*
 * Integration tests for UI-DECODE-001: FormatPage renders one SourceRecord as
 * a 6-character big main value and a 4-character side column (top: source
 * identifier, bottom: humidity or pressure).
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/PageFormatter/PageFormatter.h (no dynamic allocation, no
 * printf):
 *   void FormatPage(const SourceRecord &record, bool alternate, char big[7],
 *                   char top[5], char bottom[5]);
 * SourceRecord comes from SourceTable.h (FUNC-DECODE-006).
 */

#include <stdint.h>
#include <string.h>
#include <unity.h>

#include <PageFormatter.h>
#include <SourceTable.h>

// Degree glyph; kept a separate literal so "\x7F" never eats a hex digit
#define DEG "\x7F"

static const uint8_t T = SourceRecord::Temperature;
static const uint8_t H = SourceRecord::Humidity;
static const uint8_t P = SourceRecord::Pressure;
static const uint8_t LX = SourceRecord::Lux;
static const uint8_t MV = SourceRecord::Millivolts;

static const SourceRecord::Kind ORE = SourceRecord::Oregon;
static const SourceRecord::Kind LAC = SourceRecord::Lacrosse;
static const SourceRecord::Kind LUX = SourceRecord::LacrosseLux;
static const SourceRecord::Kind X10 = SourceRecord::X10;

struct Page {
  char big[7];
  char top[5];
  char bottom[5];
};

// Buffers start dirty so unwritten characters show up
static Page format(const SourceRecord &record, bool alternate) {
  Page page;
  memset(&page, '#', sizeof page);
  FormatPage(record, alternate, page.big, page.top, page.bottom);
  return page;
}

struct PageCase {
  const char *id;
  SourceRecord record;  // kind, id, code, fields, temperature, humidity,
                        // pressure, lux, millivolts (lastSeenSeconds unused)
  bool alternate;
  const char *big;
  const char *top;
  const char *bottom;
};

static const PageCase pageCases[] = {
    // Test clause of the requirement
    {"spec-basile",
     {ORE, 1, 0x01, T | H, 235, 670, 0, 0, 0},
     false,
     "23.5" DEG " ",
     "1-01",
     " 67%"},
    {"spec-bthr-humidity",
     {ORE, 2, 0x60, T | H | P, 188, 770, 9690, 0, 0},
     false,
     "18.8" DEG " ",
     "2-60",
     " 77%"},
    {"spec-bthr-pressure",
     {ORE, 2, 0x60, T | H | P, 188, 770, 9690, 0, 0},
     true,
     "18.8" DEG " ",
     "2-60",
     " 969"},
    {"spec-oregon-temperature-only",
     {ORE, 3, 0xB8, T, 215, 0, 0, 0, 0},
     false,
     "21.5" DEG " ",
     "3-b8",
     "    "},
    {"spec-lacrosse",
     {LAC, 3, 0, T | H, -102, 425, 0, 0, 0},
     false,
     "-10.2" DEG,
     "  L3",
     " 43%"},
    {"spec-lacrosse-pressure-humidity",
     {LAC, 1, 0, T | H | P, 215, 425, 10134, 0, 0},
     false,
     "21.5" DEG " ",
     "  L1",
     " 43%"},
    {"spec-lacrosse-pressure-pressure",
     {LAC, 1, 0, T | H | P, 215, 425, 10134, 0, 0},
     true,
     "21.5" DEG " ",
     "  L1",
     "1013"},
    {"spec-lux",
     {LUX, 2, 0, LX, 0, 0, 0, 1000, 0},
     false,
     "1000lx",
     "  L2",
     "    "},
    {"spec-x10",
     {X10, 204, 0, MV, 0, 0, 0, 0, 3960},
     false,
     "3.96V ",
     "X204",
     "    "},
    {"spec-x10-without-field",
     {X10, 10, 0, 0, 0, 0, 0, 0, 0},
     false,
     "      ",
     " X10",
     "    "},
    {"spec-minus-half",
     {LAC, 0, 0, T, -5, 0, 0, 0, 0},
     false,
     "-0.5" DEG " ",
     "  L0",
     "    "},
    {"spec-humidity-capped",
     {LAC, 0, 0, T | H, 200, 999, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "  L0",
     " 99%"},
    // Temperature
    {"temperature-worst-case",
     {LAC, 7, 0, T | H, -999, 999, 0, 0, 0},
     false,
     "-99.9" DEG,
     "  L7",
     " 99%"},
    {"temperature-high",
     {LAC, 0, 0, T, 999, 0, 0, 0, 0},
     false,
     "99.9" DEG " ",
     "  L0",
     "    "},
    {"temperature-zero",
     {LAC, 0, 0, T, 0, 0, 0, 0, 0},
     false,
     "0.0" DEG "  ",
     "  L0",
     "    "},
    {"temperature-minus-one",
     {LAC, 0, 0, T, -10, 0, 0, 0, 0},
     false,
     "-1.0" DEG " ",
     "  L0",
     "    "},
    {"temperature-before-lux",
     {LAC, 0, 0, T | LX, 215, 0, 0, 1000, 0},
     false,
     "21.5" DEG " ",
     "  L0",
     "    "},
    {"lux-before-millivolts",
     {LUX, 0, 0, LX | MV, 0, 0, 0, 1000, 3960},
     false,
     "1000lx",
     "  L0",
     "    "},
    // Lux
    {"lux-zero",
     {LUX, 7, 0, LX, 0, 0, 0, 0, 0},
     false,
     "0lx   ",
     "  L7",
     "    "},
    {"lux-four-digits-max",
     {LUX, 7, 0, LX, 0, 0, 0, 9999, 0},
     false,
     "9999lx",
     "  L7",
     "    "},
    // Millivolts
    {"millivolts-rounds-down",
     {X10, 10, 0, MV, 0, 0, 0, 0, 2994},
     false,
     "2.99V ",
     " X10",
     "    "},
    {"millivolts-rounds-up",
     {X10, 10, 0, MV, 0, 0, 0, 0, 2995},
     false,
     "3.00V ",
     " X10",
     "    "},
    {"millivolts-hundredths-padded",
     {X10, 10, 0, MV, 0, 0, 0, 0, 1050},
     false,
     "1.05V ",
     " X10",
     "    "},
    {"millivolts-below-half-hundredth",
     {X10, 10, 0, MV, 0, 0, 0, 0, 4},
     false,
     "0.00V ",
     " X10",
     "    "},
    {"millivolts-ten-volts",
     {X10, 10, 0, MV, 0, 0, 0, 0, 9999},
     false,
     "10.00V",
     " X10",
     "    "},
    // Identifier
    {"oregon-channel-0",
     {ORE, 0, 0x0A, T, 200, 0, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "0-0a",
     "    "},
    {"oregon-code-letters-cd",
     {ORE, 1, 0xCD, T, 200, 0, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "1-cd",
     "    "},
    {"oregon-code-letters-ef",
     {ORE, 3, 0xEF, T, 200, 0, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "3-ef",
     "    "},
    {"x10-one-digit",
     {X10, 7, 0, MV, 0, 0, 0, 0, 2950},
     false,
     "2.95V ",
     "  X7",
     "    "},
    {"x10-max-address",
     {X10, 255, 0, MV, 0, 0, 0, 0, 2950},
     false,
     "2.95V ",
     "X255",
     "    "},
    {"lacrosse-two-digits",
     {LAC, 15, 0, T, 200, 0, 0, 0, 0},
     false,
     "20.0" DEG " ",
     " L15",
     "    "},
    // Humidity / pressure
    {"humidity-rounds-down",
     {LAC, 1, 0, T | H, 200, 424, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "  L1",
     " 42%"},
    {"humidity-994-below-cap",
     {LAC, 1, 0, T | H, 200, 994, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "  L1",
     " 99%"},
    {"humidity-995-capped",
     {LAC, 1, 0, T | H, 200, 995, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "  L1",
     " 99%"},
    {"humidity-zero",
     {LAC, 1, 0, T | H, 200, 0, 0, 0, 0},
     false,
     "20.0" DEG " ",
     "  L1",
     "  0%"},
    {"humidity-only-ignores-alternate",
     {LAC, 1, 0, T | H, 200, 425, 0, 0, 0},
     true,
     "20.0" DEG " ",
     "  L1",
     " 43%"},
    {"pressure-only-not-alternate",
     {LAC, 5, 0, T | P, 200, 0, 10135, 0, 0},
     false,
     "20.0" DEG " ",
     "  L5",
     "1014"},
    {"pressure-only-alternate",
     {LAC, 5, 0, T | P, 200, 0, 10135, 0, 0},
     true,
     "20.0" DEG " ",
     "  L5",
     "1014"},
    {"neither-ignores-alternate",
     {ORE, 3, 0xB8, T, 215, 0, 0, 0, 0},
     true,
     "21.5" DEG " ",
     "3-b8",
     "    "},
};

// @req UI-DECODE-001
void FormatPage_renders_main_value_identifier_and_side_value() {
  for (const PageCase &c : pageCases) {
    // Given
    const SourceRecord &record = c.record;

    // When
    Page page = format(record, c.alternate);

    // Then
    TEST_ASSERT_EQUAL_size_t_MESSAGE(6, strlen(c.big), c.id);  // table typo
    TEST_ASSERT_EQUAL_STRING_MESSAGE(c.big, page.big, c.id);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(c.top, page.top, c.id);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(c.bottom, page.bottom, c.id);
  }
}

static const bool kAlternates[] = {false, true};
static const char allowedGlyphs[] = "0123456789 .-%" DEG "VlxLXabcdef";

static void assertGlyphs(const char *id, const char *text, size_t length) {
  for (size_t i = 0; i < length; i++) {
    TEST_ASSERT_NOT_NULL_MESSAGE(strchr(allowedGlyphs, text[i]), id);
  }
}

// @req UI-DECODE-001
void FormatPage_uses_only_the_allowed_glyphs() {
  for (const PageCase &c : pageCases) {
    for (bool alternate : kAlternates) {
      // Given
      const SourceRecord &record = c.record;

      // When
      Page page = format(record, alternate);

      // Then
      assertGlyphs(c.id, page.big, 6);
      assertGlyphs(c.id, page.top, 4);
      assertGlyphs(c.id, page.bottom, 4);
    }
  }
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(FormatPage_renders_main_value_identifier_and_side_value);
  RUN_TEST(FormatPage_uses_only_the_allowed_glyphs);
  return UNITY_END();
}
