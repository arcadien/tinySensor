/*
 * TinySensor.cpp
 *
 * Created: 02/10/2018 21:23:28
 * Author : aurelien
 */

#include "config.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h> // memcpy
#include <util/delay.h>

#if defined(USE_OREGON)
#include "protocol/oregon.h"
#endif

#if defined(USE_BME280)
#include "sensors/bme280/SparkFunBME280.h"
#endif

#if defined(USE_DS18B20)
#include "sensors/ds18b20/include/ds18b20/ds18b20.h"
#endif

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

  // unused pins are input + pullup, as stated
  // in Atmel's doc "AVR4013: picoPower Basics"

  // all pins as input to avoid power draw
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  //
  PORTA |= 0b01111001;
  PORTB |= 0b11110110;

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

  // set output pins
  DDRB |= _BV(TX_RADIO_PIN);
  DDRA |= _BV(RADIO_POWER_PIN);
  DDRB |= _BV(LED_PIN);
}

#if defined(USE_OREGON)
Oregon oregon(2);
#endif

#if defined(USE_BME280)
BME280 bme280;
#endif

int main(void) {

  Setup();

#if defined(USE_OREGON)
  oregon.setType(oregon._oregonMessageBuffer, OREGON_TYPE);
  oregon.setChannel(oregon._oregonMessageBuffer, Oregon::Channel::TWO);
  oregon.setId(oregon._oregonMessageBuffer, OREGON_ID);
#endif

#if defined(USE_BME280)
  bme280.setI2CAddress(0x76);
  bme280.beginI2C();
  bme280.setStandbyTime(5);
#endif

#if defined(USE_OREGON)
// Buffer for Oregon message
#if OREGON_MODE == MODE_0
  uint8_t _lastOregonMessageBuffer[8] = {};
#elif OREGON_MODE == MODE_1
  uint8_t _lastOregonMessageBuffer[9] = {};
#elif OREGON_MODE == MODE_2
  uint8_t _lastOregonMessageBuffer[11] = {};
#else
#error mode unknown
#endif
#endif

  while (1) {

#if defined(USE_OREGON) && defined(USE_BME280)
    Wire.begin();

    oregon.setBatteryLevel(oregon._oregonMessageBuffer, 1);
    oregon.setTemperature(oregon._oregonMessageBuffer, bme280.readTempC());
    oregon.setHumidity(oregon._oregonMessageBuffer, bme280.readFloatHumidity());
    oregon.setPressure(oregon._oregonMessageBuffer,
                       (bme280.readFloatPressure() / 100));
    oregon.calculateAndSetChecksum(oregon._oregonMessageBuffer);

    Wire.end();
#endif

#if defined(USE_DS18B20)

    ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr);

    // Delay (sensor needs time to perform conversion)
    _delay_ms(1000);

    // Read temperature (without ROM matching)
    int16_t temperature = 0;
    ds18b20read(&PORTA, &DDRA, &PINA, (1 << 3), nullptr, &temperature);

    oregon.setBatteryLevel(oregon._oregonMessageBuffer, 1);
    oregon.setTemperature(oregon._oregonMessageBuffer, temperature / 16);
    oregon.calculateAndSetChecksum(oregon._oregonMessageBuffer);

#endif

#if defined(USE_OREGON)

#if (OREGON_MODE == MODE_0)
    // temp update?
    bool hasChanged =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6]);

#elif (OREGON_MODE == MODE_1)
    // temp or humidity update?
    bool hasChanged =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7]);
#elif (OREGON_MODE == MODE_2)
    // temp, humidity or pressure update?
    bool hasChanged =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7] ||
         oregon._oregonMessageBuffer[8] != _lastOregonMessageBuffer[8] ||
         oregon._oregonMessageBuffer[9] != _lastOregonMessageBuffer[9]);
#else
#error OREGON_MODE has an uknown value
#endif

    PORTB |= _BV(LED_PIN); // led on

    if (hasChanged) {

      // Activate radio power
      PORTA |= _BV(RADIO_POWER_PIN);
      _delay_ms(20);

      // led off here, it will allow a short
      // blink when actually emitting new data
      PORTB &= ~_BV(LED_PIN);

// Buffer for Oregon message
#if OREGON_MODE == MODE_0
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 8);
#elif OREGON_MODE == MODE_1
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 9);
#elif MOREGON_MODEODE == MODE_2
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
      PORTB |= _BV(LED_PIN);

      oregon.sendOregon(oregon._oregonMessageBuffer,
                        sizeof(oregon._oregonMessageBuffer));

      // radio off
      PORTA &= ~_BV(RADIO_POWER_PIN);
    }
    // led off
    PORTB &= ~_BV(LED_PIN);

#endif
    sleep(8);
  }
}
