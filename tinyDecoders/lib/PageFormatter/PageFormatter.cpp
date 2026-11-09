#include "PageFormatter.h"

static const uint8_t LineLength = 16;
static const char Degree = '\x7F';

static const uint32_t SecondsPerMinute = 60;
static const uint32_t SecondsPerHour = 3600;
static const uint32_t SecondsPerDay = 86400;
static const uint32_t MaxDays = 99;
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
  at = PutChar(at, Degree);
  return PutChar(at, 'C');
}

static char *PutHumidity(char *at, uint16_t tenths) {
  uint32_t percent = RoundTenths(tenths);
  at = PutNumber(at,
                 percent > MaxHumidityPercent ? MaxHumidityPercent : percent);
  return PutChar(at, '%');
}

// Volts with two decimals, from millivolts rounded to the nearest hundredth
static char *PutVolts(char *at, uint16_t millivolts) {
  uint16_t hundredths = (millivolts + 5u) / 10;
  at = PutNumber(at, hundredths / 100);
  at = PutChar(at, '.');
  at = PutNumber(at, hundredths / 10 % 10);
  at = PutNumber(at, hundredths % 10);
  return PutChar(at, 'V');
}

static char *PutLabel(char *at, const SourceRecord &record) {
  if (record.kind == SourceRecord::Oregon)
    at = PutText(at, "Ore");
  else if (record.kind == SourceRecord::X10)
    at = PutChar(at, 'X');
  else
    at = PutText(at, "Lac");
  return PutNumber(at, record.id);
}

static char *PutAge(char *at, uint32_t seconds) {
  if (seconds < SecondsPerMinute)
    return PutChar(PutNumber(at, seconds), 's');
  if (seconds < SecondsPerHour)
    return PutChar(PutNumber(at, seconds / SecondsPerMinute), 'm');
  if (seconds < SecondsPerDay)
    return PutChar(PutNumber(at, seconds / SecondsPerHour), 'h');
  uint32_t days = seconds / SecondsPerDay;
  return PutChar(PutNumber(at, days > MaxDays ? MaxDays : days), 'd');
}

// Starts the next left item, one space after the previous one
static char *Separate(const char *line, char *at) {
  return at == line ? at : PutChar(at, ' ');
}

// Copies the text [begin, end) so that it ends at the last column
static void PutRight(char *line, const char *begin, const char *end) {
  char *at = line + LineLength - (end - begin);
  while (begin != end) *at++ = *begin++;
}

static void Clear(char *line) {
  for (uint8_t i = 0; i < LineLength; i++) line[i] = ' ';
  line[LineLength] = '\0';
}

// Item widths are bounded by the decoded ranges, so items and right-aligned
// text never overlap or overflow the 16 columns
void FormatPage(const SourceRecord &record, uint32_t ageSeconds, char line1[17],
                char line2[17]) {
  Clear(line1);
  char *at = line1;
  if (record.fields & SourceRecord::Temperature)
    at = PutTemperature(Separate(line1, at), record.temperatureTenths);
  if (record.fields & SourceRecord::Humidity)
    at = PutHumidity(Separate(line1, at), record.humidityTenths);
  if (record.fields & SourceRecord::Lux)
    at = PutText(PutNumber(Separate(line1, at), record.lux), "lx");
  if (record.fields & SourceRecord::Millivolts)
    at = PutVolts(Separate(line1, at), record.millivolts);
  char label[8];
  PutRight(line1, label, PutLabel(label, record));

  Clear(line2);
  if (record.fields & SourceRecord::Pressure)
    PutText(PutNumber(line2, RoundTenths(record.pressureTenths)), "hPa");
  char age[4];
  PutRight(line2, age, PutAge(age, ageSeconds));
}
