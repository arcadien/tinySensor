
#pragma once

#ifndef __AVR_ATmega328_HAL__
#define __AVR_ATmega328_HAL__
#endif

/*
 * 1-wire interface is on PA3
 * Power for radio is on PA2 (sensor_vcc)
 * LED pin is PB1
 * PWM pin is PA7   // no PA on UNO
 * SDA pin is PA6   // no PA on UNO
 * SCL pin is PA4   // no PA on UNO
 * MISO pin is PA5  // no PA on UNO
 * TX radio pin is PB0
 * BAT sensor pin is PA1    // no PA on UNO
 * analog sensor pin is PA0 // no PA on UNO
 */

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

  void Hibernate(uint8_t seconds) override;
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
