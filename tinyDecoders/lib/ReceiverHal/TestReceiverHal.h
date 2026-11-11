#pragma once

#include <stdint.h>
#include <string.h>

#include <deque>
#include <string>
#include <vector>

// Native ReceiverHal double: queued pulses, settable clock, drawn 2 x 16
// screen, recorded RFLink lines (TECH-DECODE-002)
class TestReceiverHal {
 public:
  static const bool HasLineOutput = true;

  TestReceiverHal() { memset(screen, ' ', sizeof screen); }

  bool PopPulse(bool &high, uint16_t &durationUs) {
    if (pulses.empty())
      return false;
    high = pulses.front().high;
    durationUs = pulses.front().durationUs;
    pulses.pop_front();
    return true;
  }
  uint32_t NowSeconds() const { return nowSeconds; }
  void DrawChar(uint8_t row, uint8_t col, char c) {
    screen[row][col] = c;
    drawCount++;
  }
  void WriteLine(const char *line) { lines.push_back(line); }

  void PushPulse(bool high, uint16_t durationUs) {
    pulses.push_back({high, durationUs});
  }
  void SetNowSeconds(uint32_t s) { nowSeconds = s; }
  // 16 characters, not '\0'-terminated
  const char *Row(uint8_t row) const { return screen[row]; }
  uint16_t DrawCount() const { return drawCount; }
  const std::vector<std::string> &Lines() const { return lines; }

 private:
  struct QueuedPulse {
    bool high;
    uint16_t durationUs;
  };

  std::deque<QueuedPulse> pulses;
  uint32_t nowSeconds = 0;
  char screen[2][16];
  uint16_t drawCount = 0;
  std::vector<std::string> lines;
};
