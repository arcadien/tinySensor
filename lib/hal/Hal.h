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

  virtual void Hibernate(uint16_t seconds) = 0;

  virtual void RadioGoLow() = 0;
  virtual void RadioGoHigh() = 0;

  virtual void SerialGoHigh() = 0;
  virtual void SerialGoLow() = 0;

  virtual void PowerOnSensors() = 0;
  virtual void PowerOffSensors() = 0;

  uint16_t ComputeVccMv(uint16_t known11refValue) {
    uint16_t rawAnalog11ref = GetRawInternal11Ref();
    float tmp = 1024.f / rawAnalog11ref;
    uint16_t vccMv = (uint16_t)(known11refValue * tmp);
    return vccMv;
  }

  uint16_t ConvertAnalogValueToMv(uint16_t adcReading, uint16_t vccInMv) {
    float ratio = vccInMv / 1024.f;
    return (ratio * adcReading);
  }

  virtual uint16_t GetRawBattery() = 0;
  virtual uint16_t GetRawInternal11Ref() = 0;
  virtual uint16_t GetRawAnalogSensor() = 0;

  virtual void Init() = 0;
};
#endif