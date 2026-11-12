#include "Receiver.h"

#include <string.h>

#include <PageFormatter.h>

#ifdef AVR
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(address) (*(const uint8_t *)(address))
#endif

static const char WaitingText[] PROGMEM = "Waiting for data";
static const uint8_t WaitingPage = 1;
static const uint8_t TopPage = 0;
static const uint8_t BottomPage = 2;
static const uint8_t SideColumn = 12;
static const uint16_t AllSourceCells = (1u << 14) - 1;

Receiver::Receiver(ReceiverHal *hal) : hal(hal) {
  // The screen starts cleared
  memset(drawn, ' ', sizeof drawn);
}

void Receiver::Poll() {
  bool decoded = Receive();
  uint32_t now = hal->NowSeconds();
  if (decoded || !composed || now != composedAt)
    Compose(now);
  DrawNextCell();
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

// A layout switch redraws every cell of the new layout: entering the waiting
// page from a source page blanks the source cells first, while entering a
// source page covers the whole screen, waiting text included
void Receiver::Compose(uint32_t now) {
  sources.Expire(now);
  bool empty = sources.Count() == 0;
  if (empty) {
    memset(big, ' ', sizeof big - 1);
    memset(top, ' ', sizeof top - 1);
    memset(bottom, ' ', sizeof bottom - 1);
    if (!composed || !waiting) {
      if (composed)
        forced = AllSourceCells;
      nextWaitingCell = 0;
    }
  } else {
    const SourceRecord &record =
        sources.At(now / SecondsPerPage % sources.Count());
    FormatPage(record, now / SecondsPerAlternate % 2 == 1, big, top, bottom);
    if (!composed || waiting) {
      forced = AllSourceCells;
      nextWaitingCell = WaitingCells;
    }
  }
  waiting = empty;
  composed = true;
  composedAt = now;
}

char Receiver::Target(uint8_t cell) const {
  if (cell < 6)
    return big[cell];
  if (cell < 10)
    return top[cell - 6];
  return bottom[cell - 10];
}

void Receiver::DrawNextCell() {
  for (uint8_t cell = 0; cell < SourceCells; cell++) {
    char c = Target(cell);
    uint16_t bit = 1u << cell;
    if (!(forced & bit) && c == drawn[cell])
      continue;
    forced &= ~bit;
    drawn[cell] = c;
    if (cell < 6)
      hal->DrawBigChar(cell, c);
    else if (cell < 10)
      hal->DrawChar(TopPage, SideColumn + cell - 6, c);
    else
      hal->DrawChar(BottomPage, SideColumn + cell - 10, c);
    return;
  }
  if (nextWaitingCell < WaitingCells) {
    hal->DrawChar(WaitingPage, nextWaitingCell,
                  pgm_read_byte(&WaitingText[nextWaitingCell]));
    nextWaitingCell++;
  }
}
