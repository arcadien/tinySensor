#pragma once

#include <Hal.h>

// One measurement/transmission cycle, all I/O through `hal` (TECH-MAIN-001).
void runCycle(Hal *hal);
