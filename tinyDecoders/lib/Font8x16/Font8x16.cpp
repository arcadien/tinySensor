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
