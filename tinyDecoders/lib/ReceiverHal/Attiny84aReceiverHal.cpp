#ifdef AVR
#include "Attiny84aReceiverHal.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

namespace {

struct CapturedPulse {
  uint16_t durationUs;
  bool high;
};

const uint8_t PulseCapacity = 32;  // power of two, indexes wrap with a mask
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

}  // namespace

void Attiny84aReceiverHal::Init() {
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
