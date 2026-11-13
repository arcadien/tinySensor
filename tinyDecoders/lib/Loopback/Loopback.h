#pragma once

#include <stdint.h>
#include <vector>

// Level held on the radio line for a duration (TECH-DECODE-001)
struct Pulse {
  bool high;
  uint16_t durationUs;
};

// Converts TestHal_ Orders tokens into pulses, using nominal delay durations
std::vector<Pulse> OrdersToPulses(const std::vector<unsigned char> &orders);
