#pragma once

#include <stdint.h>

#include <SourceTable.h>

// Renders one source record and its age as two 16-character lines plus '\0'
// (UI-DECODE-001): line 1 values then the source label, line 2 pressure and
// the Oregon rolling code then the age, label and age right-aligned.
void FormatPage(const SourceRecord &record, uint32_t ageSeconds, char line1[17],
                char line2[17]);
