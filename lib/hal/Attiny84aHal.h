#pragma once

#ifndef __AVR_ATtiny84A_HAL__
#define __AVR_ATtiny84A_HAL__
#endif

#include <Hal.h>

#include <stdint.h>

#if defined(USE_I2C)
#include <TinyI2CMaster.h>
#endif
class Attiny84aHal : public Hal
{
public:
  Attiny84aHal();

  uint16_t GetVccVoltageMv(void) override;
  uint16_t GetBatteryVoltageMv() override;

  void LedOn() override;
  void LedOff() override;

  void RadioGoLow() override;
  void RadioGoHigh() override;

  void Hibernate(uint8_t seconds) override;
  void PowerOnSensors() override;
  void PowerOffSensors() override;

  void inline Delay30ms() override;
  void inline Delay512Us() override;
  void inline Delay1024Us() override;

  void InitI2C(void) override
  {
#if defined(USE_I2C)
    TinyI2C.init();
#endif
  }
};
