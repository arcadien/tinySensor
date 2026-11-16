#pragma once
#include <stdint.h>

/*
 * Values read during one measurement cycle.
 *
 * Each field is only meaningful when its flag is set: a sensor that is not
 * fitted, or that failed to read, simply leaves its flag cleared. This
 * replaces the former "NOT_SET = -1000" sentinel floats.
 */
struct Measurements {
  enum Field : uint8_t {
    TEMPERATURE = 0x01,
    HUMIDITY = 0x02,
    PRESSURE = 0x04,
    LUMINOSITY = 0x08,
  };

  float temperature = 0;    // °C
  float humidity = 0;       // %RH
  float pressure = 0;       // hPa
  uint16_t luminosity = 0;  // lux
  uint8_t present = 0;      // bit field of Field

  bool Has(Field field) const { return (present & field) != 0; }

  void SetTemperature(float value) {
    temperature = value;
    present |= TEMPERATURE;
  }
  void SetHumidity(float value) {
    humidity = value;
    present |= HUMIDITY;
  }
  void SetPressure(float value) {
    pressure = value;
    present |= PRESSURE;
  }
  void SetLuminosity(uint16_t value) {
    luminosity = value;
    present |= LUMINOSITY;
  }
};
