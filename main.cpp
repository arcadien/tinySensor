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
#include <string.h>
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

/*
 * Puts MCU to sleep for specified number of seconds
 *
 * As the hardware watchdog is set for timeout after 8s,
 * the value used here will be divided by 8 (and rounded if needed).
 * It is better to use a multiple of 8 as value.
 */
void sleep(uint8_t s);

/*
 * Initial board setup
 *
 * This is where pins and various registers are set
 * for maximized power saving.
 */
void setup();

/*!
 * Entry point of the firmware.
 *
 * Will call setup, and enter in an infinite loop
 * where some code is executed, then the MCU is put
 * to deep sleep during a fixed number of seconds, then
 * the loop is processed again, aet-caetera.
 *
 */
int main(void);

/*
 * The firmware force emission of
 * a signal every quarter of an hour
 * even if sensor data does not change,
 * just to be sure sensor is not out of
 * power or bugged (900s)
 */
volatile uint16_t secondCounter;

/*!
 * Interrupt routine called each 8 seconds in the
 * default setup, which is the longest timeout period
 * accepted by the hardware watchdog
 */
volatile uint8_t sleep_interval;
ISR(WATCHDOG_vect) {
  wdt_reset();
  ++sleep_interval;
  // Re-enable WDT interrupt
  _WD_CONTROL_REG |= (1 << WDIE);
}

#if defined(USE_OREGON)
Oregon oregon;
#endif

#if defined(USE_BME280)
BME280 bme280;
#endif

void setup() {
  wdt_disable();

  secondCounter = 0;

  //
  //// Watchdog setup - 8s sleep time
  //
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);

  //
  //// unused pins are input + pullup, as stated
  //// in Atmel's doc "AVR4013: picoPower Basics"
  //

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

int main(void) {

  setup();

#if defined(USE_OREGON)
  oregon.setType(oregon._oregonMessageBuffer, OREGON_TYPE);
  oregon.setChannel(oregon._oregonMessageBuffer, Oregon::Channel::ONE);
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

    bool shouldEmitData = false;
#if defined(USE_OREGON)

#if (OREGON_MODE == MODE_0)
    // temp update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6]);

#elif (OREGON_MODE == MODE_1)
    // temp or humidity update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7]);
#elif (OREGON_MODE == MODE_2)
    // temp, humidity or pressure update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7] ||
         oregon._oregonMessageBuffer[8] != _lastOregonMessageBuffer[8] ||
         oregon._oregonMessageBuffer[9] != _lastOregonMessageBuffer[9]);
#else
#error OREGON_MODE has an unknown value
#endif

    // absolute counter for emission ~ each 15 minutes
    if (secondCounter > 900) {
      secondCounter = 0;
      shouldEmitData = true;
    }

    PORTB |= _BV(LED_PIN); // led on

    if (shouldEmitData) {

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
#elif OREGON_MODE == MODE_2
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
    sleep(SLEEP_TIME_IN_SECONDS);
    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}

/*
 * Puts MCU to sleep for specified number of seconds
 */
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
