#include "Receiver.h"

#include <string.h>

#include <PageFormatter.h>

Receiver::Receiver(ReceiverHal *hal) : hal(hal) {
  memset(shown, ' ', sizeof shown);
}

void Receiver::Poll() {
  bool decoded = Receive();
  uint32_t now = hal->NowSeconds();
  if (decoded || !composed || now != composedAt)
    Compose(now);
  DrawFirstChange();
}

// Feeds every pending pulse to all decoders; true when a frame was stored
bool Receiver::Receive() {
  uint32_t now = hal->NowSeconds();
  bool decoded = false;
  bool high;
  uint16_t durationUs;
  while (hal->PopPulse(high, durationUs)) {
    if (x10.Feed(high, durationUs)) {
      sources.Update(x10, now);
      decoded = true;
    }
    if (lacrosse.Feed(high, durationUs)) {
      sources.Update(lacrosse, now);
      decoded = true;
    }
    if (oregon.Feed(high, durationUs)) {
      sources.Update(oregon, now);
      decoded = true;
    }
  }
  return decoded;
}

void Receiver::Compose(uint32_t now) {
  composed = true;
  composedAt = now;
  if (sources.Count() == 0) {
    // A record without fields gives only its label and the right-aligned age
    SourceRecord none = {};
    FormatPage(none, now, lines[0], lines[1]);
    memcpy(lines[0], "Ore Lac X       ", Columns);
    return;
  }
  const SourceRecord &record =
      sources.At(now / SecondsPerPage % sources.Count());
  FormatPage(record, now - record.lastSeenSeconds, lines[0], lines[1]);
}

void Receiver::DrawFirstChange() {
  for (uint8_t row = 0; row < Rows; row++) {
    for (uint8_t col = 0; col < Columns; col++) {
      if (lines[row][col] != shown[row][col]) {
        hal->DrawChar(row, col, lines[row][col]);
        shown[row][col] = lines[row][col];
        return;
      }
    }
  }
}
