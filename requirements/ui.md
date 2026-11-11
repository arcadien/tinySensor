# UI Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### UI-DECODE-001
**Title:** FormatPage renders one source record and its age as two 16-character lines
**Status:** validated
**Dependencies:** FUNC-DECODE-006
**Description:** Free function `void FormatPage(const SourceRecord& record, uint32_t ageSeconds, char line1[17], char line2[17])` in `tinyDecoders/lib/PageFormatter/PageFormatter.h` (no dynamic allocation, no `printf`) writes exactly 16 characters plus `'\0'` per line, space-filled. Line 1: the present items among temperature, humidity, lux, millivolts, in that order, separated by one space from column 0; the label ends at column 16. Line 2: pressure, if present, from column 0; for Oregon records, then the rolling code `r` + `code` as 2 lowercase hex digits (one space after the pressure, else from column 0); the age ends at column 16. Items: temperature `[-]I.T` then `'\x7F'` (degree glyph) and `C` (−5 gives `-0.5`); humidity `(h+5)/10` capped at 99, then `%`; pressure `(p+5)/10` then `hPa`; lux in decimal then `lx`; millivolts as `(mV+5)/10` hundredths of volt `I.FF` then `V`. Label: `Ore` (Oregon), `Lac` (Lacrosse, LacrosseLux) or `X` (X10) followed by `id` in decimal. Age: below 60 `<s>s`, below 3600 `<s/60>m`, below 86400 `<s/3600>h`, else `<min(s/86400, 99)>d`. Output uses only `0-9`, space, `.`, `-`, `%`, `'\x7F'`, `C O r e L a b c f X h P l x V s m d`. Test (`°` = `'\x7F'`): Oregon 1, code 0xB8, 215, 600, 10130, 12 s gives `21.5°C 60%  Ore1` / `1013hPa rb8  12s`; Oregon 2, code 0x0F, 215 only, 12 s gives `21.5°C      Ore2` / `r0f          12s`; Lacrosse 3, −102, 425, 300 s gives `-10.2°C 43% Lac3` / `              5m`; X10 10, 2950 mV, 7200 s gives `2.95V        X10` / `              2h`; humidity 999 gives `99%`; 9 000 000 s gives `99d`.

### UI-DECODE-002
**Title:** Font8x16 holds the 8x16 glyphs of the FormatPage character set in flash, in SSD1306 page byte order
**Status:** validated
**Dependencies:** UI-DECODE-001
**Description:** Free function `void ReadGlyph(char c, uint8_t glyph[16])` in `tinyDecoders/lib/Font8x16/Font8x16.h` copies the glyph of `c` from a `PROGMEM` table (on native, `PROGMEM` is empty and reads are plain loads). Bytes 0–7 are columns 0–7 of the top 8 pixel rows, bytes 8–15 the bottom 8 rows, bit 0 the top pixel: the SSD1306 page layout. The table holds only the 30 non-space characters of the UI-DECODE-001 set (`'\x7F'` drawn as `°`). Space and any other character give 16 zero bytes. Bitmaps come from Spleen 8x16 (BSD-2-Clause, licence text in the generated file header), produced by `tinyDecoders/scripts/gen_font8x16.py <spleen-8x16.bdf>`. Test: each of the 30 glyphs has a set bit and they are pairwise distinct; space, `A`, `\0` give zeros.
