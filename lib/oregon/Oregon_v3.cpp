#include <Hal.h>
#include <Oregon_v3.h>
#include <bitOps.h>
#include <string.h>

// v3, two nibbles with unknown format
// see "Message Layout" section
#define POSTAMBLE_VALUE 0x0
#define PREAMBLE_BYTE1 0xFF
#define PREAMBLE_BYTE2 0xFF
#define PREAMBLE_BYTE3 0xFF

// BTHR918 pressure byte: hPa - 795, as decoded by RFLink (FUNC-OREGON-008)
#define PRESSURE_SCALING_VALUE 795

OregonV3::OregonV3(Hal *hal) : _hal(hal) {
  memset(message, 0, MESSAGE_SIZE_IN_BYTES);
}

const unsigned char *OregonV3::GetMessage() {
  return message;
}

void OregonV3::SetBatteryLow(bool batteryIsLow) {
  if (batteryIsLow) {
    message[4] |= (1 << 2);  // set low status
  } else {
    message[4] &= 0xFB;  // clear low status
  }
}

void OregonV3::SetPressure(uint16_t pressure) {
  messageStatus |= 1 << 2;

  // prediction
  if (pressure < 1000) {
    // rainy
    message[9] = 0x30;
  } else if (pressure < 1010) {
    // cloudy
    message[9] = 0x20;
  } else if (pressure < 1025) {
    // partly cloudy
    message[9] = 0x60;
  } else {
    // Sunny
    message[9] = 0xC0;
  }

  int16_t offset = (int16_t)pressure - PRESSURE_SCALING_VALUE;
  message[8] = offset < 0 ? 0 : offset > 0xFF ? 0xFF : offset;
}

void OregonV3::SendMSB(const uint8_t data) {
  (BITREAD(data, 4)) ? SendOne() : SendZero();
  (BITREAD(data, 5)) ? SendOne() : SendZero();
  (BITREAD(data, 6)) ? SendOne() : SendZero();
  (BITREAD(data, 7)) ? SendOne() : SendZero();
}

void OregonV3::SendLSB(const uint8_t data) {
  (BITREAD(data, 0)) ? SendOne() : SendZero();
  (BITREAD(data, 1)) ? SendOne() : SendZero();
  (BITREAD(data, 2)) ? SendOne() : SendZero();
  (BITREAD(data, 3)) ? SendOne() : SendZero();
}

int OregonV3::Sum(uint8_t count, const uint8_t *data) {
  int s = 0;
  for (uint8_t i = 0; i < count; ++i) {
    s += (data[i] & 0xF0) >> 4;
    s += (data[i] & 0xF);
  }
  return s;
}

void OregonV3::SendData(const uint8_t *data, uint8_t size) {
  for (uint8_t i = 0; i < size; ++i) {
    SendLSB(data[i]);
    SendMSB(data[i]);
  }
}

void OregonV3::DelayHalfPeriod() {
  _hal->Delay512Us();
}

void OregonV3::DelayPeriod() {
  _hal->Delay1024Us();
}

void OregonV3::SendZero() {
  _hal->RadioGoHigh();
  DelayHalfPeriod();
  _hal->RadioGoLow();
  DelayPeriod();
  _hal->RadioGoHigh();
  DelayHalfPeriod();
}

void OregonV3::SetChannel(uint8_t channel) {
  if (channel < 1 || channel > 3)
    return;
  message[2] = (message[2] & 0x0F) | (1 << (4 + (channel - 1)));
}

void OregonV3::SetRollingCode(uint8_t rollingCode) {
  message[3] = rollingCode;
}

void OregonV3::SendOne() {
  _hal->RadioGoLow();
  DelayHalfPeriod();
  _hal->RadioGoHigh();
  DelayPeriod();
  _hal->RadioGoLow();
  DelayHalfPeriod();
}

void OregonV3::SetHumidity(uint8_t humidity) {
  messageStatus |= 1 << 1;

  // units share byte 6 with the temperature sign bit
  uint8_t dozens = humidity / 10;
  message[6] = (message[6] & 0x0F) | ((humidity - dozens * 10) << 4);
  message[7] = dozens;
}

void OregonV3::SetTemperature(float temperature) {
  messageStatus |= 1;
  // A reused instance must not keep the previous sign (FUNC-OREGON-015)
  message[6] &= 0xF7;
  if (temperature < 0) {
    message[6] |= 0x08;
    temperature *= -1;
  }

  // Determine decimal and float part

  // ex. for 27.4 degrees C
  uint8_t tempIntegerPart = (uint8_t)temperature;  // 27

  // (int)(27/10) = 2
  uint8_t temperatureDozen = (uint8_t)(tempIntegerPart / 10);

  // 2.7 - 20 = (int)7.4=7
  uint8_t temperatureUnit = (uint8_t)round(
      (float)((float)tempIntegerPart / 10.0 - (float)temperatureDozen) * 10.0);

  uint8_t temperatureDecimal = (uint8_t)round(
      (float)(temperature - (float)tempIntegerPart)  // 27.4 - 27 = 0.4
      * 10                                           // 0.4 * 10 = 4
  );

  message[4] = (message[4] & 0x0F) | (temperatureDecimal << 4);
  message[5] = (temperatureDozen << 4) | temperatureUnit;
}

void OregonV3::FinalizeMessage() {
  uint8_t messageLength = 7;
  if ((messageStatus & 0b00000100) == 0b00000100) {
    messageLength += 4;
  } else if ((messageStatus & 0b00000010) == 0b00000010) {
    messageLength += 2;
  } else if ((messageStatus & 0b00000001) == 0b00000001) {
    messageLength++;
  }

  if (messageLength == 8) {
    // THN132N (Temperature)
    message[0] = 0xEA;
    message[1] = 0x4C;
    int s = ((Sum(6, message) + (message[6] & 0xF) - 0xa) & 0xff);
    message[6] = (message[6] & 0x0F) | ((s & 0x0F) << 4);
    message[7] = (s & 0xF0) >> 4;
  }

  if (messageLength == 9) {
    // THGR228N (Temperature + Humidity)
    message[0] = 0x1A;
    message[1] = 0x2D;
    message[8] = ((Sum(8, message) - 0xa) & 0xFF);
  }

  if (messageLength == 11) {
    // BTHR918 (temp, humidity, pressure)
    message[0] = 0x5A;
    message[1] = 0x5D;
    message[10] = ((Sum(10, message) - 0xa) & 0xFF);
  }
}
void OregonV3::Send() {
  // The specification document says that the SYNC must be sent.
  // With the RFLINK decoder, which is the reference for this library,
  // the SYNC is not needed.

  FinalizeMessage();

  const uint8_t preamble[] = {PREAMBLE_BYTE1, PREAMBLE_BYTE2, PREAMBLE_BYTE3};
  SendData(preamble, 3);
  SendData(message, MESSAGE_SIZE_IN_BYTES);
  const uint8_t postamble[] = {POSTAMBLE_VALUE};
  SendData(postamble, 1);
  _hal->RadioGoLow();
}
