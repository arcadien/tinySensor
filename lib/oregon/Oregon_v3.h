#pragma once

#include <Hal.h>
#include <math.h>
#include <stdint.h>

/*
 * Emulation of environment sensors using Oregon v3 protocol
 *
 *
 *  - Temperature:                     emulates THN132N (id 0xEC, 0x40)
 *  - Temperature, humidity:           emulates THGR122NX (id 0x1D, 0x20)
 *  - Temperature, humidity, pressure: emulates BTHR968 (id 0x5D, 0x60)
 *
 * Payload data is stored in the `message` property. Payload is made of
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

  static const int ORDERS_COUNT_FOR_A_BIT = 6;

  // 6 unsigned chars per bit, 24 times "1"
  const static int PREAMBLE_BYTE_LENGTH = 24 * 6;
  const static int POSTAMBLE_BYTE_LENGTH = 2 * 6;

  static const uint8_t MESSAGE_SIZE_IN_BYTES = 11;
  unsigned char message[MESSAGE_SIZE_IN_BYTES];

  OregonV3(Hal *hal) : _hal(hal) {
    for (uint8_t index = 0; index < MESSAGE_SIZE_IN_BYTES; index++) {
      message[index] = 0;
    }
  }

  void SetChannel(unsigned char channel) {
    message[2] |= 1 << (4 + (channel - 1));
  }

  /*
   * \param rollingCode must be less than MAX_ROLLING_CODE_VALUE
   */
  void SetRollingCode(unsigned char rollingCode) {
    unsigned char rollingCodeTens = (unsigned char)rollingCode / 10;
    message[2] |= rollingCodeTens & 0x0f;                    // nibble 4
    message[3] |= (rollingCode - rollingCodeTens * 10) << 4; // nibble 5
  }

  void SetBatteryLow() { message[4] |= 0xC; }

  void SetTemperature(float temperature);

  void SetHumidity(int humidity);

  /*!
   *
   * Pressure is set using hPa
   *
   * Values out of possible ones on earth (less that 850, more than 1099) are
   * ignored.
   */
  void SetPressure(int pressure) {

    messageStatus |= 1 << 2;

    if ((pressure > 850) && (pressure < 1100)) {
      message[8] = pressure - PRESSURE_SCALING_VALUE;
      message[9] = 0xC0;
    }
  }

  
  /*!
   * Emit message according to available data
   */
  void Send();

  void SendZero();

  void SendOne();

  uint8_t GetMessageStatus() { return messageStatus; }

  static bool BitRead(uint8_t value, uint8_t bit) {
    return (((value) >> (bit)) & 0x01);
  }

  void SendData(const uint8_t *data, uint8_t size);

private:
  int Sum(uint8_t count, const uint8_t *data);
  void SendMSB(const uint8_t data);
  void SendLSB(const uint8_t data);

  void DelayHalfPeriod();
  void DelayPeriod();

  static const uint16_t HALF_DELAY_US = 512;
  static const uint16_t DELAY_US = HALF_DELAY_US * 2;

  // pressure is a value expressed in hPa, in interval ]850, 1100[
  // To allow storage in a char, PRESSURE_SCALING_VALUE is removed from actual
  // pressure value. It is added when decoding the message
  static const int PRESSURE_SCALING_VALUE = 856;

  // As rolling code is spread on two bytes, then its
  // max value is 15 tens and 15 units, because 15 aka 0xF is the max value on 4
  // bits
  static const unsigned char MAX_ROLLING_CODE_VALUE = 165;

  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   */
  uint8_t messageStatus = 0;

  // one nibble as 0101, so 0b00001010
  // see "Message Layout" section
  static const uint8_t SYNC = {0b00001010};

  // v3, two nibbles with unknown format
  // see "Message Layout" section
  static const uint8_t POSTAMBLE[1];

  // v3, preamble is 6 nibbles of 1, 24 times '1'
  // see "Message Layout" section
  static const uint8_t PREAMBLE[3];

  Hal *_hal;
};