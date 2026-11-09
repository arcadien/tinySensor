#include "SourceTable.h"

static const uint16_t OregonThgn123n = 0x1D20;
static const uint16_t OregonBthr918 = 0x5A5D;

static const uint8_t LacrosseHumidityType = 1;
static const uint8_t LacrossePressureType = 4;
static const uint8_t LacrosseLuxType = 5;

static const uint32_t MaxBcdValue = 0x9999;

void SourceTable::Update(const OregonDecoder &decoder, uint32_t nowSeconds) {
  SourceRecord record = {SourceRecord::Oregon,
                         decoder.Channel(),
                         SourceRecord::Temperature,
                         decoder.TemperatureTenths(),
                         0,
                         0,
                         0,
                         0,
                         nowSeconds};
  uint16_t device = decoder.DeviceId();
  if (device == OregonThgn123n || device == OregonBthr918) {
    record.fields |= SourceRecord::Humidity;
    record.humidityTenths = decoder.HumidityPercent() * 10;
  }
  if (device == OregonBthr918) {
    record.fields |= SourceRecord::Pressure;
    record.pressureTenths = decoder.PressureHpa() * 10;
  }
  Store(record);
}

void SourceTable::Update(const LacrosseDecoder &decoder, uint32_t nowSeconds) {
  SourceRecord record = {
      SourceRecord::Lacrosse, decoder.Address(), 0, 0, 0, 0, 0, 0, nowSeconds};
  uint8_t type = decoder.Type();
  if (type == LacrosseLuxType) {
    record.kind = SourceRecord::LacrosseLux;
    record.fields = SourceRecord::Lux;
    record.lux = decoder.Lux();
  } else {
    record.fields = SourceRecord::Temperature;
    record.temperatureTenths = decoder.TemperatureTenths();
  }
  if (type == LacrosseHumidityType || type == LacrossePressureType) {
    record.fields |= SourceRecord::Humidity;
    record.humidityTenths = decoder.HumidityTenths();
  }
  if (type == LacrossePressureType) {
    record.fields |= SourceRecord::Pressure;
    record.pressureTenths = decoder.PressureTenths();
  }
  Store(record);
}

// The X10 value carries millivolts as 4 BCD digits; anything else is no field
void SourceTable::Update(const X10Decoder &decoder, uint32_t nowSeconds) {
  SourceRecord record = {SourceRecord::X10, decoder.Address(), 0, 0, 0, 0, 0, 0,
                         nowSeconds};
  uint32_t bcd = decoder.Value();
  bool valid = bcd <= MaxBcdValue;
  uint16_t millivolts = 0;
  for (int8_t shift = 12; shift >= 0; shift -= 4) {
    uint8_t digit = (bcd >> shift) & 0x0F;
    valid = valid && digit <= 9;
    millivolts = millivolts * 10 + digit;
  }
  if (valid) {
    record.fields = SourceRecord::Millivolts;
    record.millivolts = millivolts;
  }
  Store(record);
}

void SourceTable::Store(const SourceRecord &record) {
  uint8_t slot = SlotFor(record);
  if (slot == count)
    count++;
  records[slot] = record;
}

// The source's own slot, else the next free one, else the least recently seen
uint8_t SourceTable::SlotFor(const SourceRecord &record) const {
  for (uint8_t i = 0; i < count; i++) {
    if (records[i].kind == record.kind && records[i].id == record.id)
      return i;
  }
  if (count < kCapacity)
    return count;
  uint8_t oldest = 0;
  for (uint8_t i = 1; i < count; i++) {
    if (records[i].lastSeenSeconds < records[oldest].lastSeenSeconds)
      oldest = i;
  }
  return oldest;
}
