#ifndef __HAL_DEF__
#define __HAL_DEF__

#include <stdint.h>
/*
 * Interface definition for lower level hardware function
 *
 */
class Hal {

public:
  virtual void LedOn() = 0;
  virtual void LedOff() = 0;

  virtual void Delay400Us() = 0;

  virtual void Delay1s() = 0;
  virtual void Delay30ms() = 0;
  virtual void Delay512Us() = 0;
  virtual void Delay1024Us() = 0;

  virtual void Hibernate(uint8_t seconds) = 0;

  virtual void RadioGoLow() = 0;
  virtual void RadioGoHigh() = 0;

  virtual void PowerOnSensors() = 0;
  virtual void PowerOffSensors() = 0;

  virtual uint16_t GetBatteryVoltageMv() = 0;
  virtual uint16_t GetVccVoltageMv() = 0;

  virtual void Init() = 0;
};
#endif