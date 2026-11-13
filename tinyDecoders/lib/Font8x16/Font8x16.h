#pragma once

#include <stdint.h>

// Copies the 8x16 glyph of `c` in SSD1306 page order (UI-DECODE-002): bytes
// 0-7 are columns 0-7 of the top 8 pixel rows, bytes 8-15 the bottom ones,
// bit 0 the top pixel. Characters outside the FormatPage set are blank.
void ReadGlyph(char c, uint8_t glyph[16]);

// Copies page `page` (0-3, top to bottom) of the 16x32 glyph of `c`, the 8x16
// glyph doubled in both directions (UI-DECODE-003), same byte layout.
void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16]);
