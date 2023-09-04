#pragma once

#include <Hal.h>
#include <math.h>
#include <stdint.h>

/*
 * Emulation of environment sensors using WS7000 family Lacrosse Technology
 * protocol
 *
 * Information about protocol from https://www.f6fbb.org/domo/sensors/
 *
 *
 *  - Temperature:                      emulates WS7000-27/28 (sensor type 0)
 *  - Temperature, humidity:            emulates WS7000-22/25 (sensor type 1)
 *  - Temperature, humidity, barometer: emulates WS7000-20    (sensor type 4)
 *  - Light:                            emulates WS2500-19    (sensor type 5)
 *
 *
 */
class LacrosseWS7000 {

public:
  enum Address : uint8_t { ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN };

  class NumericalSplit {
  public:
    NumericalSplit() : dozens(0), units(0), decimals(0), isNegative(false) {}

    NumericalSplit(float initialValue)
        : dozens(0), units(0), decimals(0), isNegative(false) {
      if (initialValue < 0) {
        isNegative = true;
        initialValue = -initialValue;
      } else {
        isNegative = false;
      }

      uint8_t integerValue = floor(initialValue);
      if (integerValue >= 100) {
        dozens = 10;
        units = 0;
        decimals = 0;
      } else {
        dozens = ((uint8_t)(floor(integerValue / 10))) & 0x0F;
        units = ((uint8_t)(floor(integerValue - (dozens * 10)))) & 0x0F;
        decimals = ((uint8_t)round((initialValue - integerValue) * 10)) & 0x0F;
      }
    }
    uint8_t dozens;
    uint8_t units;
    uint8_t decimals;
    bool isNegative;
  };

  class NumericalSplitHundreds {
  public:
    NumericalSplitHundreds()
        : hundreds(0), dozens(0), units(0), decimals(0), isNegative(false) {}

    NumericalSplitHundreds(float initialValue)
        : hundreds(0), dozens(0), units(0), decimals(0), isNegative(false) {

      if (initialValue < 0) {
        isNegative = true;
        initialValue = -initialValue;
      } else {
        isNegative = false;
      }
      uint16_t integerValue = floor(initialValue);
      uint16_t totalIntegerValue = integerValue;

      hundreds = ((uint8_t)(floor(integerValue / 100))) & 0x0F;
      integerValue -= 100 * hundreds;
      dozens = (uint8_t)(floor((integerValue / 10))) & 0x0F;
      integerValue -= dozens * 10;
      units = integerValue & 0x0F;
      decimals =
          ((uint8_t)round((initialValue - totalIntegerValue) * 10)) & 0x0F;
    }

    uint8_t hundreds;
    uint8_t dozens;
    uint8_t units;
    uint8_t decimals;
    bool isNegative;
  };

  LacrosseWS7000(Hal *hal);

  void SetAddress(Address address);

  void SetTemperature(float temperature);
  void SetHumidity(uint8_t humidity);
  void SetPressure(float pressure);
  void SetLight(uint16_t luminosity);

  void Send();

  void SendData(const uint8_t *data, uint8_t size);
  const unsigned char *GetMessage();

private:
  /**
   * 800µs pulse, 400µs pause
   */
  void SendZero();

  /**
   * 400µs pulse, 800µs pause
   */
  void SendOne();

  /**
   * Emit LSB of 'nibble' (lsb first) and a 'one' bit of separation
   */
  void SendNibble(uint8_t nibble);

  /**
   * 10 times 0
   */
  void SendPreamble();

  /**
   * Emit sensor type and return equivalent nibble
   */
  uint8_t SendType();

  /**
   * Emit sensor address and temperature sign and return equivalent nibble
   */
  uint8_t SendAddressAndTemperatureSign();

  void SendTemperature(uint8_t *nibbles);
  void SendHumidity(uint8_t *nibbles);
  void SendPressure(uint8_t *nibbles);
  void SendLuminosity(uint8_t *nibbles);

  NumericalSplit temperature;
  NumericalSplit humidity;
  NumericalSplitHundreds pressure;
  uint16_t luminosity;

  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   * bit 3 : light is set
   */
  uint8_t availableData = 0;

  Address address;
  Hal *_hal;
};