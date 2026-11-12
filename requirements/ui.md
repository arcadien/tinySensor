# UI Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### UI-DECODE-001
**Title:** FormatPage renders one source record as a big main value and a 4-character side column
**Status:** validated
**Dependencies:** FUNC-DECODE-006
**Description:** Free function `void FormatPage(const SourceRecord& record, bool alternate, char big[7], char top[5], char bottom[5])` in `tinyDecoders/lib/PageFormatter/PageFormatter.h` (no dynamic allocation, no `printf`) writes 6, 4 and 4 characters plus `'\0'`, space-filled. `big` (drawn in 16x32 cells, UI-DECODE-003) holds, from column 0, the first present among: temperature `[-]I.T` then `'\x7F'` (degree glyph, no `C`; −5 gives `-0.5°`); lux in decimal then `lx`; millivolts as `(mV+5)/10` hundredths of volt `I.FF` then `V`. None present gives 6 spaces. `top`, right-aligned, is the source identifier: Oregon `id` in decimal, `-`, `code` as 2 lowercase hex digits; Lacrosse and LacrosseLux `L` + `id` in decimal; X10 `X` + `id` in decimal. `bottom`, right-aligned: humidity `(h+5)/10` capped at 99 then `%`, or pressure `(p+5)/10` in decimal without unit; when both are present, humidity if `alternate` is false, pressure if true; neither gives 4 spaces. No age and no protocol name are shown. Output uses only `0-9`, space, `.`, `-`, `%`, `'\x7F'`, `V l x L X a b c d e f`. Test (`°` = `'\x7F'`, `big` / `top` / `bottom`): Oregon 1, code 0x01, 235, 670 gives `23.5° ` / `1-01` / ` 67%`; Oregon 2, code 0x60, 188, 770, 9690 gives `18.8° ` / `2-60` / ` 77%` (alternate false) and ` 969` (true); Oregon 3, code 0xB8, 215 only gives `21.5° ` / `3-b8` / `    `; Lacrosse 3, −102, 425 gives `-10.2°` / `  L3` / ` 43%`; Lacrosse 1, 215, 425, 10134 gives ` 43%` (false) and `1013` (true); LacrosseLux 2, 1000 lux gives `1000lx` / `  L2` / `    `; X10 204, 3960 mV gives `3.96V ` / `X204` / `    `; X10 10 without field gives 6 spaces / ` X10`; −5 gives `-0.5° `; humidity 999 gives ` 99%`.

### UI-DECODE-002
**Title:** Font8x16 holds the 8x16 glyphs of the FormatPage character set in flash, in SSD1306 page byte order
**Status:** validated
**Dependencies:** UI-DECODE-001
**Description:** Free function `void ReadGlyph(char c, uint8_t glyph[16])` in `tinyDecoders/lib/Font8x16/Font8x16.h` copies the glyph of `c` from a `PROGMEM` table (on native, `PROGMEM` is empty and reads are plain loads). Bytes 0–7 are columns 0–7 of the top 8 pixel rows, bytes 8–15 the bottom 8 rows, bit 0 the top pixel: the SSD1306 page layout. The table holds only the 32 non-space characters of the UI-DECODE-001 set and of the FUNC-DECODE-007 waiting text `Waiting for data` (`'\x7F'` drawn as `°`; the waiting text adds `W g i n o r t`), about 32 × 17 = 544 B of flash. Space and any other character give 16 zero bytes. Bitmaps come from Spleen 8x16 (BSD-2-Clause, licence text in the generated file header), produced by `tinyDecoders/scripts/gen_font8x16.py <spleen-8x16.bdf>`. Test: each of the 32 glyphs has a set bit and they are pairwise distinct; space, `A`, `C`, `s`, `\0` give zeros.

### UI-DECODE-003
**Title:** Font8x16 reads a 16x32 glyph page by doubling the 8x16 glyph in both directions
**Status:** validated
**Dependencies:** UI-DECODE-002
**Description:** Free function `void ReadBigGlyphPage(char c, uint8_t page, uint8_t bytes[16])` in `tinyDecoders/lib/Font8x16/Font8x16.h` writes page `page` (0–3, top to bottom) of the 16x32 glyph of `c`, computed from `ReadGlyph(c)` without any RAM or flash table: with `s` = glyph byte `(page / 2) * 8 + x / 2` and `n` = its low nibble (even page) or high nibble (odd page), bits `2i` and `2i+1` of `bytes[x]` are both bit `i` of `n` (bit 0 the top pixel, SSD1306 page layout). Test: for each UI-DECODE-001 character and page, `bytes[2k] == bytes[2k+1]`, bits `2i` and `2i+1` are equal, and keeping one of each pair rebuilds `ReadGlyph(c)`; space gives zeros on the 4 pages.