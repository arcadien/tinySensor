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

#include <config.h>
#include <Oregon_v3.h>

#if defined(__AVR_ATtiny84__)
#include <Attiny84aHal.h>
Attiny84aHal hal;
#else
#include <TestHal.h>
TestHal_ hal;
#endif


#if defined(USE_BME280) or defined(USE_BMP280)
#include <BMx280.h>
BMx280 bmx280(&hal);
#endif

#if defined(VOLTAGE_X10_SENSOR_ID)
#include <x10.h>
x10 voltageEmitter;
#endif

/*
#if defined(USE_DS18B20)
#include <ds18b20.h>
#endif
*/


int main(void)
{
  OregonV3 oregon(&hal);
  oregon.SetChannel(OREGON_CHANNEL);
  oregon.SetRollingCode(OREGON_ID);

  /*
   * The firmware force emission of
   * a signal at boot and every quarter of an hour
   * even if sensor data does not change,
   * just to be sure sensor is not out of
   * power or bugged (900s)
   */
  volatile uint16_t secondCounter = 901;

  while (1)
  {

    hal.PowerOnSensors();
    hal.Delay30ms();
    bool batteryIsLow = false;

#if defined(USE_I2C)
    hal.InitI2C();
#endif

#if defined(USE_BME280) || defined(USE_BMP280)
    bmx280.Begin();
    oregon.SetTemperature(bmx280.GetTemperature());
#if defined(USE_BME280)
    oregon.SetHumidity(bmx280.GetHumidity());
    oregon.SetPressure(bmx280.GetPressure() / 100);
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
    if (readStatus == DS18B20_ERROR_OK)
    {
      oregon.SetTemperature(temperature / 16);
    }

#endif

    bool shouldEmitVoltage = false;

    // absolute counter for emission ~ each 15 minutes
    if (secondCounter > 900)
    {
      secondCounter = 0;
      shouldEmitVoltage = true;
    }

    if (shouldEmitVoltage)
    {

      uint16_t voltageInMv = hal.GetBatteryVoltageMv();
      batteryIsLow = (voltageInMv < LOW_BATTERY_VOLTAGE);

#if defined(VOLTAGE_X10_SENSOR_ID)
      voltageEmitter.RFXmeter(VOLTAGE_X10_SENSOR_ID, 0, voltageInMv);
      hal.Delay30ms();
#endif
    }

    if (batteryIsLow)
    {
      oregon.SetBatteryLow();
    }

    hal.LedOn();
    oregon.Send();
    hal.LedOff();
    hal.Delay30ms();
    hal.LedOn();
    oregon.Send();
    hal.LedOff();
    hal.PowerOffSensors();

    hal.Hibernate((uint8_t)SLEEP_TIME_IN_SECONDS);
    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}
