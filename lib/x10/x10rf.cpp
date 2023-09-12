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

#define PREAMBLE_LOW 4500  // Start silence (leader) = 4,5 ms
#define PREAMBLE_HIGH 8960 // Start burst (leader) = 9ms

#define X10_RF_BIT_LONG 1120 // Bit 1 gap length
#define X10_RF_BIT_SHORT 560 // Bit 0 gap length

#define X10_RF_GAP 40000 // Length between commands

#if defined(AVR)
#include <util/delay.h>
static inline void x10_preamble_low() { _delay_us(PREAMBLE_LOW); }
static inline void x10_preamble_high() { _delay_us(PREAMBLE_HIGH); }
static inline void x10_cooldown_gap() { _delay_us(X10_RF_GAP); }
static inline void x10_short() { _delay_us(X10_RF_BIT_SHORT); }
static inline void x10_long() { _delay_us(X10_RF_BIT_LONG); }
#else
static void x10_preamble_low() {}
static void x10_preamble_high() {}
static void x10_short() {}
static void x10_long() {}
static void x10_cooldown_gap() {}
#endif

void x10rf::SendCommand(uint8_t *data, uint8_t byteCount) {
  _hal->LedOn();
  for (int i = 0; i < _rf_repeats; i++) {
    
    // preamble
    _hal->RadioGoHigh();
    x10_preamble_high();
    _hal->RadioGoLow();
    x10_preamble_low();

    for (int j = 0; j < byteCount; j++) {
      for (int i = 7; i >= 0; i--) {
        SendX10RfBit((bitRead(data[j], i) == 1));
      }
    }
    SendX10RfBit(1);

    // space between message repeat
    x10_cooldown_gap();
  }
  _hal->LedOff();
  _hal->RadioGoLow();
}

void x10rf::SendX10RfBit(uint8_t databit) {
  _hal->RadioGoHigh();
  x10_short();
  _hal->RadioGoLow();
  x10_short();
  if (databit) {
    x10_long();
  }
}

x10rf::x10rf(Hal *hal, uint8_t rf_repeats) {
  _rf_repeats = rf_repeats;
  _hal = hal;
}

uint8_t *x10rf::getMessage() { return _message; }

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
    rfxm_value = 0; // We only have 3 byte for data. Is overflowed set to 0
    // Packet type goed into MSB nibble of byte 5. Max 15 (B1111) allowed
  }

  // Use switch case to filter invalid data types
  switch (rfxm_packet_type) {
  case 0x00: // Normal. Put counter values in byte 4,2 and 3
    setMeterValue(_message, rfxm_value);
    break;
  case 0x01: // New interval time set. Byte 2 should be filled with interval
    switch (rfxm_value) {
    case 0x01:
      break; // 30sec
    case 0x02:
      break; // 01min
    case 0x04:
      break; // 06min (RFXpower = 05min)
    case 0x08:
      break; // 12min (RFXpower = 10min)
    case 0x10:
      break; // 15min
    case 0x20:
      break; // 30min
    case 0x40:
      break; // 45min
    case 0x80:
      break; // 60min
    default:
      rfxm_value = 0x01; // Set to 30 sec if no valid option is found
    }
    _message[2] = rfxm_value;
    break;
  case 0x02: // calibrate value in <counter value> in µsec.
    setMeterValue(_message, rfxm_value);
    break;
  case 0x03:
    break; // new address set
  case 0x04:
    break;   // counter value reset to zero
  case 0x0B: // counter value set
    setMeterValue(_message, rfxm_value);
    break;
  case 0x0C:
    break; // set interval mode within 5 seconds
  case 0x0D:
    break; // calibration mode within 5 seconds
  case 0x0E:
    break;   // set address mode within 5 seconds
  case 0x0F: // identification packet (byte 2 = address, byte 3 = interval)
    switch (rfxm_value) {
    case 0x01:
      break; // 30sec
    case 0x02:
      break; // 01min
    case 0x04:
      break; // 06min (RFXpower = 05min)
    case 0x08:
      break; // 12min (RFXpower = 10min)
    case 0x10:
      break; // 15min
    case 0x20:
      break; // 30min
    case 0x40:
      break; // 45min
    case 0x80:
      break; // 60min
    default:
      rfxm_value = 0x01; // Set to 30 sec if no valid option is found
    }
    _message[2] = rfxm_address;
    _message[3] = rfxm_value;
    break;
  default: // Unknown packet type. Set packet type to zero and set counter to
           // rfxm_value
    rfxm_packet_type = 0;
    setMeterValue(_message, rfxm_value);
  }
  _message[5] =
      (rfxm_packet_type << 4); // Packet type goes into byte 5's upper nibble.
  // Calculate parity which
  uint8_t parity = ~(((_message[0] & 0XF0) >> 4) + (_message[0] & 0XF) +
                     ((_message[1] & 0XF0) >> 4) + (_message[1] & 0XF) +
                     ((_message[2] & 0XF0) >> 4) + (_message[2] & 0XF) +
                     ((_message[3] & 0XF0) >> 4) + (_message[3] & 0XF) +
                     ((_message[4] & 0XF0) >> 4) + (_message[4] & 0XF) +
                     ((_message[5] & 0XF0) >> 4));
  _message[5] = (_message[5] & 0xf0) + (parity & 0X0F);
  SendCommand(_message, 6); // Send byte to be broadcasted
}

void x10rf::RFXsensor(uint8_t rfxs_address, uint8_t rfxs_type,
                      char rfxs_packet_type, uint8_t rfxs_value) {
  clearMessageBuffer(_message);
  _message[0] = (rfxs_address << 2);
  switch (rfxs_type) {
  case 't':
    break;  // Temperature (default)
  case 'a': // A/D
    _message[0] = _message[0] + 0B01;
    break;
  case 'm': // message
    _message[0] = _message[0] + 0B11;
    break;
  case 'v': // voltage
    _message[0] = _message[0] + 0B10;
    break;
  }
  _message[1] =
      (~_message[0] & 0xF0) +
      (_message[0] &
       0xF); // Calculate byte1 (byte 1 complement MSB nibble of byte0)
  _message[2] = rfxs_value;
  switch (rfxs_packet_type) {
  case 't': // temperature sensor (MSB = 0.5 degrees bit off)
    _message[3] = 0x00;
    break;
  case 'T': // temperature sensor (MSB = 0.5 degrees bit on)
    _message[3] = 0x80;
    break;
  case 'h': // RFU (humidity sensor)
    _message[3] = 0x20;
    break;
  case 'p': // RFU (pressure sensor, value/10)
    _message[3] = 0x40;
    break;
  default:
    _message[3] = 0x00;
  }
  uint8_t parity = ~(((_message[0] & 0XF0) >> 4) + (_message[0] & 0XF) +
                     ((_message[1] & 0XF0) >> 4) + (_message[1] & 0XF) +
                     ((_message[2] & 0XF0) >> 4) + (_message[2] & 0XF) +
                     ((_message[3] & 0XF0) >> 4));
  _message[3] = (_message[3] & 0xf0) + (parity & 0XF);
  SendCommand(_message, 4);
}

void x10rf::x10Switch(char house_code, uint8_t unit_code, uint8_t command) {
  clearMessageBuffer(_message);
  switch (tolower(house_code)) {
  case 'a':
    _message[0] = 0B0110;
    break;
  case 'b':
    _message[0] = 0B0111;
    break;
  case 'c':
    _message[0] = 0B0100;
    break;
  case 'd':
    _message[0] = 0B0101;
    break;
  case 'e':
    _message[0] = 0B1000;
    break;
  case 'f':
    _message[0] = 0B1001;
    break;
  case 'g':
    _message[0] = 0B1010;
    break;
  case 'h':
    _message[0] = 0B1011;
    break;
  case 'i':
    _message[0] = 0B1110;
    break;
  case 'j':
    _message[0] = 0B1111;
    break;
  case 'k':
    _message[0] = 0B1100;
    break;
  case 'l':
    _message[0] = 0B1101;
    break;
  case 'm':
    _message[0] = 0B0000;
    break;
  case 'n':
    _message[0] = 0B0001;
    break;
  case 'o':
    _message[0] = 0B0010;
    break;
  case 'p':
    _message[0] = 0B0011;
    break;
  default:
    _message[0] = 0;
    break;
  }
  _message[0] = _message[0] << 4; // House code goes into the upper nibble

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
  bitWrite(_message[2], 6, bitRead(unit_code, 2));
  bitWrite(_message[2], 3, bitRead(unit_code, 1));
  bitWrite(_message[2], 4, bitRead(unit_code, 0));
  bitWrite(_message[0], 2, bitRead(unit_code, 3));
  // Set parity
  _message[1] = ~_message[0];
  _message[3] = ~_message[2];
  SendCommand(_message, 4);
}

uint8_t x10rf::x10SecurityParity(uint8_t *data) {
  uint8_t parity =
      data[0] ^ data[1] ^ data[2] ^ data[3] ^ data[4] ^ (data[5] & 0x80);
  parity = (parity >> 4) ^ (parity & 0xf); // fold to nibble
  parity = (parity >> 2) ^ (parity & 0x3); // fold to 2 bits
  parity = (parity >> 1) ^ (parity & 0x1); // fold to 1 bit
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
