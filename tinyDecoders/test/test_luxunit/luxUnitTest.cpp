/*
 * Integration tests for TECH-DECODE-004: the lux unit cell is the character
 * LuxUnit, shared by FormatPage and the Font8x16 readers.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/Font8x16/Font8x16.h:
 *   LuxUnit, the char '\x01';
 * ReadGlyph(LuxUnit) blank and FormatPage writing LuxUnit right after the lux
 * digits. ReadBigGlyphPage(LuxUnit, page) is checked by the UI-DECODE-003
 * suite (test_font8x16) through the '\x01' literal.
 * Kept in its own suite so the other suites build before LuxUnit exists.
 */

#include <stdint.h>
#include <string.h>
#include <unity.h>

#include <Font8x16.h>
#include <PageFormatter.h>
#include <SourceTable.h>

struct LuxCase {
  const char *id;
  uint16_t lux;
  size_t unitColumn;
};

static const LuxCase luxCases[] = {
    {"one-digit", 0, 1},
    {"four-digits", 1000, 4},
    {"five-digits", 65535, 5},
};

// @req TECH-DECODE-004
void FormatPage_puts_LuxUnit_right_after_the_lux_digits() {
  TEST_ASSERT_EQUAL_CHAR('\x01', LuxUnit);
  for (const LuxCase &c : luxCases) {
    // Given
    SourceRecord record = {
        SourceRecord::LacrosseLux, 2, 0, SourceRecord::Lux, 0, 0, 0, c.lux, 0};
    char big[7], top[5], bottom[5];

    // When
    FormatPage(record, false, big, top, bottom);

    // Then
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(LuxUnit, big[c.unitColumn], c.id);
  }
}

// @req TECH-DECODE-004
void ReadGlyph_gives_a_blank_glyph_for_LuxUnit() {
  // Given
  static const uint8_t blank[16] = {0};
  uint8_t glyph[16];
  memset(glyph, 0xA5, sizeof glyph);

  // When
  ReadGlyph(LuxUnit, glyph);

  // Then
  TEST_ASSERT_EQUAL_HEX8_ARRAY(blank, glyph, 16);
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(FormatPage_puts_LuxUnit_right_after_the_lux_digits);
  RUN_TEST(ReadGlyph_gives_a_blank_glyph_for_LuxUnit);
  return UNITY_END();
}
