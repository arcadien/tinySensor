#pragma once

#include <stdint.h>

// Copies the 8x16 glyph of `c` in SSD1306 page order (UI-DECODE-002): bytes
// 0-7 are columns 0-7 of the top 8 pixel rows, bytes 8-15 the bottom ones,
// bit 0 the top pixel. Characters outside the FormatPage set are blank.
void ReadGlyph(char c, uint8_t glyph[16]);
