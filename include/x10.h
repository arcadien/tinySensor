#pragma once

#include <stdint.h>
#if defined(AVR)
#include <avr/io.h>
#include <x10rf.h>
#endif

x10rf _x10rf(PINB0, PINB1, 3);

class x10 {

#if defined(AVR)

public:
  void RFXmeter(uint8_t sensor_id, uint8_t packetType, long value) {
    _x10rf.RFXmeter(sensor_id, packetType, value);
  }

#else
public:
  void RFXmeter(uint8_t sensor_id, uint8_t packetType, long value) {}
#endif
};