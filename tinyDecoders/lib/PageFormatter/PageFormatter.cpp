#include "PageFormatter.h"

static const uint8_t BigLength = 6;
static const uint8_t SideLength = 4;
static const char Degree = '\x7F';
static const uint8_t MaxHumidityPercent = 99;

// Each writer puts its text at `at` and returns the position after it

static char *PutChar(char *at, char c) {
  *at = c;
  return at + 1;
}

static char *PutText(char *at, const char *text) {
  while (*text) *at++ = *text++;
  return at;
}

// Lowercase hex digit, computed: a digit table would cost 17 B of AVR RAM
static char *PutHexDigit(char *at, uint8_t digit) {
  return PutChar(at, digit < 10 ? '0' + digit : 'a' + digit - 10);
}

static char *PutNumber(char *at, uint32_t value) {
  uint8_t digits = 1;
  for (uint32_t rest = value / 10; rest; rest /= 10) digits++;
  char *end = at + digits;
  char *digit = end;
  do {
    *--digit = '0' + value % 10;
    value /= 10;
  } while (value);
  return end;
}

// Rounds tenths to the nearest unit, halves up
static uint32_t RoundTenths(uint16_t tenths) {
  return (tenths + 5u) / 10;
}

static char *PutTemperature(char *at, int16_t tenths) {
  uint16_t magnitude = tenths < 0 ? -tenths : tenths;
  if (tenths < 0)
    at = PutChar(at, '-');
  at = PutNumber(at, magnitude / 10);
  at = PutChar(at, '.');
  at = PutNumber(at, magnitude % 10);
  return PutChar(at, Degree);
}

static char *PutHumidity(char *at, uint16_t tenths) {
  uint32_t percent = RoundTenths(tenths);
  at = PutNumber(at,
                 percent > MaxHumidityPercent ? MaxHumidityPercent : percent);
  return PutChar(at, '%');
}

// Volts with two decimals, from millivolts rounded to the nearest hundredth
static char *PutVolts(char *at, uint16_t millivolts) {
  uint32_t hundredths = (millivolts + 5u) / 10;
  at = PutNumber(at, hundredths / 100);
  at = PutChar(at, '.');
  at = PutNumber(at, hundredths / 10 % 10);
  at = PutNumber(at, hundredths % 10);
  return PutChar(at, 'V');
}

static char *PutIdentifier(char *at, const SourceRecord &record) {
  if (record.kind == SourceRecord::Oregon) {
    at = PutChar(PutNumber(at, record.id), '-');
    return PutHexDigit(PutHexDigit(at, record.code >> 4), record.code & 0x0F);
  }
  return PutNumber(PutChar(at, record.kind == SourceRecord::X10 ? 'X' : 'L'),
                   record.id);
}

// Fills `field` (length characters plus '\0') with the text [begin, end),
// space-padded and aligned; text out of the decoded sensor ranges is clipped
// so a corrupt frame never overflows the buffer
static void PutField(char *field, uint8_t length, const char *begin,
                     const char *end, bool right) {
  uint8_t size = end - begin > length ? length : end - begin;
  for (uint8_t i = 0; i < length; i++) field[i] = ' ';
  field[length] = '\0';
  char *at = right ? field + length - size : field;
  for (uint8_t i = 0; i < size; i++) at[i] = begin[i];
}

void FormatPage(const SourceRecord &record, bool alternate, char big[7],
                char top[5], char bottom[5]) {
  char text[10];  // widest raw item: "-3276.8°"
  char *end = text;
  if (record.fields & SourceRecord::Temperature)
    end = PutTemperature(text, record.temperatureTenths);
  else if (record.fields & SourceRecord::Lux)
    end = PutText(PutNumber(text, record.lux), "lx");
  else if (record.fields & SourceRecord::Millivolts)
    end = PutVolts(text, record.millivolts);
  PutField(big, BigLength, text, end, false);

  PutField(top, SideLength, text, PutIdentifier(text, record), true);

  bool humidity = record.fields & SourceRecord::Humidity;
  end = text;
  if ((record.fields & SourceRecord::Pressure) && (alternate || !humidity))
    end = PutNumber(text, RoundTenths(record.pressureTenths));
  else if (humidity)
    end = PutHumidity(text, record.humidityTenths);
  PutField(bottom, SideLength, text, end, true);
}
