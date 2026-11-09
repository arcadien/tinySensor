#include "Loopback.h"

static uint32_t tokenDurationUs(unsigned char token) {
  switch (token) {
    case 'A':
      return 400;
    case 'D':
      return 512;
    case 'P':
      return 1024;
    case 'T':
      return 560;
    case 'U':
      return 1120;
    case 'Q':
      return 8960;
    case 'R':
      return 4500;
    case 'G':
      return 40000;
    default:
      return 0;
  }
}

std::vector<Pulse> OrdersToPulses(const std::vector<unsigned char> &orders) {
  std::vector<Pulse> pulses;
  bool high = false;
  uint32_t durationUs = 0;
  auto emit = [&]() {
    if (durationUs > 0) {
      pulses.push_back({high, static_cast<uint16_t>(durationUs)});
    }
    durationUs = 0;
  };
  for (unsigned char token : orders) {
    if ((token == 'H' || token == 'L') && (token == 'H') != high) {
      emit();
      high = (token == 'H');
    } else {
      durationUs += tokenDurationUs(token);
      if (durationUs > UINT16_MAX) {
        durationUs = UINT16_MAX;
      }
    }
  }
  emit();
  return pulses;
}
