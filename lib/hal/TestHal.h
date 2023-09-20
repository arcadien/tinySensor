#pragma once

#ifndef AVR

#include <Hal.h>
#include <chrono>
#include <stdint.h>
#include <thread>
#include <vector>

/**
 * Provides a test-enabled stub of HAL
 *
 *
 */
class TestHal_ : public Hal {
public:
  mutable std::vector<unsigned char> Orders;
  mutable bool IsLedOn;
  mutable bool SensorIsPowered;
  uint16_t rawBattery;
  uint16_t rawInternal11ref;
  uint16_t rawAnalogSensor;

  mutable bool I2CIsConfigured;

  TestHal_() {
    rawBattery = 4200;
    rawInternal11ref = 1095;
    rawAnalogSensor = 2442;
  }

  void Delay1s() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  void Delay30ms() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }

  void inline Delay400Us() { Orders.push_back('A'); }

  void inline Delay1024Us() { Orders.push_back('P'); }
  void inline Delay512Us() { Orders.push_back('D'); }

  unsigned char *GetOrders() { return Orders.data(); }
  void ClearOrders() { Orders.clear(); }
 
  inline void RadioGoHigh() override { Orders.push_back('H'); }
  inline void RadioGoLow() override { Orders.push_back('L'); }

  inline void SerialGoHigh() override { Orders.push_back('S'); }
  inline void SerialGoLow() override { Orders.push_back('W'); }

  void LedOn() override { IsLedOn = true; }
  void LedOff() override { IsLedOn = false; }

  void PowerOnSensors() override { SensorIsPowered = true; }
  void PowerOffSensors() override { SensorIsPowered = false; }

  uint16_t GetRawBattery() override { return rawBattery; }
  uint16_t GetRawInternal11Ref() override { return rawInternal11ref; }
  uint16_t GetRawAnalogSensor() override { return rawAnalogSensor; }

  void Hibernate(uint16_t seconds) override {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }

  void Init() override { I2CIsConfigured = true; }
};

extern TestHal_ TestHal;

#define OREGON_DELAY_US(x) TestHal.Delay(x);

#endif