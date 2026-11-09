#ifdef AVR
#include "Attiny84aReceiverHal.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include <Font8x16.h>
#include <TinyI2CMaster.h>

namespace {

struct CapturedPulse {
  uint16_t durationUs;
  bool high;
};

const uint8_t PulseCapacity = 16;  // power of two, indexes wrap with a mask
const uint32_t MicrosPerOverflow = 65536;
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
const uint8_t OledDisplayOn = 0xAF;

// Display off, clock, 32 lines, no offset, charge pump, page addressing,
// remapped segments and rows, contrast, precharge, VCOMH, display RAM on
const uint8_t OledInit[] PROGMEM = {
    0xAE, 0xD5, 0x80, 0xA8, 0x1F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02,
    0xA1, 0xC8, 0xDA, 0x02, 0x81, 0x8F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6};

void SendCommand(uint8_t command) {
  TinyI2C.start(OledAddress, 0);
  TinyI2C.write(OledCommands);
  TinyI2C.write(command);
  TinyI2C.stop();
}

// Places the next data byte at `page`, `column`
void Locate(uint8_t page, uint8_t column) {
  TinyI2C.start(OledAddress, 0);
  TinyI2C.write(OledCommands);
  TinyI2C.write(0xB0 + page);
  TinyI2C.write(column & 0x0F);
  TinyI2C.write(0x10 | (column >> 4));
  TinyI2C.stop();
}

void InitDisplay() {
  TinyI2C.init();
  TinyI2C.start(OledAddress, 0);
  TinyI2C.write(OledCommands);
  for (uint8_t i = 0; i < sizeof OledInit; i++)
    TinyI2C.write(pgm_read_byte(&OledInit[i]));
  TinyI2C.stop();

  for (uint8_t page = 0; page < OledPages; page++) {
    Locate(page, 0);
    TinyI2C.start(OledAddress, 0);
    TinyI2C.write(OledData);
    for (uint8_t column = 0; column < OledColumns; column++) TinyI2C.write(0);
    TinyI2C.stop();
  }
  SendCommand(OledDisplayOn);
}

}  // namespace

void Attiny84aReceiverHal::Init() {
  InitDisplay();

  DDRA &= ~(1 << PA7);
  PORTA &= ~(1 << PA7);
  TCCR1A = 0;
  // Noise canceller, prescaler 8, first capture on the edge leaving the
  // current level
  uint8_t risingEdge = (PINA & (1 << PINA7)) ? 0 : (1 << ICES1);
  TCCR1B = (1 << ICNC1) | (1 << CS11) | risingEdge;
  TIFR1 = (1 << ICF1) | (1 << TOV1);
  TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
}

bool Attiny84aReceiverHal::PopPulse(bool &high, uint16_t &durationUs) {
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

uint32_t Attiny84aReceiverHal::NowSeconds() const {
  uint32_t now;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    now = seconds;
  }
  return now;
}

void Attiny84aReceiverHal::DrawChar(uint8_t row, uint8_t col, char c) {
  uint8_t glyph[16];
  ReadGlyph(c, glyph);
  for (uint8_t half = 0; half < 2; half++) {
    Locate(2 * row + half, col * 8);
    TinyI2C.start(OledAddress, 0);
    TinyI2C.write(OledData);
    for (uint8_t i = 0; i < 8; i++) TinyI2C.write(glyph[half * 8 + i]);
    TinyI2C.stop();
  }
}

ISR(TIM1_CAPT_vect) {
  uint16_t capture = ICR1;
  bool rose = TCCR1B & (1 << ICES1);
  TCCR1B ^= (1 << ICES1);
  TIFR1 = (1 << ICF1);  // an edge change may raise ICF1 (datasheet)

  // shortcut: an overflow pending while this ISR runs is not counted yet, so
  // only pulses within a few us of 65.5 ms may be misclamped
  bool fullPeriod = overflowsSinceCapture > 1 ||
                    (overflowsSinceCapture == 1 && capture >= lastCapture);
  uint16_t durationUs = fullPeriod ? 65535 : capture - lastCapture;
  lastCapture = capture;
  overflowsSinceCapture = 0;

  if (pulseCount == PulseCapacity)
    return;  // full: drop the pulse
  uint8_t last = (firstPulse + pulseCount) & (PulseCapacity - 1);
  pulses[last].high = !rose;
  pulses[last].durationUs = durationUs;
  pulseCount++;
}

ISR(TIM1_OVF_vect) {
  if (overflowsSinceCapture < 2)
    overflowsSinceCapture++;
  overflowMicros += MicrosPerOverflow;
  if (overflowMicros >= MicrosPerSecond) {
    overflowMicros -= MicrosPerSecond;
    seconds++;
  }
}
#endif
