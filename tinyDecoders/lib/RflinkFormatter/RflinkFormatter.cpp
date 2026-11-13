#include "RflinkFormatter.h"

namespace {

// The decoder only accepts EA4C, 1A2D, 1A3D and 5A5D
const uint16_t OregonThn132n = 0xEA4C;
const uint16_t OregonBthr918 = 0x5A5D;

const uint8_t LacrosseHumidityType = 1;
const uint8_t LacrossePressureType = 4;
const uint8_t LacrosseLuxType = 5;

const uint32_t MaxBcdValue = 0x9999;

// Appends to a line buffer sized RflinkLineSize
class Line {
 public:
  Line(char *text, uint8_t counter, const char *name) : text(text) {
    Put("20;");
    Hex(counter, 2, true);
    Put(";");
    Put(name);
    Put(";");
  }

  void Put(const char *s) {
    while (*s) text[length++] = *s++;
  }
  void Hex(uint16_t value, uint8_t digits, bool upper) {
    const char *symbols = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    while (digits-- > 0)
      text[length++] = symbols[(value >> (4 * digits)) & 0xF];
  }
  void Decimal(uint16_t value) {
    char reversed[5];
    uint8_t count = 0;
    do {
      reversed[count++] = '0' + value % 10;
      value /= 10;
    } while (value > 0);
    while (count > 0) text[length++] = reversed[--count];
  }

  void Id(uint16_t id) {
    Put("ID=");
    Hex(id, 4, true);
    Put(";");
  }
  // Absolute tenths, bit 15 set when negative
  void Temperature(int16_t tenths) {
    uint16_t value = tenths < 0 ? (uint16_t)(-tenths) | 0x8000 : tenths;
    Put("TEMP=");
    Hex(value, 4, false);
    Put(";");
  }
  void DecimalField(const char *key, uint16_t value) {
    Put(key);
    Decimal(value);
    Put(";");
  }
  void HexField(const char *key, uint16_t value) {
    Put(key);
    Hex(value, 4, false);
    Put(";");
  }

  uint8_t End() {
    text[length] = '\0';
    return length;
  }

 private:
  char *text;
  uint8_t length = 0;
};

}  // namespace

uint8_t FormatRflink(const X10Decoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]) {
  Line out(line, counter, "X10Meter");
  out.Id(decoder.Address());
  // Millivolts as 4 BCD digits, same rule as SourceTable (FUNC-DECODE-006)
  uint32_t bcd = decoder.Value();
  bool valid = bcd <= MaxBcdValue;
  uint16_t millivolts = 0;
  for (int8_t shift = 12; shift >= 0; shift -= 4) {
    uint8_t digit = (bcd >> shift) & 0x0F;
    valid = valid && digit <= 9;
    millivolts = millivolts * 10 + digit;
  }
  if (valid)
    out.DecimalField("METER=", millivolts);
  return out.End();
}

uint8_t FormatRflink(const LacrosseDecoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]) {
  Line out(line, counter, "LaCrosse WS7000");
  uint8_t type = decoder.Type();
  out.Id((type << 8) | decoder.Address());
  if (type == 0 || type == LacrosseHumidityType || type == LacrossePressureType)
    out.Temperature(decoder.TemperatureTenths());
  if (type == LacrosseHumidityType || type == LacrossePressureType)
    out.DecimalField("HUM=", (decoder.HumidityTenths() + 5) / 10);
  if (type == LacrossePressureType)
    out.HexField("BARO=", (decoder.PressureTenths() + 5) / 10);
  if (type == LacrosseLuxType)
    out.HexField("LUX=", decoder.Lux());
  return out.End();
}

uint8_t FormatRflink(const OregonDecoder &decoder, uint8_t counter,
                     char line[RflinkLineSize]) {
  uint16_t device = decoder.DeviceId();
  const char *name = device == OregonThn132n   ? "Oregon Temp"
                     : device == OregonBthr918 ? "Oregon BTHR"
                                               : "Oregon TempHygro";
  Line out(line, counter, name);
  out.Id((decoder.RollingCode() << 8) | decoder.Channel());
  out.Temperature(decoder.TemperatureTenths());
  if (device != OregonThn132n)
    out.DecimalField("HUM=", decoder.HumidityPercent());
  if (device == OregonBthr918)
    out.HexField("BARO=", decoder.PressureHpa());
  out.Put(decoder.BatteryLow() ? "BAT=LOW;" : "BAT=OK;");
  return out.End();
}
