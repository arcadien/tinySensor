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
  float batteryVoltage;
  float vccVoltage;
  mutable bool I2CIsConfigured;

  TestHal_() {
    batteryVoltage = 2400; // AAA*2, little used
    vccVoltage = 3300;     // voltage after charge pump
  }

  void Delay30ms() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }

  void inline Delay1024Us() { Orders.push_back('P'); }
  void inline Delay512Us() { Orders.push_back('D'); }

  unsigned char *GetOrders() { return Orders.data(); }
  void ClearOrders() { Orders.clear(); }

  inline void RadioGoHigh() override { Orders.push_back('H'); }
  inline void RadioGoLow() override { Orders.push_back('L'); }

  void LedOn() override { IsLedOn = true; }
  void LedOff() override { IsLedOn = false; }

  void PowerOnSensors() override { SensorIsPowered = true; }
  void PowerOffSensors() override { SensorIsPowered = false; }

  uint16_t GetBatteryVoltageMv() override { return batteryVoltage; }
  uint16_t GetVccVoltageMv() override { return vccVoltage; }

  void Hibernate(uint8_t seconds) override {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
  }

  void InitI2C() override { I2CIsConfigured = true; }
};

extern TestHal_ TestHal;

#define OREGON_DELAY_US(x) TestHal.Delay(x);

#endif