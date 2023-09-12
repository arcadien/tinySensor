
#pragma once

#ifndef __AVR_ATmega328_HAL__
#define __AVR_ATmega328_HAL__
#endif

#include <Hal.h>
#include <stdint.h>

class Atmega328pHal : public Hal
{
public:
  Atmega328pHal();

  uint16_t GetVccVoltageMv(void) override;
  uint16_t GetBatteryVoltageMv() override;

  void LedOn() override;
  void LedOff() override;

  void RadioGoLow() override;
  void RadioGoHigh() override;

  void Hibernate(uint16_t seconds) override;
  void PowerOnSensors() override;
  void PowerOffSensors() override;

  void Delay1s() override;
  void Delay30ms() override;
  void Delay400Us() override;
  void Delay512Us() override;
  void Delay1024Us() override;

  void Init(void) override
  {
  }
};
