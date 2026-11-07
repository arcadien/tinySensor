#pragma once
#include <Hal.h>
#include <stdint.h>

#if defined(AVR)
#include <BH1750.h>
#endif

/*
 * BH1750 ambient light sensor at I2C address 0x23, read in one-shot
 * high-resolution mode. Same pattern as BMx280 / Ds18b20: hardware access on
 * AVR, stub on native builds.
 */
class LightSensor {
 public:
  explicit LightSensor(Hal *hal)
      : _hal(hal)
#if defined(AVR)
        ,
        _meter(0x23)
#endif
  {
  }

  // Starts a one-shot measurement, waits for it, returns lux.
  uint16_t Read() {
#if defined(AVR)
    _meter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
    for (uint8_t i = 0; i < 5; i++) {  // 150 ms conversion time
      _hal->Delay30ms();
    }
    return (uint16_t)_meter.readLightLevel();
#else
    return stubLux;
#endif
  }

#if !defined(AVR)
  uint16_t stubLux = 0;  // value returned on native builds
#endif

 private:
  Hal *_hal;
#if defined(AVR)
  BH1750 _meter;
#endif
};
