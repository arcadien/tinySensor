#pragma once

#include <cmath>
#include <stdint.h>

/*
 * Interface definition for lower level hardware function
 *
 */
class Hal {
public:
  virtual void GoLow() const = 0;
  virtual void GoHigh() const = 0;
  virtual void DelayPeriod() const = 0;
  virtual void DelayHalfPeriod() const = 0;
};

bool BitRead(uint8_t value, uint8_t bit) { return (((value) >> (bit)) & 0x01); }

/*
 * Emulation of environment sensors using Oregon v3 protocol
 *
 *
 *  - Temperature:                     emulates THN132N (id 0xEC, 0x40)
 *  - Temperature, humidity:           emulates THGR122NX (id 0x1D, 0x20)
 *  - Temperature, humidity, pressure: emulates BTHR968 (id 0x5D, 0x60)
 *
 * Payload data is stored in the `_message` property. Payload is made of
 * 'nibbles' which are 4 bits long.
 *
 * byte:      0    1    2    3    4     5      6      7      8      9    10
 * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19][20 21]
 *
 * nibbles 0..3    : sensor ID
 * nibble  4       : channel
 * nibbles 5..6    : rolling code
 * nibble  7       : battery state (0x4 is low battery)
 * nibbles 8..13   : temperature
 * nibble 12,14,15 : humidity (or-ed with temperature)
 * nibbles 16..19  : pressure
 * nibbles 20..21  : checksum
 *
 */
class OregonV3 {

public:
  // pressure is a value expressed in hPa, in interval ]850, 1100[
  // To allow storage in a char, PRESSURE_SCALING_VALUE is removed from actual
  // pressure value. It is added when decoding the message
  static const int PRESSURE_SCALING_VALUE = 856;

  // As rolling code is spread on two bytes, then its
  // max value is 15 tens and 15 units, because 15 aka 0xF is the max value on 4
  // bits
  static const unsigned char MAX_ROLLING_CODE_VALUE = 165;

  static const uint8_t MESSAGE_SIZE_IN_BYTES = 11;

  unsigned char Message[MESSAGE_SIZE_IN_BYTES];

  /*!
   * @param hal the component responsible in bit emission (delay and pin
   * states)
   */
  OregonV3(Hal const &hal) : _hal(hal) {

    for (uint8_t index = 0; index < MESSAGE_SIZE_IN_BYTES; index++) {
      Message[index] = 0;
    }
  }

  void SetChannel(unsigned char channel) {
    Message[2] |= 1 << (4 + (channel - 1));
  }

  /*
   * \param rollingCode must be less than MAX_ROLLING_CODE_VALUE
   */
  void SetRollingCode(unsigned char rollingCode) {
    unsigned char rollingCodeTens = (unsigned char)rollingCode / 10;
    Message[2] |= rollingCodeTens & 0x0f;                    // nibble 4
    Message[3] |= (rollingCode - rollingCodeTens * 10) << 4; // nibble 5
  }

  void SetBatteryLow() { Message[4] |= 0xC; }

  void SetTemperature(float temperature) {

    _messageStatus |= 1;
    if (temperature < 0) {
      Message[6] = 0x08;
      temperature *= -1;
    } else {
      Message[6] = 0x00;
    }

    // Determine decimal and float part

    // ex. for 27.4 degrees C
    int tempIntegerPart = (int)temperature; // 27

    // (int)(27/10) = 2
    int temperatureDozen = (int)(tempIntegerPart / 10);

    // 2.7 - 20 = (int)7.4=7
    int temperatureUnit = (int)round(
        (float)((float)tempIntegerPart / 10.0 - (float)temperatureDozen) *
        10.0);

    int temperatureDecimal = (int)round(
        (float)(temperature - (float)tempIntegerPart) // 27.4 - 27 = 0.4
        * 10                                          // 0.4 * 10 = 4
    );

    Message[4] |= (temperatureDecimal << 4);
    Message[5] = (temperatureDozen << 4);
    Message[5] |= temperatureUnit;
  }

  void SetHumidity(int humidity) {
    _messageStatus |= 1 << 1;

    // humidity nibbles are spread on bytes 6 and 7
    // bit 6 also contains the negative temp flag (lsb)
    Message[7] = (humidity / 10);
    Message[6] |= (humidity - Message[7] * 10) << 4;
  }

  /*!
   *
   * Presure is set using hPa
   *
   * Values out of possible ones on earth (less that 850, more than 1099) are
   * ignored.
   */
  void SetPressure(int pressure) {

    _messageStatus |= 1 << 2;

    if ((pressure > 850) && (pressure < 1100)) {
      Message[8] = pressure - PRESSURE_SCALING_VALUE;
      Message[9] = 0xC0;
    }
  }

  int Sum(uint8_t count, const uint8_t *data) {
    int s = 0;

    for (uint8_t i = 0; i < count; ++i) {
      s += (data[i] & 0xF0) >> 4;
      s += (data[i] & 0xF);
    }

    if (int(count) != count)
      s += (data[count] & 0xF0) >> 4;

    return s;
  }
  /*!
   * Emit message according to available data
   */
  void Send() {

    if (MESSAGE_SIZE_IN_BYTES == 8) {
      // THGN122N (Temperature)
      Message[0] = 0x0A;
      Message[1] = 0x4D;
      int s = ((Sum(6, Message) + (Message[6] & 0xF) - 0xa) & 0xff);
      Message[6] |= (s & 0x0F) << 4;
      Message[7] = (s & 0xF0) >> 4;
    }

    if (MESSAGE_SIZE_IN_BYTES == 9) {
      // THGN122N (Temperature + Humidity)
      Message[0] = 0x1A;
      Message[1] = 0x2D;
      Message[8] = ((Sum(8, Message) - 0xa) & 0xFF);
    }

    if (MESSAGE_SIZE_IN_BYTES == 11) {
      // BTHR918N (temp, humidity, barometer)
      Message[0] = 0x5A;
      Message[1] = 0x6D;
      Message[10] = ((Sum(10, Message) - 0xa) & 0xFF);
    }

    // The specification document says that the SYNC must be sent.
    // With the RFLINK decoder, which is the reference for this library,
    // the SYNC is not needed.

    SendData(PREAMBLE, 3);
    SendData(Message, MESSAGE_SIZE_IN_BYTES);
    SendData(POSTAMBLE, 1);
  }

  void SendData(const uint8_t *data, uint8_t size) {
    for (uint8_t i = 0; i < size; ++i) {
      SendLSB(data[i]);
      SendMSB(data[i]);
    }
  }

  void SendZero(void) {
    _hal.GoHigh();
    _hal.DelayHalfPeriod();
    _hal.GoLow();
    _hal.DelayPeriod();
    _hal.GoHigh();
    _hal.DelayHalfPeriod();
  }

  void SendOne(void) {
    _hal.GoLow();
    _hal.DelayHalfPeriod();
    _hal.GoHigh();
    _hal.DelayPeriod();
    _hal.GoLow();
    _hal.DelayHalfPeriod();
  }

  uint8_t GetMessageStatus() { return _messageStatus; }

private:
  void SendMSB(const uint8_t data) {
    (BitRead(data, 4)) ? SendOne() : SendZero();
    (BitRead(data, 5)) ? SendOne() : SendZero();
    (BitRead(data, 6)) ? SendOne() : SendZero();
    (BitRead(data, 7)) ? SendOne() : SendZero();
  }

  void SendLSB(const uint8_t data) {
    (BitRead(data, 0)) ? SendOne() : SendZero();
    (BitRead(data, 1)) ? SendOne() : SendZero();
    (BitRead(data, 2)) ? SendOne() : SendZero();
    (BitRead(data, 3)) ? SendOne() : SendZero();
  }

  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   */
  uint8_t _messageStatus;

  // one nibble as 0101, so 0b00001010
  // see "Message Layout" section
  static const uint8_t SYNC = {0b00001010};

  // v3, two nibbles with unknown format
  // see "Message Layout" section
  static const uint8_t POSTAMBLE[1];

  // v3, preamble is 6 nibbles of 1, 24 times '1'
  // see "Message Layout" section
  static const uint8_t PREAMBLE[3];

  Hal const &_hal;
};
const uint8_t OregonV3::POSTAMBLE[1] = {0x0};
const uint8_t OregonV3::PREAMBLE[3] = {0xFF, 0xFF, 0xFF};