#pragma once

#include <Hal.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Emulation of environment sensors using WS7000 family Lacrosse Technology
 * protocol
 *
 * Information about protocol from https://www.f6fbb.org/domo/sensors/
 * rtl_433 decoder:
 * https://github.com/merbanan/rtl_433/blob/master/src/devices/lacrosse_ws7000.c
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

  class NumericalValueSplitter {
   public:
    struct Result {
      Result()
          : isNegative(false), hundreds(0), dozens(0), units(0), decimals(0) {}

      Result(bool a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
          : isNegative(a), hundreds(b), dozens(c), units(d), decimals(e) {}

      bool isNegative;
      uint8_t hundreds;
      uint8_t dozens;
      uint8_t units;
      uint8_t decimals;
    };

    // Rounded to tenths (halves away from zero) with the carry, so that no
    // digit exceeds 9; the sign is the rounded value's.
    static void Split(float input, Result *result);
  };

  LacrosseWS7000(Hal *hal);

  void SetAddress(Address address);

  void SetTemperature(float temperature);
  void SetHumidity(float humidity);
  void SetPressure(float pressure);
  void SetLuminosity(uint16_t luminosity);

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

  void SendType(uint8_t *nibbles);
  void SendAddressAndTemperatureSign(uint8_t *nibbles);
  void SendTemperature(uint8_t *nibbles);
  void SendHumidity(uint8_t *nibbles);
  void SendPressure(uint8_t *nibbles);
  void SendLuminosity(uint8_t *nibbles);

  NumericalValueSplitter::Result temperature;
  NumericalValueSplitter::Result humidity;
  NumericalValueSplitter::Result pressure;
  NumericalValueSplitter::Result luminosity;
  Address address;

  Hal *_hal;

  /*
   * bit 0 : temperature is set
   * bit 1 : humidity is set
   * bit 2 : pressure is set
   * bit 3 : light is set
   */
  uint8_t availableData = 0;
};