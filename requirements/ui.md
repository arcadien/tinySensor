# UI Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->
<!-- UI requirements state what the screen shows; function names, signatures and memory layout belong in docs/architecture.md. -->

### UI-DECODE-001
**Title:** A source page shows its main value in big characters and a 4-character side column
**Status:** validated
**Dependencies:** [FUNC-DECODE-006](functional.md#func-decode-006)
**Description:** A source page has a big field of 6 characters (drawn in 16x32 cells, [UI-DECODE-003](#ui-decode-003)), and a side column of two right-aligned 4-character lines: the identifier on top, the second value below. On the 128x32 screen, big cell `i` (0–5) covers pixel columns 16i to 16i + 15 over all 32 pixel rows, and the side column takes 8x16 columns 12–15: the identifier on row 0 (pixel rows 0–15) and the second value on row 2 (pixel rows 16–31). These 14 cells cover the whole screen. Unused positions are spaces. The big field shows, from its first column, the first value present among: temperature `[-]I.T` then `°` (no `C`; −5 tenths shows `-0.5°`); lux in decimal, 1 to 5 digits, then in the next big cell the unit `lx` as two small characters in its lower half ([UI-DECODE-003](#ui-decode-003)); millivolts rounded to hundredths of a volt `I.FF` then `V`. With none of them, it is blank. The identifier is: for Oregon, the channel in decimal, `-`, the rolling code as 2 lowercase hex digits; for Lacrosse and Lacrosse luminosity, `L` and the address in decimal; for X10, `X` and the address in decimal. The second value is the humidity rounded to whole %, capped at 99, then `%`, or the pressure rounded to whole hPa without unit. When both are present, the page shows humidity, then pressure on the alternate slot ([FUNC-DECODE-007](functional.md#func-decode-007)). With neither, it is blank. No age and no protocol name are shown. The page uses only `0-9`, space, `.`, `-`, `%`, `°`, `V l x L X a b c d e f`. Examples (big field / identifier / second value): Oregon 1, code 0x01, 23.5 °C, 67.0 % gives `23.5° ` / `1-01` / ` 67%`; Oregon 2, code 0x60, 18.8 °C, 77.0 %, 969.0 hPa gives `18.8° ` / `2-60` / ` 77%`, then ` 969` on the alternate slot; Oregon 3, code 0xB8, 21.5 °C only gives `21.5° ` / `3-b8` / `    `; Lacrosse 3, −10.2 °C, 42.5 % gives `-10.2°` / `  L3` / ` 43%`; Lacrosse 1, 21.5 °C, 42.5 %, 1013.4 hPa gives ` 43%`, then `1013` on the alternate slot; Lacrosse luminosity 2, 1000 lux gives `1000`, the `lx` cell, a space / `  L2` / `    `; 65535 lux gives `65535` then the `lx` cell; 0 lux gives `0`, the `lx` cell, 4 spaces; X10 204, 3960 mV gives `3.96V ` / `X204` / `    `; X10 10 without value gives 6 spaces / ` X10`; −0.5 °C gives `-0.5° `; 99.9 % gives ` 99%`. On the receiver, a source page shows its big field in big cells 0–5, its identifier in columns 12–15 of row 0 and its second value in columns 12–15 of row 2.

### UI-DECODE-002
**Title:** Small characters are the 8x16 Spleen glyphs of the characters the pages use
**Status:** validated
**Dependencies:** [UI-DECODE-001](#ui-decode-001)
**Description:** Text is drawn in 8x16 characters taken from the Spleen 8x16 font (BSD-2-Clause). The font holds only the 32 visible characters of the [UI-DECODE-001](#ui-decode-001) set and of the waiting text `Waiting for data` ([FUNC-DECODE-007](functional.md#func-decode-007)), which adds `W g i n o r t`; `°` is the degree sign. Space and any other character draw blank. Examples: space, `A`, `C`, `s` and the null character draw blank.

### UI-DECODE-003
**Title:** Big characters are the 8x16 glyphs doubled in width and height; the lux unit cell holds `lx` in plain 8x16 glyphs
**Status:** validated
**Dependencies:** [UI-DECODE-002](#ui-decode-002)
**Description:** A big 16x32 character is its 8x16 glyph ([UI-DECODE-002](#ui-decode-002)) with every pixel doubled horizontally and vertically, so big characters need no font of their own. The lux unit cell of [UI-DECODE-001](#ui-decode-001) is not doubled: its upper half is blank, and its lower half shows the 8x16 glyph of `l` in pixel columns 0–7 and that of `x` in columns 8–15.