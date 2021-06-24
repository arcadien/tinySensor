#pragma once
#include <Hal.h>

#if defined(AVR)
#include <SparkFunBME280.h>
#endif
class BMx280 {

#if defined(AVR)
  BME280 _bmX280;

public:

  BMx280() {
    _bmX280.setI2CAddress(0x76);
    _bmX280.beginI2C();
  }

  float GetTemperature() { return _bmX280.readTempC(); }
  float GetPressure() { return _bmX280.readFloatPressure(); }
  float GetHumidity() { return _bmX280.readFloatHumidity(); }
#else
public:
  float GetTemperature() { return 0; }
  float GetPressure() { return 0; }
  float GetHumidity() { return 0; }
#endif

  BMx280(Hal *hal) : hal(hal) {}

private:
  Hal *hal;
};