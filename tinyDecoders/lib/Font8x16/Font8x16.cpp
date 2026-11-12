#include "Font8x16.h"

#ifdef AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(address) (*(const uint8_t *)(address))
#endif

#include "Font8x16Glyphs.h"

void ReadGlyph(char c, uint8_t glyph[16]) {
  for (uint8_t i = 0; i < 16; i++) glyph[i] = 0;
  for (uint8_t g = 0; g < GlyphCount; g++) {
    if ((char)pgm_read_byte(&GlyphChars[g]) == c) {
      for (uint8_t i = 0; i < 16; i++)
        glyph[i] = pgm_read_byte(&GlyphBytes[g][i]);
      return;
    }
  }
}

void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16]) {
  uint8_t glyph[16];
  ReadGlyph(c, glyph);
  for (uint8_t x = 0; x < 16; x++) {
    uint8_t nibble = glyph[(page / 2) * 8 + x / 2] >> ((page % 2) * 4);
    uint8_t doubled = 0;
    for (uint8_t i = 0; i < 4; i++) {
      if (nibble & (1 << i))
        doubled |= 3 << (2 * i);
    }
    bytes[x] = doubled;
  }
}
