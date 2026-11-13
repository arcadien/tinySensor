#pragma once

#include <Hal.h>
#include <math.h>
#include <stdint.h>

/*
 * Emulation of environment sensors using the Oregon v2.1 layout decoded by
 * RFLink Plugins/Plugin_048.c (couin3)
 *
 *  - Temperature:                     emulates THN132N    (id 0xEA, 0x4C)
 *  - Temperature, humidity:           emulates THGR228N   (id 0x1A, 0x2D)
 *  - Temperature, humidity, pressure: emulates BTHR918    (id 0x5A, 0x5D)
 *
 * Payload data is stored in the `message` property. Payload is made of
 * 'nibbles' which are 4 bits long; nibble 2i is the low nibble of byte i.
 *
 * byte:      0    1    2    3    4     5      6      7      8      9    10
 * nibbles: [0 1][2 3][4 5][6 7][8 9][10 11][12 13][14 15][16 17][18 19][20 21]
 *
 * bytes 0-1 : device ID, nibble 0 being the sync nibble 0xA
 * byte  2   : channel in the high nibble
 * byte  3   : rolling code
 * byte  4   : flags (bit 2 battery low) | temperature tenths << 4
 * byte  5   : temperature units | tens << 4
 * byte  6   : sign (bit 3) | humidity units << 4
 * byte  7   : humidity tens
 * byte  8   : pressure hPa - 795, clamped 0-255
 * byte  9   : weather prediction
 * checksum  : nibbles 13-14 (THN132N), byte 8 (THGR228N), byte 10 (BTHR918)
 *
 */
class OregonV3 {
 public:
  static const int ORDERS_COUNT_FOR_A_BIT = 6;
  static const uint8_t MESSAGE_SIZE_IN_BYTES = 11;

  // 6 unsigned chars per bit, 24 times "1"
  static const int PREAMBLE_BYTE_LENGTH = 24 * 6;
  static const int POSTAMBLE_BYTE_LENGTH = 2 * 6;

  OregonV3(Hal *hal);

  static bool BitRead(uint8_t value, uint8_t bit) {
    return (((value) >> (bit)) & 0x01);
  }

  void SetChannel(unsigned char channel);

  void SetRollingCode(unsigned char rollingCode);

  void SetBatteryLow(bool batteryIsLow);

  void SetTemperature(float temperature);

  void SetHumidity(uint8_t humidity);

  /*!
   *
   * Pressure is set using hPa
   *
   * Stored as hPa - 795, clamped to 0-255 (795-1050 hPa).
   */
  void SetPressure(uint16_t pressure);

  /*!
   * Emit message according to available data
   */
  void Send();

  void SendZero();

  void SendOne();

  uint8_t GetMessageStatus() { return messageStatus; }

  void SendData(const uint8_t *data, uint8_t size);

  const unsigned char *GetMessage();

  void FinalizeMessage();

 private:
  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   */
  uint8_t messageStatus = 0;

  uint8_t message[MESSAGE_SIZE_IN_BYTES];

  Hal *_hal;

  int Sum(uint8_t count, const uint8_t *data);
  void SendMSB(const uint8_t data);
  void SendLSB(const uint8_t data);

  void DelayHalfPeriod();
  void DelayPeriod();
};