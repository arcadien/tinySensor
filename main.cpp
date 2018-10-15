/*
 * TinySensor.cpp
 *
 * Created: 02/10/2018 21:23:28
 * Author : aurelien
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <string.h> // memcpy

#include "protocol/oregon.h"
#include "sensors/bme280/SparkFunBME280.h"

#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))

volatile uint8_t sleep_interval;
ISR(WATCHDOG_vect) {
  wdt_reset();
  ++sleep_interval;
  // Re-enable WDT interrupt
  _WD_CONTROL_REG |= (1 << WDIE);
}

// Puts MCU to sleep for specified number of seconds
void sleep(uint8_t s) {
  s >>= 3; // or s/8
  if (s == 0)
    s = 1;
  sleep_interval = 0;
  while (sleep_interval < s) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();

    // cut power to USI and Timer 0
    uint8_t PRR_backup = PRR;
    PRR |= (1 << PRUSI);
    PRR |= (1 << PRTIM0);

    sleep_mode();
    // here the system is shut down

    // here the system wakes up
    sleep_disable();

    // restore power on USI and Timer 0
    PRR = PRR_backup;
  }
}

void Setup() {
  wdt_disable();
  
  // 
  // Watchdog setup - 8s sleep time
  // 
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);

  // all pins as input to avoid power draw
  DDRA = 0;
  DDRB = 0;
  
  // pull-up all pins by default,
  // but PA7 (tx) and PB1 (led)
  PORTA |= 0b01111111;
  PORTB |= 0b11111101;

  PRR &= ~_BV(PRTIM1);    // no timer1
  PRR &= ~_BV(PRADC);     // no adc
  ADCSRA &= ~(1 << ADEN); // p. 146

  ACSR |= (1 << ACD); // no analog comparator (p. 129)

  // deactivate brownout detection during sleep (p.36)
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR |= (1 << BODS);
  MCUSR &= ~(1 << BODSE);

  // don't use ADC buffers (p. 131)
  DIDR0 |= (1 << ADC2D) | (1 << ADC1D);

  // radio TX pin is PA7
  // set as output
  DDRA |= _BV(PA7);

  // PA2 is led pin
  DDRB |= _BV(PB1);
}

Oregon oregon(2);
BME280 bme280;

int main(void) {

  Setup();

  const uint8_t oregonType[] = {0x5A, 0x5D};
  oregon.setType(oregon._oregonMessageBuffer, oregonType);
  oregon.setChannel(oregon._oregonMessageBuffer, Oregon::Channel::TWO);
  oregon.setId(oregon._oregonMessageBuffer, 0xBA);

  bme280.setI2CAddress(0x76);
  bme280.beginI2C();
  bme280.setStandbyTime(5);

// Buffer for Oregon message
#if MODE == MODE_0
  uint8_t _lastOregonMessageBuffer[8] = {};
#elif MODE == MODE_1
  uint8_t _lastOregonMessageBuffer[9] = {};
#elif MODE == MODE_2
  uint8_t _lastOregonMessageBuffer[11] = {};
#else
#error mode unknown
#endif

  while (1) {

    Wire.begin();

    PORTB |= _BV(PB1); // led on
    oregon.setBatteryLevel(oregon._oregonMessageBuffer, 1);
    oregon.setTemperature(oregon._oregonMessageBuffer, bme280.readTempC());
    oregon.setHumidity(oregon._oregonMessageBuffer, bme280.readFloatHumidity());
    oregon.setPressure(oregon._oregonMessageBuffer,
                       (bme280.readFloatPressure() / 100));
    oregon.calculateAndSetChecksum(oregon._oregonMessageBuffer);

    Wire.end();

    // compare bytes 4, 5, 6 and 7 (temp and humidity)
    bool hasChanged =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7]);

    if (hasChanged) {

      // led off here, it will allow a short
      // blink when actually emitting new data
      PORTB &= ~_BV(PB1);

// Buffer for Oregon message
#if MODE == MODE_0
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 8);
#elif MODE == MODE_1
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 9);
#elif MODE == MODE_2
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 11);
#else
#error mode unknown
#endif

      oregon.txPinLow();
      _delay_us(oregon.TWOTIME * 8);

      oregon.sendOregon(oregon._oregonMessageBuffer,
                        sizeof(oregon._oregonMessageBuffer));

      // pause before new transmission
      oregon.txPinLow();
      _delay_us(oregon.TWOTIME * 8);

      // second emission with led on
      PORTB |= _BV(PB1);

      oregon.sendOregon(oregon._oregonMessageBuffer,
                        sizeof(oregon._oregonMessageBuffer));
    }
    PORTB &= ~_BV(PB1);

    sleep(64);
  }
}
