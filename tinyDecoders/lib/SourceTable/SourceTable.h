#pragma once

#include <stdint.h>

#include <LacrosseDecoder.h>
#include <OregonDecoder.h>
#include <X10Decoder.h>

// Last decoded values of one radio source, in common units (FUNC-DECODE-006).
// Values whose bit is clear in `fields` are 0.
struct SourceRecord {
  enum Kind : uint8_t { Oregon, Lacrosse, LacrosseLux, X10 };
  enum : uint8_t {
    Temperature = 1,
    Humidity = 2,
    Pressure = 4,
    Lux = 8,
    Millivolts = 16
  };

  Kind kind;
  uint8_t id;
  uint8_t code;  // Oregon rolling code, 0 for the other kinds
  uint8_t fields;
  int16_t temperatureTenths;
  uint16_t humidityTenths;
  uint16_t pressureTenths;
  uint16_t lux;
  uint16_t millivolts;
  uint32_t lastSeenSeconds;
};

// Keeps the last record of each source (kind, id, code), in first-seen order.
// When full, the least recently seen source is replaced.
class SourceTable {
 public:
#ifdef AVR
  static const uint8_t kCapacity = 16;
#else
  static const uint8_t kCapacity = 8;
#endif

  // Each called after that decoder's Feed returned true
  void Update(const OregonDecoder &decoder, uint32_t nowSeconds);
  void Update(const LacrosseDecoder &decoder, uint32_t nowSeconds);
  void Update(const X10Decoder &decoder, uint32_t nowSeconds);

  uint8_t Count() const { return count; }
  const SourceRecord &At(uint8_t index) const { return records[index]; }

 private:
  void Store(const SourceRecord &record);
  uint8_t SlotFor(const SourceRecord &record) const;

  SourceRecord records[kCapacity];
  uint8_t count = 0;
};
