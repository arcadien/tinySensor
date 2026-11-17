#pragma once

#include <stdint.h>

// Lux unit big cell, written by FormatPage after the lux digits; its small
// glyph is blank (TECH-DECODE-004)
static const char LuxUnit = '\x01';

// Copies the 8x16 glyph of `c` in SSD1306 page order (UI-DECODE-002): bytes
// 0-7 are columns 0-7 of the top 8 pixel rows, bytes 8-15 the bottom ones,
// bit 0 the top pixel. Characters outside the FormatPage set are blank.
void ReadGlyph(char c, uint8_t glyph[16]);

// Copies page `page` (0-3, top to bottom) of the 16x32 glyph of `c`, the 8x16
// glyph doubled in both directions (UI-DECODE-003), same byte layout.
// LuxUnit gives blank pages 0-1 and the plain 8x16 l and x in pages 2-3.
void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16]);
