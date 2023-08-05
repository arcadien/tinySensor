#include <Hal.h>
#include <Oregon_v3.h>

// v3, two nibbles with unknown format
// see "Message Layout" section
static const uint8_t POSTAMBLE[] = {0x0};

// v3, preamble is 6 nibbles of 1, 24 times '1'
// see "Message Layout" section
static const uint8_t PREAMBLE[] = {0xFF, 0xFF, 0xFF};

unsigned char message[OregonV3::MESSAGE_SIZE_IN_BYTES];

static const uint16_t HALF_DELAY_US = 512;
static const uint16_t DELAY_US = HALF_DELAY_US * 2;

// pressure is a value expressed in hPa, in interval ]850, 1100[
// To allow storage in a char, PRESSURE_SCALING_VALUE is removed from actual
// pressure value. It is added when decoding the message
static const int PRESSURE_SCALING_VALUE = 856;

// As rolling code is spread on two bytes, then its
// max value is 15 tens and 15 units, because 15 aka 0xF is the max value on 4
// bits
static const unsigned char MAX_ROLLING_CODE_VALUE = 0xA5; // 165

// one nibble as 0101, so 0b00001010
// see "Message Layout" section
static const uint8_t SYNC = {0b00001010};

OregonV3::OregonV3(Hal *hal) : _hal(hal) {
  for (uint8_t index = 0; index < MESSAGE_SIZE_IN_BYTES; index++) {
    message[index] = 0;
  }
}

const unsigned char *OregonV3::GetMessage() { return message; }

void OregonV3::SetBatteryLow() { message[3] |= 0x04; }

void OregonV3::SetPressure(int pressure) {

  messageStatus |= 1 << 2;

  if ((pressure > 850) && (pressure < 1100)) {
    message[8] = pressure - PRESSURE_SCALING_VALUE;
    message[9] = 0xC0;
  }
}

void OregonV3::SendMSB(const uint8_t data) {
  (BitRead(data, 4)) ? SendOne() : SendZero();
  (BitRead(data, 5)) ? SendOne() : SendZero();
  (BitRead(data, 6)) ? SendOne() : SendZero();
  (BitRead(data, 7)) ? SendOne() : SendZero();
}

void OregonV3::SendLSB(const uint8_t data) {
  (BitRead(data, 0)) ? SendOne() : SendZero();
  (BitRead(data, 1)) ? SendOne() : SendZero();
  (BitRead(data, 2)) ? SendOne() : SendZero();
  (BitRead(data, 3)) ? SendOne() : SendZero();
}

int OregonV3::Sum(uint8_t count, const uint8_t *data) {
  int s = 0;

  for (uint8_t i = 0; i < count; ++i) {
    s += (data[i] & 0xF0) >> 4;
    s += (data[i] & 0xF);
  }

  if (int(count) != count)
    s += (data[count] & 0xF0) >> 4;

  return s;
}

void OregonV3::SendData(const uint8_t *data, uint8_t size) {
  for (uint8_t i = 0; i < size; ++i) {
    SendLSB(data[i]);
    SendMSB(data[i]);
  }
}

void OregonV3::DelayHalfPeriod() { _hal->Delay512Us(); }

void OregonV3::DelayPeriod() { _hal->Delay1024Us(); }

void OregonV3::SendZero() {
  _hal->RadioGoHigh();
  DelayHalfPeriod();
  _hal->RadioGoLow();
  DelayPeriod();
  _hal->RadioGoHigh();
  DelayHalfPeriod();
}

void OregonV3::SetChannel(unsigned char channel) {
  message[2] |= 1 << (4 + (channel - 1));
}

/*
 * \param rollingCode must be less than MAX_ROLLING_CODE_VALUE
 */
void OregonV3::SetRollingCode(unsigned char rollingCode) {
  unsigned char rollingCodeTens = (unsigned char)rollingCode / 10;
  message[2] |= rollingCodeTens & 0x0f;                    // nibble 4
  message[3] |= (rollingCode - rollingCodeTens * 10) << 4; // nibble 5
}

void OregonV3::SendOne() {
  _hal->RadioGoLow();
  DelayHalfPeriod();
  _hal->RadioGoHigh();
  DelayPeriod();
  _hal->RadioGoLow();
  DelayHalfPeriod();
}

void OregonV3::SetHumidity(int humidity) {
  messageStatus |= 1 << 1;

  // humidity nibbles are spread on bytes 6 and 7
  // bit 6 also contains the negative temp flag (lsb)
  message[7] = (humidity / 10);
  message[6] |= (humidity - message[7] * 10) << 4;
}

void OregonV3::SetTemperature(float temperature) {

  messageStatus |= 1;
  if (temperature < 0) {
    message[6] = 0x08;
    temperature *= -1;
  } else {
    message[6] = 0x00;
  }

  // Determine decimal and float part

  // ex. for 27.4 degrees C
  uint8_t tempIntegerPart = (uint8_t)temperature; // 27

  // (int)(27/10) = 2
  uint8_t temperatureDozen = (uint8_t)(tempIntegerPart / 10);

  // 2.7 - 20 = (int)7.4=7
  uint8_t temperatureUnit = (uint8_t)round(
      (float)((float)tempIntegerPart / 10.0 - (float)temperatureDozen) * 10.0);

  uint8_t temperatureDecimal = (uint8_t)round(
      (float)(temperature - (float)tempIntegerPart) // 27.4 - 27 = 0.4
      * 10                                          // 0.4 * 10 = 4
  );

  message[4] |= (temperatureDecimal << 4);
  message[4] |= temperatureUnit;
  message[5] = (temperatureDozen << 4);
}

void OregonV3::ComputeSensorId() {
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
    message[0] = 0xEC;
    message[1] = 0x40;
    int s = ((Sum(6, message) + (message[6] & 0xF) - 0xa) & 0xff);
    message[6] |= (s & 0x0F) << 4;
    message[7] = (s & 0xF0) >> 4;
  }

  if (messageLength == 9) {
    // THGN123N (Temperature + Humidity)
    message[0] = 0x1D;
    message[1] = 0x20;
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

  ComputeSensorId();
  // The specification document says that the SYNC must be sent.
  // With the RFLINK decoder, which is the reference for this library,
  // the SYNC is not needed.

  SendData(PREAMBLE, 3);
  SendData(message, MESSAGE_SIZE_IN_BYTES);
  SendData(POSTAMBLE, 1);
}
