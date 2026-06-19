#pragma once
#include <Hal.h>

#if defined(AVR)
#include <avr/io.h>
#include <ds18b20.h>
#endif

class Ds18b20 {
#if defined(AVR)
 public:
  Ds18b20(Hal *hal) : _hal(hal) {}
  void Begin() {}
  void Convert() {
    ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr);
    _hal->Delay1s();
  }
  int16_t Read() {
    int16_t temp = 0;
    auto status = ds18b20read(&PORTA, &DDRA, &PINA, (1 << 3), nullptr, &temp);
    if (status == DS18B20_ERROR_OK) {
      return temp / 16;
    }
    return 0;
  }
#else
 public:
  Ds18b20(Hal *hal) : _hal(hal) {}
  void Begin() {}
  void Convert() {}
  int16_t Read() {
    return 0;
  }
#endif

 private:
  Hal *_hal;
};
