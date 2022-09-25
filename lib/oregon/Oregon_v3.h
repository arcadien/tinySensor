#pragma once

#include <Hal.h>
#include <math.h>
#include <stdint.h>

/*
 * Emulation of environment sensors using Oregon v3 protocol
 *
 *
 *  - Temperature:                     emulates TH132N     (id 0xEC, 0x40)
 *  - Temperature, humidity:           emulates THGR122NX  (id 0x1D, 0x20)
 *  - Temperature, humidity, pressure: emulates BTHR968    (id 0x5D, 0x60)
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
class OregonV3
{

public:
  static const int ORDERS_COUNT_FOR_A_BIT = 6;
  static const uint8_t MESSAGE_SIZE_IN_BYTES = 11;

  // 6 unsigned chars per bit, 24 times "1"
  static const int PREAMBLE_BYTE_LENGTH = 24 * 6;
  static const int POSTAMBLE_BYTE_LENGTH = 2 * 6;

  OregonV3(Hal *hal);

  static bool BitRead(uint8_t value, uint8_t bit)
  {
    return (((value) >> (bit)) & 0x01);
  }

  void SetChannel(unsigned char channel);

  /*
   * \param rollingCode must be less than MAX_ROLLING_CODE_VALUE
   */
  void SetRollingCode(unsigned char rollingCode);

  void SetBatteryLow();

  void SetTemperature(float temperature);

  void SetHumidity(int humidity);

  /*!
   *
   * Pressure is set using hPa
   *
   * Values out of possible ones on earth (less that 850, more than 1099) are
   * ignored.
   */
  void SetPressure(int pressure);

  /*!
   * Emit message according to available data
   */
  void Send();

  void SendZero();

  void SendOne();

  uint8_t GetMessageStatus() { return messageStatus; }

  void SendData(const uint8_t *data, uint8_t size);

  const unsigned char *GetMessage();

private:
  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   */
  uint8_t messageStatus = 0;

  Hal *_hal;

  int Sum(uint8_t count, const uint8_t *data);
  void SendMSB(const uint8_t data);
  void SendLSB(const uint8_t data);

  void DelayHalfPeriod();
  void DelayPeriod();
};