#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)
#include "AtmegaReceiverHal.h"

#include <Arduino.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <util/atomic.h>

#include <Font8x16.h>

namespace {

// ICP1, the only pin-level difference between the two boards
#ifdef __AVR_ATmega328P__
#define ICP1_DDR DDRB
#define ICP1_PORT PORTB
#define ICP1_PIN PINB
#define ICP1_BIT PB0  // Pro Mini D8
#else
#define ICP1_DDR DDRD
#define ICP1_PORT PORTD
#define ICP1_PIN PIND
#define ICP1_BIT PD4  // Pro Micro pin 4
#endif

struct CapturedPulse {
  uint16_t durationUs;
  bool high;
};

// Absorbs the I2C and USB waits; power of two, indexes wrap with a mask
const uint8_t PulseCapacity = 64;
const uint32_t MicrosPerOverflow = 32768;  // 65536 ticks of 0.5 us
const uint32_t MicrosPerSecond = 1000000;

volatile CapturedPulse pulses[PulseCapacity];
volatile uint8_t firstPulse = 0;
volatile uint8_t pulseCount = 0;

volatile uint16_t lastCapture = 0;
// 2 so that the first pulse, of unknown start, is clamped to 65535
volatile uint8_t overflowsSinceCapture = 2;

volatile uint32_t overflowMicros = 0;
volatile uint32_t seconds = 0;

const uint8_t OledAddress = 0x3C;
const uint8_t OledCommands = 0x00;  // control byte: command stream follows
const uint8_t OledData = 0x40;      // control byte: data stream follows
const uint8_t OledPages = 4;
const uint8_t OledColumns = 128;
const uint8_t OledBytesPerWrite = 8;  // within the 32-byte Wire buffer
const uint8_t OledDisplayOn = 0xAF;
const uint32_t OledClockHz = 400000;

// SSD1306 128x32 init (PLAT-DECODE-004): display off,
// clock, 32 lines, no offset, charge pump, page addressing, remapped segments
// and rows, contrast, precharge, VCOMH, display RAM on
const uint8_t OledInit[] PROGMEM = {
    0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02,
    0xA1, 0xC8, 0xDA, 0x02, 0x81, 0x8F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6};

const unsigned long SerialBaud = 57600;  // ignored by USB CDC, rflink default

void SendCommand(uint8_t command) {
  Wire.beginTransmission(OledAddress);
  Wire.write(OledCommands);
  Wire.write(command);
  Wire.endTransmission();
}

// Places the next data byte at `page`, `column`
void Locate(uint8_t page, uint8_t column) {
  Wire.beginTransmission(OledAddress);
  Wire.write(OledCommands);
  Wire.write(0xB0 + page);
  Wire.write(column & 0x0F);
  Wire.write(0x10 | (column >> 4));
  Wire.endTransmission();
}

void SendData(const uint8_t *bytes) {
  Wire.beginTransmission(OledAddress);
  Wire.write(OledData);
  Wire.write(bytes, OledBytesPerWrite);
  Wire.endTransmission();
}

void InitDisplay() {
  Wire.begin();
  Wire.setClock(OledClockHz);
  Wire.beginTransmission(OledAddress);
  Wire.write(OledCommands);
  for (uint8_t i = 0; i < sizeof OledInit; i++)
    Wire.write(pgm_read_byte(&OledInit[i]));
  Wire.endTransmission();

  const uint8_t blank[OledBytesPerWrite] = {};
  for (uint8_t page = 0; page < OledPages; page++) {
    Locate(page, 0);
    for (uint8_t column = 0; column < OledColumns; column += OledBytesPerWrite)
      SendData(blank);
  }
  SendCommand(OledDisplayOn);
}

}  // namespace

void AtmegaReceiverHal::Init() {
  InitDisplay();
  Serial.begin(SerialBaud);

  // RXB6 at 5 V drives ICP1 directly: input, no pull-up
  ICP1_DDR &= ~(1 << ICP1_BIT);
  ICP1_PORT &= ~(1 << ICP1_BIT);
  // The Arduino init() leaves Timer1 in 8-bit PWM: reconfigure it fully.
  // Normal mode, noise canceller, prescaler 8, first capture on the edge
  // leaving the current level
  uint8_t risingEdge = (ICP1_PIN & (1 << ICP1_BIT)) ? 0 : (1 << ICES1);
  TCCR1A = 0;
  TCCR1C = 0;
  TCCR1B = (1 << ICNC1) | (1 << CS11) | risingEdge;
  TIFR1 = (1 << ICF1) | (1 << TOV1);
  TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
}

bool AtmegaReceiverHal::PopPulse(bool &high, uint16_t &durationUs) {
  bool popped = false;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (pulseCount > 0) {
      high = pulses[firstPulse].high;
      durationUs = pulses[firstPulse].durationUs;
      firstPulse = (firstPulse + 1) & (PulseCapacity - 1);
      pulseCount--;
      popped = true;
    }
  }
  return popped;
}

uint32_t AtmegaReceiverHal::NowSeconds() const {
  uint32_t now;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    now = seconds;
  }
  return now;
}

void AtmegaReceiverHal::DrawChar(uint8_t row, uint8_t col, char c) {
  uint8_t glyph[16];
  ReadGlyph(c, glyph);
  for (uint8_t half = 0; half < 2; half++) {
    Locate(2 * row + half, col * 8);
    SendData(&glyph[half * OledBytesPerWrite]);
  }
}

// Never waits for the host: the Serial operator bool would block 10 ms
void AtmegaReceiverHal::WriteLine(const char *line) {
  size_t length = strlen(line);
  if ((size_t)Serial.availableForWrite() < length + 2)
    return;  // no room in the USB endpoint: drop the line
  Serial.write(line, length);
  Serial.write("\r\n");
}

ISR(TIMER1_CAPT_vect) {
  uint16_t capture = ICR1;
  bool rose = TCCR1B & (1 << ICES1);
  TCCR1B ^= (1 << ICES1);
  TIFR1 = (1 << ICF1);  // an edge change may raise ICF1 (datasheet)

  // shortcut: an overflow pending while this ISR runs is not counted yet, so
  // only pulses within a few us of 32.8 ms may be misclamped
  bool fullPeriod = overflowsSinceCapture > 1 ||
                    (overflowsSinceCapture == 1 && capture >= lastCapture);
  uint16_t durationUs =
      fullPeriod ? 65535 : (uint16_t)(capture - lastCapture) / 2;
  lastCapture = capture;
  overflowsSinceCapture = 0;

  if (pulseCount == PulseCapacity)
    return;  // full: drop the pulse
  uint8_t last = (firstPulse + pulseCount) & (PulseCapacity - 1);
  pulses[last].high = !rose;
  pulses[last].durationUs = durationUs;
  pulseCount++;
}

ISR(TIMER1_OVF_vect) {
  if (overflowsSinceCapture < 2)
    overflowsSinceCapture++;
  overflowMicros += MicrosPerOverflow;
  if (overflowMicros >= MicrosPerSecond) {
    overflowMicros -= MicrosPerSecond;
    seconds++;
  }
}
#endif
