#pragma once

#include <stdint.h>

#include <LacrosseDecoder.h>
#include <OregonDecoder.h>
#include <ReceiverHal.h>
#include <RflinkFormatter.h>
#include <SourceTable.h>
#include <X10Decoder.h>

// Writes one RFLink line per decoded frame with a wrapping counter
// (FUNC-DECODE-009)
template <bool HasLineOutput>
class RflinkOutput {
 protected:
  template <class Decoder>
  void WriteRflink(ReceiverHal *hal, const Decoder &decoder) {
    char line[RflinkLineSize];
    FormatRflink(decoder, lineCounter++, line);
    hal->WriteLine(line);
  }

 private:
  uint8_t lineCounter = 0;
};

// No line output: empty base, so neither the counter nor the formatter cost
// RAM or flash
template <>
class RflinkOutput<false> {
 protected:
  template <class Decoder>
  void WriteRflink(ReceiverHal *, const Decoder &) {}
};

// Decodes received frames and shows one page per source, cycling every 10 s,
// or a waiting page while no source is known (FUNC-DECODE-007). Draws at most
// one cell per Poll so the pulse buffer is drained often.
class Receiver : private RflinkOutput<ReceiverHal::HasLineOutput> {
 public:
  explicit Receiver(ReceiverHal *hal);
  void Poll();

 private:
  // Source cells: big 0-5, top 6-9, bottom 10-13
  static const uint8_t SourceCells = 14;
  static const uint8_t WaitingCells = 16;
  static const uint8_t SecondsPerPage = 10;
  static const uint8_t SecondsPerAlternate = 2;

  bool Receive();
  void Compose(uint32_t now);
  char Target(uint8_t cell) const;
  void DrawNextCell();

  ReceiverHal *hal;
  X10Decoder x10;
  LacrosseDecoder lacrosse;
  OregonDecoder oregon;
  SourceTable sources;
  // Page to show; all spaces on the waiting page
  char big[7];
  char top[5];
  char bottom[5];
  char drawn[SourceCells];  // source cells as on the screen
  uint16_t forced = 0;      // bit cell: source cell to draw even if unchanged
  uint8_t nextWaitingCell = WaitingCells;  // WaitingCells: nothing to draw
  bool waiting = false;
  bool composed = false;
  uint32_t composedAt = 0;
};
