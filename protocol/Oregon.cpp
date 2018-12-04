#include "oregon.h"
#include "config.h"

uint8_t Oregon::PREAMBLE[] = {0xFF, 0xFF};
uint8_t Oregon::POSTAMBLE[] = {0x00};

void Oregon::txPinLow() { PORTB &= ~_BV(PB0); }

void Oregon::txPinHigh() { PORTB |= _BV(PB0); }

inline void Oregon::sendZero(void) {
  txPinHigh();
  _delay_us(TIME);
  txPinLow();
  _delay_us(TWOTIME);
  txPinHigh();
  _delay_us(TIME);
}

inline void Oregon::sendOne(void) {
  txPinLow();
  _delay_us(TIME);
  txPinHigh();
  _delay_us(TWOTIME);
  txPinLow();
  _delay_us(TIME);
}

inline void Oregon::sendQuarterMSB(const uint8_t data) {
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
}

inline void Oregon::sendQuarterLSB(const uint8_t data) {
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
}

void Oregon::sendData(uint8_t *data, uint8_t size) {
  for (uint8_t i = 0; i < size; ++i) {
    sendQuarterLSB(data[i]);
    sendQuarterMSB(data[i]);
  }
}

void Oregon::sendOregon(uint8_t *data, uint8_t size) {
  sendPreamble();
  // sendSync();
  sendData(data, size);
  sendPostamble();
}

inline void Oregon::sendPreamble(void) { sendData(PREAMBLE, 2); }

inline void Oregon::sendPostamble(void) {

  sendData(POSTAMBLE, 1);

}

inline void Oregon::sendSync(void) { sendQuarterLSB(0xA); }

void Oregon::setType(uint8_t *data, const uint8_t *type) {
  data[0] = type[0];
  data[1] = type[1];
}

void Oregon::setChannel(uint8_t *data, uint8_t channel) { data[2] = channel; }

void Oregon::setId(uint8_t *data, uint8_t ID) { data[3] = ID; }

void Oregon::setBatteryLevel(uint8_t *data, uint8_t level) {
  if (!level)
    data[4] = 0x0C;
  else
    data[4] = 0x00;
}

void Oregon::setTemperature(uint8_t *data, float temp) {
  // Set temperature sign
  if (temp < 0) {
    data[6] = 0x08;
    temp *= -1;
  } else {
    data[6] = 0x00;
  }

  // Determine decimal and float part
  int tempInt = (int)temp;
  int td = (int)(tempInt / 10);
  int tf = (int)round((float)((float)tempInt / 10 - (float)td) * 10);

  int tempFloat = (int)round((float)(temp - (float)tempInt) * 10);

  // Set temperature decimal part
  data[5] = (td << 4);
  data[5] |= tf;

  // Set temperature float part
  data[4] |= (tempFloat << 4);
}

void Oregon::setHumidity(uint8_t *data, uint8_t hum) {
  data[7] = (hum / 10);
  data[6] |= (hum - data[7] * 10) << 4;
}

void Oregon::setPressure(uint8_t *data, float pres) {
  if ((pres > 850) && (pres < 1100)) {
    data[8] = (int)round(pres) - 856;
    data[9] = 0xC0;
  }
}

int Oregon::Sum(uint8_t count, const uint8_t *data) {
  int s = 0;

  for (uint8_t i = 0; i < count; ++i) {
    s += (data[i] & 0xF0) >> 4;
    s += (data[i] & 0xF);
  }

  if (int(count) != count)
    s += (data[count] & 0xF0) >> 4;

  return s;
}

void Oregon::calculateAndSetChecksum(uint8_t *data) {
#if OREGON_MODE == MODE_0
  int s = ((Sum(6, data) + (data[6] & 0xF) - 0xa) & 0xff);
  data[6] |= (s & 0x0F) << 4;
  data[7] = (s & 0xF0) >> 4;

#elif OREGON_MODE == MODE_1
  data[8] = ((Sum(8, data) - 0xa) & 0xFF);

#else
  data[10] = ((Sum(10, data) - 0xa) & 0xFF);
#endif
}
