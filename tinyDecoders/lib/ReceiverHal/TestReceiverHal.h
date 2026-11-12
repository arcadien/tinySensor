#pragma once

#include <stdint.h>
#include <string.h>

#include <deque>
#include <string>
#include <vector>

// Native ReceiverHal double: queued pulses, settable clock, drawn 3 x 16
// text pages and 6-character big line, recorded RFLink lines (TECH-DECODE-002)
class TestReceiverHal {
 public:
  static const bool HasLineOutput = true;

  TestReceiverHal() {
    memset(screen, ' ', sizeof screen);
    memset(big, ' ', sizeof big);
  }

  bool PopPulse(bool &high, uint16_t &durationUs) {
    if (pulses.empty())
      return false;
    high = pulses.front().high;
    durationUs = pulses.front().durationUs;
    pulses.pop_front();
    return true;
  }
  uint32_t NowSeconds() const { return nowSeconds; }
  void DrawChar(uint8_t page, uint8_t col, char c) {
    screen[page][col] = c;
    drawCount++;
  }
  void DrawBigChar(uint8_t col, char c) {
    big[col] = c;
    drawCount++;
  }
  void WriteLine(const char *line) { lines.push_back(line); }

  void PushPulse(bool high, uint16_t durationUs) {
    pulses.push_back({high, durationUs});
  }
  void SetNowSeconds(uint32_t s) { nowSeconds = s; }
  // 16 characters, not '\0'-terminated
  const char *Row(uint8_t page) const { return screen[page]; }
  const char *Big() const { return big; }
  uint16_t DrawCount() const { return drawCount; }
  const std::vector<std::string> &Lines() const { return lines; }

 private:
  struct QueuedPulse {
    bool high;
    uint16_t durationUs;
  };

  std::deque<QueuedPulse> pulses;
  uint32_t nowSeconds = 0;
  char screen[3][16];  // by start page
  char big[6];
  uint16_t drawCount = 0;
  std::vector<std::string> lines;
};
