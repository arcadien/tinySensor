/*
 * This file is part of the TinySensor distribution
 * (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019 Aurelien Labrosse
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

#include <BMx280.h>
#include <config.h>
#include <conversionTools.h>
#include <x10rf.h>

#if defined(AVR)
#include <ds18b20.h>
#endif

#include <LacrosseWS7000.h>

#if defined(__AVR_ATtiny84__)
#include <Attiny84aHal.h>
Attiny84aHal hal;
#else
#include <TestHal.h>
TestHal_ hal;
#endif

#if defined(USE_BME280) or defined(USE_BMP280)
BMx280 bmx280(&hal);
#endif

#if defined(VOLTAGE_X10_SENSOR_ID)
x10rf x10encoder(&hal, 5);
#endif

int main(void) {
  LacrosseWS7000 lacrosseEncoder(&hal);
  lacrosseEncoder.SetAddress(SENSOR_ID);

  /*
   * The firmware force emission of
   * a signal at boot and every quarter of an hour
   * even if sensor data does not change,
   * just to be sure sensor is not out of
   * power or bugged (900s)
   */
  volatile uint16_t secondCounter = 901;

  while (1) {

    hal.Init();
    hal.PowerOnSensors();
    hal.Delay30ms();
    hal.LedOn();

    static const float NOT_SET = -1000;
    float temperature = NOT_SET;
    float humidity = NOT_SET;
    float pressure = NOT_SET;

#if defined(USE_BME280) || defined(USE_BMP280)
    bmx280.Begin();
    temperature = bmx280.GetTemperature();
#if defined(USE_BME280)
    humidity = bmx280.GetHumidity();
    pressure = bmx280.GetPressure() / 100; // Pa -> hPa
#endif
#endif

#if defined(USE_DS18B20)

    ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr);

    // Delay (sensor needs time to perform conversion)
    hal.Delay1s();

    // Read temperature (without ROM matching)
    int16_t temperature = 0;
    auto readStatus =
        ds18b20read(&PORTA, &DDRA, &PINA, (1 << 3), nullptr, &temperature);
    if (readStatus == DS18B20_ERROR_OK) {
      temperature = (temperature / 16);
    }

#endif

#if defined(VOLTAGE_X10_SENSOR_ID)

    if (secondCounter > 900) {
      secondCounter = 0;
      uint16_t batteryVoltageInMv;
#ifdef BATTERY_IS_VCC
      batteryVoltageInMv = hal.GetVccVoltageMv();
#else
      batteryVoltageInMv = hal.GetBatteryVoltageMv();
#endif
      x10encoder.RFXmeter(VOLTAGE_X10_SENSOR_ID, 0x00,
                          ConversionTools::dec16ToHex(batteryVoltageInMv));
      hal.Delay30ms();
    }
#endif

    if (temperature != NOT_SET) {
      lacrosseEncoder.SetTemperature(temperature);
      // x10encoder.RFXsensor(TEMP_SENSOR_ID, 't', 't', temperature);
    }
    if (humidity != NOT_SET) {
      lacrosseEncoder.SetHumidity(humidity);
      // x10encoder.RFXsensor(HUM_SENSOR_ID, 'a', 'h', humidity);
    }
    if (pressure != NOT_SET) {
      lacrosseEncoder.SetPressure(pressure);
      // x10encoder.RFXsensor(PRESSURE_SENSOR_ID, 'a', 'p', (pressure/10));
    }

    lacrosseEncoder.Send();
    hal.Delay30ms();
    lacrosseEncoder.Send();
    hal.Delay30ms();

    hal.LedOff();
    hal.PowerOffSensors();
    hal.Hibernate((uint16_t)SLEEP_TIME_IN_SECONDS);
    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}
