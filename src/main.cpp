/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019 Aurélien Labrosse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#if defined(AVR)

#include <config.h>

#include <Oregon_TM.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <readVcc.h>
#include <string.h>

#if defined(VOLTAGE_X10_SENSOR_ID)
#include <x10rf.h>
x10rf voltageX10Sensor = x10rf(TX_RADIO_PIN, LED_PIN, 3);
#endif

#if defined(OREGON_MODE)
Oregon_TM oregon = Oregon_TM(&PORTB, TX_RADIO_PIN, OREGON_NIBBLES_COUNT);
#endif

#if defined(USE_DS18B20)
#include <ds18b20.h>
#endif

#if defined(USE_BME280) or defined(USE_BMP280)
#include <SparkFunBME280.h>
BME280 bmX280;
#endif

void UseLessPowerAsPossible() {
  // AVR4013: picoPower Basics
  // unused pins should be set as
  // input + pullup to minimize consumption
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  //
  PORTA |= 0b01111001;
  PORTB |= 0b11111100;

  // no timer1
  PRR &= ~_BV(PRTIM1);

  // no timer0
  PRR &= ~_BV(PRTIM0);

  // no analog p. 129, 146, 131
  ADCSRA = 0;
  /*
  ADCSRA &= ~(1 << ADEN);
  DIDR0 |= (1 << ADC2D) | (1 << ADC1D); // buffers
  */

  // Analog comparator power down
  ACSR |= (1 << ACD);

  // deactivate brownout detection during sleep (p.36)
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR |= (1 << BODS);
  MCUSR &= ~(1 << BODSE);
}

/*
 * The firmware force emission of
 * a signal at boot and every quarter of an hour
 * even if sensor data does not change,
 * just to be sure sensor is not out of
 * power or bugged (900s)
 */
uint16_t secondCounter;

/*!
 * Interrupt routine called each 8 seconds in the
 * default setup, which is the longest timeout period
 * accepted by the hardware watchdog
 */
volatile uint8_t sleep_interval;
ISR(WATCHDOG_vect) {
  ++sleep_interval;
  // Re-enable WDT interrupt
  _WD_CONTROL_REG |= (1 << WDIE);
}

void setup() {

  UseLessPowerAsPossible();

  // 901 so that first emission is at boot
  secondCounter = 901;

  // Watchdog setup - 8s sleep time
  MCUSR = 0x00;
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);
  wdt_reset();

  // set output pins
  DDRB |= _BV(TX_RADIO_PIN);
  DDRA |= _BV(SENSOR_VCC);
  DDRB |= _BV(LED_PIN);

#if defined(USE_BME280) || defined(USE_BMP280)
  bmX280.setI2CAddress(0x76);
#endif
}

/*
 * Puts MCU to sleep for specified number of seconds
 *
 * As the hardware watchdog is set for timeout after 8s,
 * the value used here will be divided by 8 (and rounded if needed).
 * It is better to use a multiple of 8 as value.
 */
void sleep(uint16_t s) {

  cli();
  sleep_interval = 0;
  s = (uint16_t)(s / 8);
  if (s == 0)
    s = 1;

  uint8_t PRR_backup = PRR;
  uint8_t porta_backup = PORTA;
  uint8_t portb_backup = PORTB;
  uint8_t ddra_backup = DDRA;
  uint8_t ddrb_backup = DDRB;

  PRR |= (1 << PRUSI) | (1 << PRTIM0) | (1 << PRTIM1) | (1 << PRADC);

  // all pins as input to avoid power draw
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  PORTA |= 0b01110000;

  // pullup reset only
  // ( only 4 lasts are available - p. 67)
  PORTB |= 0b00001000;

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  while (sleep_interval < s) {
    wdt_reset();
    sei();
    sleep_mode();
  }
  sleep_disable();

  // restore
  PRR = PRR_backup;
  DDRA = ddra_backup;
  DDRB = ddrb_backup;
  PORTA = porta_backup;
  PORTB = portb_backup;
}

int avr_main(void) {

  bool batteryIsLow = false;
  uint16_t voltageInMv = 0;

  setup();

#if defined(OREGON_MODE)
  oregon.setType(OREGON_TYPE);
  oregon.setChannel(OREGON_CHANNEL);
  oregon.setId(OREGON_ID);
#endif

  while (1) {

    voltageInMv = 0;

    // led on
    PORTB |= _BV(LED_PIN);

    // sensors and RF power
    PORTA |= _BV(SENSOR_VCC);

#if defined(USE_I2C)
    TinyI2C.init();
#endif

#if defined(USE_BME280) || defined(USE_BMP280)
    bmX280.beginI2C();
    _delay_ms(5);
    oregon.setTemperature(bmX280.readTempC());

#if defined(USE_BME280)
    oregon.setHumidity(bmX280.readFloatHumidity());
    oregon.setPressure((bmX280.readFloatPressure() / 100));
#endif
#endif

#if defined(USE_DS18B20)

    ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr);

    // Delay (sensor needs time to perform conversion)
    _delay_ms(1000);

    // Read temperature (without ROM matching)
    int16_t temperature = 0;
    auto readStatus =
        ds18b20read(&PORTA, &DDRA, &PINA, (1 << 3), nullptr, &temperature);
    if (readStatus == DS18B20_ERROR_OK) {
      oregon.setTemperature(temperature / 16);
    }

#endif

    // absolute counter for emission ~ each 15 minutes
    if (secondCounter > 900) {
#if defined(USE_CHARGE_PUMP)
      voltageInMv = readBatteryVoltage();
#else
      voltageInMv = readVcc();
#endif
      batteryIsLow = (voltageInMv < LOW_BATTERY_VOLTAGE);

#if defined(VOLTAGE_X10_SENSOR_ID)
      voltageX10Sensor.RFXmeter(VOLTAGE_X10_SENSOR_ID, 0, voltageInMv);
#endif
      secondCounter = 0;
    }

#if defined(OREGON_MODE)
    oregon.setBatteryFlag(batteryIsLow ? true : false);
    oregon.transmit();
#endif

    // sensor power off
    PORTA &= ~_BV(SENSOR_VCC);

    _delay_ms(30);
    PORTB &= ~_BV(LED_PIN);

    sleep(SLEEP_TIME_IN_SECONDS);

    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}
#endif