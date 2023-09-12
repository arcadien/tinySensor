/*
x10rf.h
Library for sending x10 messages by RF.
Created by Pieter Paul Baron (embedded [at] ppbaron.nl), November 2013.
Released into the public domain.

Library to send x10 messages via a cheap 433Mhz OOK device. No X10 Firecracker
(CMA17A) necessary. Decoding messages is not implemented. This library can
emulate x10 switches and security devices and also RFXMeter and RFXSensor
devices manufactured by RFXCom. (www.rfxcom.com)

Tested on a TI Stellarpad (LM4F120H5QR) and Energia 0101E0010. This should also
work on Arduino (small modifications) or other TI Launchpad devices.

Rework by Aurelien Labrosse on August 2023. Now uses hardware abstraction layer
from TinySensor project which allow easy port to other platform as well as
testing support.

*/

// ensure this library description is only included once
#ifndef x10rf_h
#define x10rf_h

#define ON 0x00
#define OFF 0x20
#define BRIGHT 0x88
#define DIM 0x98

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#ifndef bitWrite
#define bitWrite(value, bit, bitvalue)                                         \
  (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif
#include <Hal.h>
#include <stdint.h>

#include <stdint.h>
#include <stdlib.h>

class x10rf {
public:
  x10rf(Hal *hal, uint8_t rf_repeats);

  /** Max value is 16777215 (0xffffff) because only 3 bytes are used */
  void RFXmeter(uint8_t rfxm_address, uint8_t rfxm_packet_type,
                uint32_t rfxm_value);
  void RFXsensor(uint8_t rfxs_address, uint8_t rfxs_type, char rfxs_packet_type,
                 uint8_t rfxs_value);
  void x10Switch(char house_code, uint8_t unit_code, uint8_t command);
  void x10Security(uint8_t address, uint8_t id, uint8_t command);

  uint8_t *getMessage();

  uint8_t x10SecurityParity(uint8_t *data);

private:
  void SendX10RfBit(uint8_t databit);
  void SendCommand(uint8_t *data, uint8_t byteCount);

  Hal *_hal;
  uint8_t _rf_repeats;
  uint8_t _message[6];
};
#endif