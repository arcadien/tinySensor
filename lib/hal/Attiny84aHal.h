#pragma once

#ifndef __AVR_ATtiny84A_HAL__
#define __AVR_ATtiny84A_HAL__
#endif

/*
 * 1-wire interface is on PA3
 * Power for radio is on PA2 (sensor_vcc)
 * LED pin is PB1
 * PWM pin is PA7
 * SDA pin is PA6
 * SCL pin is PA4
 * MISO pin is PA5
 * TX radio pin is PB0
 * BAT sensor pin is PA1
 * analog sensor pin is PA0
 */

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

  void Delay1s() override;
  void Delay30ms() override;
  void Delay512Us() override;
  void Delay1024Us() override;

  void Init(void) override
  {
#if defined(USE_I2C)
    TinyI2C.init();
#endif
  }
};
