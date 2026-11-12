#include "Receiver.h"

#include <string.h>

#include <PageFormatter.h>

Receiver::Receiver(ReceiverHal *hal) : hal(hal) {
  // The screen starts cleared: blank target lines, nothing to draw
  memset(lines, ' ', sizeof lines);
}

void Receiver::Poll() {
  bool decoded = Receive();
  uint32_t now = hal->NowSeconds();
  if (decoded || !composed || now != composedAt) {
    char previous[Rows][Columns + 1];
    memcpy(previous, lines, sizeof lines);
    Compose(now);
    MarkChanges(previous);
  }
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
      WriteRflink(hal, x10);
      decoded = true;
    }
    if (lacrosse.Feed(high, durationUs)) {
      sources.Update(lacrosse, now);
      WriteRflink(hal, lacrosse);
      decoded = true;
    }
    if (oregon.Feed(high, durationUs)) {
      sources.Update(oregon, now);
      WriteRflink(hal, oregon);
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
    // Not Oregon, so line 2 holds only the uptime (no rolling code)
    SourceRecord none = {SourceRecord::X10};
    FormatPage(none, now, lines[0], lines[1]);
    memcpy(lines[0], "Ore Lac X       ", Columns);
    return;
  }
  const SourceRecord &record =
      sources.At(now / SecondsPerPage % sources.Count());
  FormatPage(record, now - record.lastSeenSeconds, lines[0], lines[1]);
}

void Receiver::MarkChanges(const char previous[Rows][Columns + 1]) {
  for (uint8_t row = 0; row < Rows; row++)
    for (uint8_t col = 0; col < Columns; col++)
      if (lines[row][col] != previous[row][col])
        toDraw |= (uint32_t)1 << (row * Columns + col);
}

void Receiver::DrawFirstChange() {
  for (uint8_t bit = 0; bit < Rows * Columns; bit++) {
    if (toDraw & ((uint32_t)1 << bit)) {
      toDraw &= ~((uint32_t)1 << bit);
      uint8_t row = bit / Columns;
      uint8_t col = bit % Columns;
      hal->DrawChar(2 * row, col, lines[row][col]);
      return;
    }
  }
}
