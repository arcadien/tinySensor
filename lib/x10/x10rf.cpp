/*
x10rf.cpp
Library for sending x10 messages by RF.
Created by Pieter Paul Baron (embedded [at] ppbaron.nl), November 2013.
Released into the public domain.

Library to send x10 messages via a cheap 433Mhz OOK device. No X10 Firecracker
(CMA17A) necessary. Decoding messages is not implemented. This library can
emulate x10 switches and security devices and also RFXMeter and RFXSensor
devices manufactured by RFXCom. (www.rfxcom.com)

Tested on a TI Stellaris Launchpad (LM4F120H5QR) and Energia 0101E0010. This
should also work on Arduino (small modifications) or other TI Launchpad devices.

Rework by Aurelien Labrosse on August 2023. Now uses hardware abstraction layer
from TinySensor project which allow easy port to other platform as well as
testing support.

*/

#include "x10rf.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BITREAD(value, bit) (((value) >> (bit)) & 0x01)
#define BITWRITE(value, bit, bitvalue) \
  do {                                 \
    if (bitvalue)                      \
      (value) |= (1 << (bit));         \
    else                               \
      (value) &= ~(1 << (bit));        \
  } while (0)
#define NIBBLE_SUM(b) ((((b)&0xF0) >> 4) + ((b)&0xF))

#define PREAMBLE_LOW 4500   // Start silence (leader) = 4,5 ms
#define PREAMBLE_HIGH 8960  // Start burst (leader) = 9ms

#define X10_RF_BIT_LONG 1120  // Bit 1 gap length
#define X10_RF_BIT_SHORT 560  // Bit 0 gap length

#define X10_RF_GAP 40000  // Length between commands

void x10rf::SendCommand(uint8_t *data, uint8_t byteCount) {
  _hal->LedOn();
  for (int i = 0; i < _rf_repeats; i++) {
    // preamble
    _hal->RadioGoHigh();
    _hal->DelayX10PreambleHigh();
    _hal->RadioGoLow();
    _hal->DelayX10PreambleLow();

    for (int j = 0; j < byteCount; j++) {
      for (int i = 7; i >= 0; i--) {
        SendX10RfBit((bitRead(data[j], i) == 1));
      }
    }
    SendX10RfBit(1);

    // space between message repeat
    _hal->DelayX10Gap();
  }
  _hal->LedOff();
  _hal->RadioGoLow();
}

void x10rf::SendX10RfBit(uint8_t databit) {
  _hal->RadioGoHigh();
  _hal->DelayX10BitShort();
  _hal->RadioGoLow();
  _hal->DelayX10BitShort();
  if (databit) {
    _hal->DelayX10BitLong();
  }
}

x10rf::x10rf(Hal *hal, uint8_t rf_repeats) {
  _rf_repeats = rf_repeats;
  _hal = hal;
}

uint8_t *x10rf::getMessage() {
  return _message;
}

static void clearMessageBuffer(uint8_t *messageBuffer) {
  memset(messageBuffer, 0, 6);
}

static void setMeterValue(uint8_t *buffer, uint32_t value) {
  buffer[4] = (uint8_t)((value >> 16) & 0xff);
  buffer[2] = (uint8_t)((value >> 8) & 0xff);
  buffer[3] = (uint8_t)(value & 0xff);
}

void x10rf::RFXmeter(uint8_t rfxm_address, uint8_t rfxm_packet_type,
                     uint32_t rfxm_value) {
  clearMessageBuffer(_message);
  _message[0] = rfxm_address;

  // Calculate byte1 (byte 1 complement upper nibble of byte0)
  _message[1] = (~_message[0] & 0xF0) + (_message[0] & 0xF);

  if (rfxm_value > 0xFFFFFF) {
    rfxm_value = 0;  // We only have 3 byte for data. Is overflowed set to 0
    // Packet type goed into MSB nibble of byte 5. Max 15 (B1111) allowed
  }

  // Use switch case to filter invalid data types
  switch (rfxm_packet_type) {
    case 0x00:  // Normal. Put counter values in byte 4,2 and 3
      setMeterValue(_message, rfxm_value);
      break;
    case 0x01:  // New interval time set. Byte 2 should be filled with interval
      switch (rfxm_value) {
        case 0x01:
          break;  // 30sec
        case 0x02:
          break;  // 01min
        case 0x04:
          break;  // 06min (RFXpower = 05min)
        case 0x08:
          break;  // 12min (RFXpower = 10min)
        case 0x10:
          break;  // 15min
        case 0x20:
          break;  // 30min
        case 0x40:
          break;  // 45min
        case 0x80:
          break;  // 60min
        default:
          rfxm_value = 0x01;  // Set to 30 sec if no valid option is found
      }
      _message[2] = rfxm_value;
      break;
    case 0x02:  // calibrate value in <counter value> in µsec.
      setMeterValue(_message, rfxm_value);
      break;
    case 0x03:
      break;  // new address set
    case 0x04:
      break;    // counter value reset to zero
    case 0x0B:  // counter value set
      setMeterValue(_message, rfxm_value);
      break;
    case 0x0C:
      break;  // set interval mode within 5 seconds
    case 0x0D:
      break;  // calibration mode within 5 seconds
    case 0x0E:
      break;    // set address mode within 5 seconds
    case 0x0F:  // identification packet (byte 2 = address, byte 3 = interval)
      switch (rfxm_value) {
        case 0x01:
          break;  // 30sec
        case 0x02:
          break;  // 01min
        case 0x04:
          break;  // 06min (RFXpower = 05min)
        case 0x08:
          break;  // 12min (RFXpower = 10min)
        case 0x10:
          break;  // 15min
        case 0x20:
          break;  // 30min
        case 0x40:
          break;  // 45min
        case 0x80:
          break;  // 60min
        default:
          rfxm_value = 0x01;  // Set to 30 sec if no valid option is found
      }
      _message[2] = rfxm_address;
      _message[3] = rfxm_value;
      break;
    default:  // Unknown packet type. Set packet type to zero and set counter to
              // rfxm_value
      rfxm_packet_type = 0;
      setMeterValue(_message, rfxm_value);
  }
  _message[5] =
      (rfxm_packet_type << 4);  // Packet type goes into byte 5's upper nibble.
  // Calculate parity which
  uint8_t parity = ~(NIBBLE_SUM(_message[0]) + NIBBLE_SUM(_message[1]) +
                     NIBBLE_SUM(_message[2]) + NIBBLE_SUM(_message[3]) +
                     NIBBLE_SUM(_message[4]) + ((_message[5] & 0xF0) >> 4));
  _message[5] = (_message[5] & 0xf0) + (parity & 0X0F);
  SendCommand(_message, 6);  // Send byte to be broadcasted
}

void x10rf::RFXsensor(uint8_t rfxs_address, uint8_t rfxs_type,
                      char rfxs_packet_type, uint8_t rfxs_value) {
  clearMessageBuffer(_message);
  _message[0] = (rfxs_address << 2);
  switch (rfxs_type) {
    case 't':
      break;   // Temperature (default)
    case 'a':  // A/D
      _message[0] = _message[0] + 0B01;
      break;
    case 'm':  // message
      _message[0] = _message[0] + 0B11;
      break;
    case 'v':  // voltage
      _message[0] = _message[0] + 0B10;
      break;
  }
  _message[1] =
      (~_message[0] & 0xF0) +
      (_message[0] &
       0xF);  // Calculate byte1 (byte 1 complement MSB nibble of byte0)
  _message[2] = rfxs_value;
  switch (rfxs_packet_type) {
    case 't':  // temperature sensor (MSB = 0.5 degrees bit off)
      _message[3] = 0x00;
      break;
    case 'T':  // temperature sensor (MSB = 0.5 degrees bit on)
      _message[3] = 0x80;
      break;
    case 'h':  // RFU (humidity sensor)
      _message[3] = 0x20;
      break;
    case 'p':  // RFU (pressure sensor, value/10)
      _message[3] = 0x40;
      break;
    default:
      _message[3] = 0x00;
  }
  uint8_t parity = ~(NIBBLE_SUM(_message[0]) + NIBBLE_SUM(_message[1]) +
                     NIBBLE_SUM(_message[2]) + ((_message[3] & 0xF0) >> 4));
  _message[3] = (_message[3] & 0xf0) + (parity & 0XF);
  SendCommand(_message, 4);
}

void x10rf::x10Switch(char house_code, uint8_t unit_code, uint8_t command) {
  clearMessageBuffer(_message);
  // House code lookup table: indices a-p map to nibble values
  static const uint8_t house_codes[] = {
      0b0110, 0b0111, 0b0100, 0b0101, 0b1000, 0b1001, 0b1010, 0b1011,
      0b1110, 0b1111, 0b1100, 0b1101, 0b0000, 0b0001, 0b0010, 0b0011};
  char lower = tolower(house_code);
  if (lower >= 'a' && lower <= 'p') {
    _message[0] = house_codes[lower - 'a'];
  } else {
    _message[0] = 0;
  }
  _message[0] = _message[0] << 4;  // House code goes into the upper nibble

  switch (command) {
    case ON:
    case OFF:
    case BRIGHT:
    case DIM:
      _message[2] = command;
      break;
  }
  // Set unit number
  unit_code = unit_code - 1;
  BITWRITE(_message[2], 6, BITREAD(unit_code, 2));
  BITWRITE(_message[2], 3, BITREAD(unit_code, 1));
  BITWRITE(_message[2], 4, BITREAD(unit_code, 0));
  BITWRITE(_message[0], 2, BITREAD(unit_code, 3));
  // Set parity
  _message[1] = ~_message[0];
  _message[3] = ~_message[2];
  SendCommand(_message, 4);
}

uint8_t x10rf::x10SecurityParity(uint8_t *data) {
  uint8_t parity =
      data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4] ^ (data[5] & 0x80);
  parity = (parity >> 4) ^ (parity & 0xf);  // fold to nibble
  parity = (parity >> 2) ^ (parity & 0x3);  // fold to 2 bits
  parity = (parity >> 1) ^ (parity & 0x1);  // fold to 1 bit
  return parity;
}

void x10rf::x10Security(uint8_t address, uint8_t id, uint8_t command) {
  clearMessageBuffer(_message);

  // first part of device ID
  _message[0] = address;
  _message[1] = (~_message[0] & 0xF) + (_message[0] & 0xF0);

  // command
  _message[2] = command;
  _message[3] = ~_message[2];

  // second part of device ID
  _message[4] = id;

  // even parity
  _message[5] = x10SecurityParity(_message);

  SendCommand(_message, 6);
}
