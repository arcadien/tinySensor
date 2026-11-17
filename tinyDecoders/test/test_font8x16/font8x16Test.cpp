/*
 * Integration tests for UI-DECODE-002: Font8x16 holds the 8x16 glyphs of the
 * FormatPage character set (UI-DECODE-001) and of the waiting text
 * "Waiting for data" (FUNC-DECODE-007), in SSD1306 page byte order.
 *
 * Production side must provide (code-builder handles this), in
 * tinyDecoders/lib/Font8x16/Font8x16.h:
 *   void ReadGlyph(char c, uint8_t glyph[16]);
 * reading a PROGMEM table (on native, PROGMEM is empty and reads are plain
 * loads), generated from Spleen 8x16 by tinyDecoders/scripts/gen_font8x16.py.
 * UI-DECODE-003 adds, in the same header:
 *   void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16]);
 * page 0-3 of the 16x32 glyph, computed from ReadGlyph (no extra table).
 * The lux unit cell '\x01' (LuxUnit) is not doubled: pages 0-1 blank, pages
 * 2-3 the top then bottom halves of the 8x16 'l' (cols 0-7) and 'x' (8-15).
 */

#include <stdint.h>
#include <string.h>
#include <unity.h>

#include <Font8x16.h>

// The 32 non-space characters of UI-DECODE-001 ('\x7F' is the degree glyph)
// and of the waiting text "Waiting for data" (adds W g i n o r t)
static const char drawable[] =
    "0123456789.-%\x7F"
    "VlxLXabcdef"
    "Wginort";

struct Glyph {
  uint8_t bytes[16];
};

// Buffer starts dirty so unwritten bytes show up
static Glyph read(char c) {
  Glyph glyph;
  memset(glyph.bytes, 0xA5, sizeof glyph.bytes);
  ReadGlyph(c, glyph.bytes);
  return glyph;
}

static bool hasSetPixel(const Glyph &glyph) {
  for (uint8_t b : glyph.bytes) {
    if (b != 0)
      return true;
  }
  return false;
}

// @req UI-DECODE-002
void ReadGlyph_draws_every_character_of_the_page_set_distinctly() {
  TEST_ASSERT_EQUAL_size_t(32, strlen(drawable));  // table typo
  for (size_t i = 0; i < strlen(drawable); i++) {
    // Given
    char c = drawable[i];

    // When
    Glyph glyph = read(c);

    // Then
    char id[] = {c, '\0'};
    TEST_ASSERT_TRUE_MESSAGE(hasSetPixel(glyph), id);
    for (size_t j = 0; j < i; j++) {
      Glyph other = read(drawable[j]);
      TEST_ASSERT_TRUE_MESSAGE(
          memcmp(glyph.bytes, other.bytes, sizeof glyph.bytes) != 0, id);
    }
  }
}

struct BlankCase {
  const char *id;
  char c;
};

static const BlankCase blankCases[] = {
    {"space", ' '},
    {"uppercase-A-not-in-set", 'A'},
    {"uppercase-C-dropped-with-the-unit", 'C'},
    {"lowercase-s-dropped-with-the-age", 's'},
    {"nul", '\0'},
};

// @req UI-DECODE-002
void ReadGlyph_gives_a_blank_glyph_outside_the_set() {
  static const uint8_t blank[16] = {0};
  for (const BlankCase &b : blankCases) {
    // Given
    char c = b.c;

    // When
    Glyph glyph = read(c);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(blank, glyph.bytes, 16, b.id);
  }
}

// UI-DECODE-001 characters drawn in 16x32 cells, space included
static const char bigSet[] = " 0123456789.-%\x7FVlxLXabcdef";

// @req UI-DECODE-003
void ReadBigGlyphPage_doubles_the_8x16_glyph() {
  for (size_t k = 0; k < strlen(bigSet); k++) {
    // Given
    char c = bigSet[k];
    char id[] = {c, '\0'};
    uint8_t pages[4][16];
    memset(pages, 0xA5, sizeof pages);

    // When
    for (uint8_t page = 0; page < 4; page++)
      ReadBigGlyphPage(c, page, pages[page]);

    // Then
    uint8_t rebuilt[16] = {0};
    for (uint8_t page = 0; page < 4; page++) {
      for (uint8_t x = 0; x < 16; x += 2) {
        uint8_t b = pages[page][x];
        TEST_ASSERT_EQUAL_HEX8_MESSAGE(b, pages[page][x + 1], id);
        uint8_t nibble = 0;
        for (uint8_t i = 0; i < 4; i++) {
          uint8_t lo = (b >> (2 * i)) & 1;
          TEST_ASSERT_EQUAL_UINT8_MESSAGE(lo, (b >> (2 * i + 1)) & 1, id);
          nibble |= lo << i;
        }
        rebuilt[(page / 2) * 8 + x / 2] |= nibble << ((page % 2) * 4);
      }
    }
    Glyph glyph = read(c);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(glyph.bytes, rebuilt, 16, id);
  }
}

// @req UI-DECODE-003
void ReadBigGlyphPage_gives_blank_pages_for_space() {
  static const uint8_t blank[16] = {0};
  for (uint8_t page = 0; page < 4; page++) {
    // Given
    uint8_t bytes[16];
    memset(bytes, 0xA5, sizeof bytes);

    // When
    ReadBigGlyphPage(' ', page, bytes);

    // Then
    TEST_ASSERT_EQUAL_HEX8_ARRAY(blank, bytes, 16);
  }
}

// Lux unit cell, LuxUnit in Font8x16.h; literal so the suite builds without it
static const char luxUnit = '\x01';

// @req UI-DECODE-003
void ReadBigGlyphPage_draws_the_lux_unit_as_small_l_and_x_in_the_lower_half() {
  // Given
  Glyph l = read('l');
  Glyph x = read('x');
  uint8_t expected[4][16] = {};
  for (uint8_t half = 0; half < 2; half++) {
    memcpy(expected[2 + half], l.bytes + half * 8, 8);
    memcpy(expected[2 + half] + 8, x.bytes + half * 8, 8);
  }
  uint8_t pages[4][16];
  memset(pages, 0xA5, sizeof pages);

  // When
  for (uint8_t page = 0; page < 4; page++)
    ReadBigGlyphPage(luxUnit, page, pages[page]);

  // Then
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, pages, sizeof pages);
}

void setUp(void) {}
void tearDown(void) {}

int main(int, char **) {
  UNITY_BEGIN();
  RUN_TEST(ReadGlyph_draws_every_character_of_the_page_set_distinctly);
  RUN_TEST(ReadGlyph_gives_a_blank_glyph_outside_the_set);
  RUN_TEST(ReadBigGlyphPage_doubles_the_8x16_glyph);
  RUN_TEST(ReadBigGlyphPage_gives_blank_pages_for_space);
  RUN_TEST(
      ReadBigGlyphPage_draws_the_lux_unit_as_small_l_and_x_in_the_lower_half);
  return UNITY_END();
}
