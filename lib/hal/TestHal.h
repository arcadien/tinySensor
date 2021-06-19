#pragma once
#include <Hal.h>
#include <cstdint>
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

  void DelayUs(long delay) const {
    if (delay == 1024) // DELAY_US
    {
      DelayPeriod();
    } else if (delay == 512) // HALF_DELAY_US
    {
      DelayHalfPeriod();
    }
  }

  void DelayPeriod() const { Orders.push_back('P'); }
  void DelayHalfPeriod() const { Orders.push_back('D'); }

  unsigned char *GetOrders() { return Orders.data(); }
  void ClearOrders() { Orders.clear(); }
  inline void RadioGoHigh() const override { Orders.push_back('H'); }
  inline void RadioGoLow() const override { Orders.push_back('L'); }

  void LedOn() const override {
      IsLedOn = true;
  }
  void LedOff() const override {
      IsLedOn = false;
  }
};

extern TestHal_ TestHal;

#define OREGON_DELAY_US(x) TestHal.Delay(x);