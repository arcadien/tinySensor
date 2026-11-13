#pragma once

#include <stdint.h>

#include <SourceTable.h>

// Renders one source record as a 6-character big main value, left-aligned,
// and two 4-character side texts, right-aligned: `top` the source identifier,
// `bottom` humidity or pressure, pressure when both and `alternate`
// (UI-DECODE-001). Each buffer is space-filled and '\0'-terminated.
void FormatPage(const SourceRecord &record, bool alternate, char big[7],
                char top[5], char bottom[5]);
