# UI Requirements

<!-- Requirements are added here during Phase 2 of the RBD workflow. -->
<!-- Format: req(ID): title -->

### UI-DECODE-001
**Title:** FormatPage renders one source record and its age as two 16-character lines
**Status:** validated
**Dependencies:** FUNC-DECODE-006
**Description:** Free function `void FormatPage(const SourceRecord& record, uint32_t ageSeconds, char line1[17], char line2[17])` in `tinyDecoders/lib/PageFormatter/PageFormatter.h` (no dynamic allocation, no `printf`) writes exactly 16 characters plus `'\0'` per line, space-filled. Line 1: the present items among temperature, humidity, lux, millivolts, in that order, separated by one space from column 0; the label ends at column 16. Line 2: pressure, if present, from column 0; the age ends at column 16. Items: temperature `[-]I.T` then `'\x7F'` (degree glyph) and `C` (−5 gives `-0.5`); humidity `(h+5)/10` capped at 99, then `%`; pressure `(p+5)/10` then `hPa`; lux in decimal then `lx`; millivolts as `(mV+5)/10` hundredths of volt `I.FF` then `V`. Label: `Ore` (Oregon), `Lac` (Lacrosse, LacrosseLux) or `X` (X10) followed by `id` in decimal. Age: below 60 `<s>s`, below 3600 `<s/60>m`, below 86400 `<s/3600>h`, else `<min(s/86400, 99)>d`. Output uses only `0-9`, space, `.`, `-`, `%`, `'\x7F'`, `C O r e L a c X h P l x V s m d`. Test (`°` = `'\x7F'`): Oregon 1, 215, 600, 10130, 12 s gives `21.5°C 60%  Ore1` / `1013hPa      12s`; Lacrosse 3, −102, 425, 300 s gives `-10.2°C 43% Lac3` / `              5m`; X10 10, 2950 mV, 7200 s gives `2.95V        X10` / `              2h`; humidity 999 gives `99%`; 9 000 000 s gives `99d`.
