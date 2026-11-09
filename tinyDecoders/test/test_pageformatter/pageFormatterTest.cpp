/*
 * Integration tests for UI-DECODE-001: FormatPage renders one SourceRecord
 * and its age as two 16-character lines.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/PageFormatter/PageFormatter.h (no dynamic allocation, no
 * printf):
 *   void FormatPage(const SourceRecord &record, uint32_t ageSeconds,
 *                   char line1[17], char line2[17]);
 * SourceRecord comes from SourceTable.h (FUNC-DECODE-006), with Kind and
 * field bits nested in it (SourceRecord::Oregon, SourceRecord::Temperature).
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
  char line1[17];
  char line2[17];
};

// Buffers start dirty so unwritten characters show up
static Page format(const SourceRecord &record, uint32_t ageSeconds) {
  Page page;
  memset(&page, '#', sizeof page);
  FormatPage(record, ageSeconds, page.line1, page.line2);
  return page;
}

struct PageCase {
  const char *id;
  SourceRecord record;  // kind, id, fields, temperature, humidity, pressure,
                        // lux, millivolts (lastSeenSeconds unused)
  uint32_t age;
  const char *line1;
  const char *line2;
};

static const PageCase pageCases[] = {
    {"spec-oregon",
     {ORE, 1, T | H | P, 215, 600, 10130, 0, 0},
     12,
     "21.5" DEG "C 60%  Ore1",
     "1013hPa      12s"},
    {"spec-lacrosse",
     {LAC, 3, T | H, -102, 425, 0, 0, 0},
     300,
     "-10.2" DEG "C 43% Lac3",
     "              5m"},
    {"spec-x10",
     {X10, 10, MV, 0, 0, 0, 0, 2950},
     7200,
     "2.95V        X10",
     "              2h"},
    {"spec-humidity-capped",
     {ORE, 2, T | H, 237, 999, 0, 0, 0},
     0,
     "23.7" DEG "C 99%  Ore2",
     "              0s"},
    {"worst-case-line1",
     {LAC, 7, T | H, -999, 999, 0, 0, 0},
     0,
     "-99.9" DEG "C 99% Lac7",
     "              0s"},
    {"temperature-only",
     {ORE, 1, T, -102, 0, 0, 0, 0},
     0,
     "-10.2" DEG "C     Ore1",
     "              0s"},
    {"temperature-minus-half",
     {LAC, 0, T, -5, 0, 0, 0, 0},
     0,
     "-0.5" DEG "C      Lac0",
     "              0s"},
    {"temperature-minus-one",
     {LAC, 0, T, -10, 0, 0, 0, 0},
     0,
     "-1.0" DEG "C      Lac0",
     "              0s"},
    {"temperature-zero",
     {LAC, 0, T, 0, 0, 0, 0, 0},
     0,
     "0.0" DEG "C       Lac0",
     "              0s"},
    {"temperature-high",
     {LAC, 0, T, 999, 0, 0, 0, 0},
     0,
     "99.9" DEG "C      Lac0",
     "              0s"},
    {"humidity-rounds-down",
     {LAC, 1, T | H, 200, 424, 0, 0, 0},
     0,
     "20.0" DEG "C 42%  Lac1",
     "              0s"},
    {"humidity-rounds-up",
     {LAC, 1, T | H, 200, 425, 0, 0, 0},
     0,
     "20.0" DEG "C 43%  Lac1",
     "              0s"},
    {"humidity-994-below-cap",
     {LAC, 1, T | H, 200, 994, 0, 0, 0},
     0,
     "20.0" DEG "C 99%  Lac1",
     "              0s"},
    {"humidity-995-capped",
     {LAC, 1, T | H, 200, 995, 0, 0, 0},
     0,
     "20.0" DEG "C 99%  Lac1",
     "              0s"},
    {"humidity-zero",
     {LAC, 1, T | H, 200, 0, 0, 0, 0},
     0,
     "20.0" DEG "C 0%   Lac1",
     "              0s"},
    {"pressure-rounds-down",
     {LAC, 5, T | H | P, -102, 425, 10134, 0, 0},
     0,
     "-10.2" DEG "C 43% Lac5",
     "1013hPa       0s"},
    {"pressure-rounds-up",
     {LAC, 5, T | H | P, 200, 500, 10135, 0, 0},
     0,
     "20.0" DEG "C 50%  Lac5",
     "1014hPa       0s"},
    {"pressure-three-digits",
     {LAC, 5, T | H | P, 200, 500, 8500, 0, 0},
     0,
     "20.0" DEG "C 50%  Lac5",
     "850hPa        0s"},
    {"lux",
     {LUX, 7, LX, 0, 0, 0, 1000, 0},
     0,
     "1000lx      Lac7",
     "              0s"},
    {"lux-zero",
     {LUX, 7, LX, 0, 0, 0, 0, 0},
     0,
     "0lx         Lac7",
     "              0s"},
    {"lux-max",
     {LUX, 7, LX, 0, 0, 0, 65535, 0},
     0,
     "65535lx     Lac7",
     "              0s"},
    {"millivolts-rounds-down",
     {X10, 10, MV, 0, 0, 0, 0, 2994},
     0,
     "2.99V        X10",
     "              0s"},
    {"millivolts-rounds-up",
     {X10, 10, MV, 0, 0, 0, 0, 2995},
     0,
     "3.00V        X10",
     "              0s"},
    {"millivolts-hundredths-padded",
     {X10, 10, MV, 0, 0, 0, 0, 1050},
     0,
     "1.05V        X10",
     "              0s"},
    {"millivolts-below-half-hundredth",
     {X10, 10, MV, 0, 0, 0, 0, 4},
     0,
     "0.00V        X10",
     "              0s"},
    {"millivolts-ten-volts",
     {X10, 10, MV, 0, 0, 0, 0, 9999},
     0,
     "10.00V       X10",
     "              0s"},
    {"x10-without-field",
     {X10, 10, 0, 0, 0, 0, 0, 0},
     0,
     "             X10",
     "              0s"},
    {"x10-max-address",
     {X10, 255, MV, 0, 0, 0, 0, 2950},
     0,
     "2.95V       X255",
     "              0s"},
};

static void assertLine(const char *id, const char *expected,
                       const char *actual) {
  TEST_ASSERT_EQUAL_size_t_MESSAGE(16, strlen(expected), id);  // table typo
  TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, actual, id);
}

// @req UI-DECODE-001
void FormatPage_renders_values_label_and_pressure() {
  for (const PageCase &c : pageCases) {
    // Given
    const SourceRecord &record = c.record;

    // When
    Page page = format(record, c.age);

    // Then
    assertLine(c.id, c.line1, page.line1);
    assertLine(c.id, c.line2, page.line2);
  }
}

struct AgeCase {
  const char *id;
  uint32_t age;
  const char *line2;
};

static const AgeCase ageCases[] = {
    {"0s", 0, "              0s"},
    {"59s", 59, "             59s"},
    {"60s-is-1m", 60, "              1m"},
    {"3599s-is-59m", 3599, "             59m"},
    {"3600s-is-1h", 3600, "              1h"},
    {"86399s-is-23h", 86399, "             23h"},
    {"86400s-is-1d", 86400, "              1d"},
    {"99d", 99UL * 86400, "             99d"},
    {"just-below-100d", 100UL * 86400 - 1, "             99d"},
    {"100d-capped", 100UL * 86400, "             99d"},
    {"spec-9000000s", 9000000, "             99d"},
    {"max-age", 0xFFFFFFFF, "             99d"},
};

// @req UI-DECODE-001
void FormatPage_renders_age_at_the_end_of_line2() {
  for (const AgeCase &c : ageCases) {
    // Given
    SourceRecord record = {X10, 10, 0, 0, 0, 0, 0, 0};

    // When
    Page page = format(record, c.age);

    // Then
    assertLine(c.id, c.line2, page.line2);
  }
}

static const char allowedGlyphs[] = "0123456789 .-%" DEG "COreLacXhPlxVsmd";

static void assertGlyphs(const char *id, const char *line) {
  for (int i = 0; i < 16; i++) {
    TEST_ASSERT_NOT_NULL_MESSAGE(strchr(allowedGlyphs, line[i]), id);
  }
}

// @req UI-DECODE-001
void FormatPage_uses_only_the_allowed_glyphs() {
  for (const PageCase &c : pageCases) {
    for (const AgeCase &a : ageCases) {
      // Given
      const SourceRecord &record = c.record;

      // When
      Page page = format(record, a.age);

      // Then
      assertGlyphs(c.id, page.line1);
      assertGlyphs(a.id, page.line2);
    }
  }
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(FormatPage_renders_values_label_and_pressure);
  RUN_TEST(FormatPage_renders_age_at_the_end_of_line2);
  RUN_TEST(FormatPage_uses_only_the_allowed_glyphs);
  return UNITY_END();
}
