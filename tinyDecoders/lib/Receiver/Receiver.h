#pragma once

#include <stdint.h>

#include <LacrosseDecoder.h>
#include <OregonDecoder.h>
#include <ReceiverHal.h>
#include <SourceTable.h>
#include <X10Decoder.h>

// Decodes received frames and shows one page per source, cycling every 4 s
// (FUNC-DECODE-007). Draws at most one character per Poll so the pulse
// buffer is drained often.
class Receiver {
 public:
  explicit Receiver(ReceiverHal *hal);
  void Poll();

 private:
  static const uint8_t Rows = 2;
  static const uint8_t Columns = 16;
  static const uint8_t SecondsPerPage = 4;

  bool Receive();
  void Compose(uint32_t now);
  void MarkChanges(const char previous[Rows][Columns + 1]);
  void DrawFirstChange();

  ReceiverHal *hal;
  X10Decoder x10;
  LacrosseDecoder lacrosse;
  OregonDecoder oregon;
  SourceTable sources;
  char lines[Rows][Columns + 1];  // page to show, '\0' left by FormatPage
  uint32_t toDraw = 0;  // bit row * Columns + col: character not drawn yet
  bool composed = false;
  uint32_t composedAt = 0;
};
