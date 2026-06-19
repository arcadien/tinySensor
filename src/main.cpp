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
 * along with this program. If not, see <http://www.gnu.org/licenses/\>.
 */

#include <config.h>
#include <conversionTools.h>

#if defined(USE_BH1750)
#include <BH1750.h>
#endif

#if defined(USE_DS18B20)
#include <Ds18b20.h>
#endif

#if defined(USE_LACROSSE)
#include <LacrosseWS7000.h>
#elif defined(USE_OREGON)
#include <Oregon_v3.h>
#endif

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

#if defined(USE_DS18B20)
Ds18b20 ds18b20sensor(&hal);
#endif

#if defined(BATTERY_VOLTAGE_X10_ID) or defined(ANALOG1_X10_ID)
#include <x10rf.h>
x10rf x10encoder(&hal, 2);
#endif

#define NOT_SET (-1000.0f)
#define TRANSMIT_WITH_LED(code) \
  do {                          \
    hal.LedOn();                \
    code;                       \
    hal.LedOff();               \
    hal.Delay30ms();            \
  } while (0)

#if defined(USE_SERIAL_LOG)
#include <SoftSerial.h>
SoftSerial swSerial(&hal);

static void SerialPrintInfo(const char *name, uint16_t value,
                            const char *unit) {
  swSerial.print(name);
  swSerial.print(": ");
  swSerial.print(value);
  swSerial.print(unit);
  swSerial.println();
}
#else
static void SerialPrintInfo(const char *, uint16_t, const char *) {}
#endif

int main(void) {
  // Initialize hardware once at startup
  hal.Init();

  uint16_t batteryVoltageInMv = 0;
  bool lowBattery = false;

#if defined(USE_LACROSSE)
  LacrosseWS7000 environmentEncoder(&hal);
  environmentEncoder.SetAddress(LACROSSE_ID);
#elif defined(USE_OREGON)
  OregonV3 environmentEncoder(&hal);
  environmentEncoder.SetChannel(OREGON_CHANNEL);
  environmentEncoder.SetRollingCode(OREGON_RCODE);
#endif

  while (1) {
    hal.PowerOnSensors();

    float temperature = NOT_SET;
    float humidity = NOT_SET;
    float pressure = NOT_SET;

#if defined(USE_BME280) || defined(USE_BMP280)
    bmx280.Begin();
    hal.Delay30ms();
    temperature = bmx280.GetTemperature();
#if defined(USE_BME280)
    humidity = bmx280.GetHumidity();
    pressure = bmx280.GetPressure() / 100;
#endif
    bmx280.Shutdown();
#endif

#if defined(USE_DS18B20)
    ds18b20sensor.Convert();
    int16_t temp = ds18b20sensor.Read();
    if (temp != 0) {
      temperature = (float)temp;
    }
#endif

#if defined(BATTERY_VOLTAGE_X10_ID)
    uint16_t vccMv = hal.ComputeVccMv(INTERNAL_1v1);
#if defined(BATTERY_IS_VCC)
    batteryVoltageInMv = vccMv;
#else
    uint16_t rawBattery = hal.GetRawBattery();
    batteryVoltageInMv = hal.ConvertAnalogValueToMv(rawBattery, vccMv);
#endif
    lowBattery = (batteryVoltageInMv < LOW_BATTERY_VOLTAGE);
    SerialPrintInfo("Vbatt", batteryVoltageInMv, " mV");
    if (!lowBattery) {
      TRANSMIT_WITH_LED(
          x10encoder.RFXmeter(BATTERY_VOLTAGE_X10_ID, 0x00,
                              ConversionTools::dec16ToHex(batteryVoltageInMv)));
    }
#endif

#if defined(ANALOG1_X10_ID)
    if (!lowBattery) {
      uint16_t analogMeasurement = hal.GetRawAnalogSensor();
      SerialPrintInfo("Raw analog", analogMeasurement, " ADC");
      uint16_t analogVoltage =
          hal.ConvertAnalogValueToMv(analogMeasurement, vccMv);
      SerialPrintInfo("Analog voltage", analogVoltage, " mV");
      TRANSMIT_WITH_LED(x10encoder.RFXmeter(
          ANALOG1_X10_ID, 0x00, ConversionTools::dec16ToHex(analogVoltage)));
    }
#endif

#if defined(USE_BH1750)
    if (!lowBattery) {
      LacrosseWS7000 lightEncoder(&hal);
      lightEncoder.SetAddress(LACROSSE_ID);
      BH1750 lightMeter(0x23);
      lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
      _delay_ms(150);
      uint16_t lux = (uint16_t)lightMeter.readLightLevel();
      SerialPrintInfo("Luminosity", lux, " Lux");
      lightEncoder.SetLuminosity(lux);
      TRANSMIT_WITH_LED(lightEncoder.Send());
      TRANSMIT_WITH_LED(lightEncoder.Send());
    }
#endif

#if defined(USE_LACROSSE) or defined(USE_OREGON)
    if (temperature != NOT_SET) {
      environmentEncoder.SetTemperature(temperature);
    }
    if (humidity != NOT_SET) {
      environmentEncoder.SetHumidity(humidity);
    }
    if (pressure != NOT_SET) {
      environmentEncoder.SetPressure(pressure);
    }

#if defined(USE_SERIAL_LOG)
    SerialPrintInfo("Temp.", temperature * 10, " °C");
    SerialPrintInfo("RH ", humidity, " %");
    SerialPrintInfo("Pressure ", pressure, " hPA");
#endif

    if (temperature != NOT_SET) {
#if defined(USE_OREGON)
      environmentEncoder.SetBatteryLow(lowBattery);
#endif
      TRANSMIT_WITH_LED(environmentEncoder.Send());
    }
#endif

    hal.PowerOffSensors();
    hal.Hibernate((uint16_t)SLEEP_TIME_IN_SECONDS);
  }
}
