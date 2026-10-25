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
#include <Measurements.h>

#if defined(__AVR_ATtiny84__)
#include <Attiny84aHal.h>
Attiny84aHal hal;
#else
#include <TestHal.h>
TestHal_ hal;
#endif

#if defined(USE_LACROSSE) or defined(USE_OREGON)
#include <EnvironmentEncoder.h>
#define HAS_ENVIRONMENT_ENCODER
#endif

#if defined(USE_BME280) or defined(USE_BMP280)
#include <BMx280.h>
BMx280 bmx280(&hal);
#endif

#if defined(USE_DS18B20)
#include <Ds18b20.h>
Ds18b20 ds18b20sensor(&hal);
#endif

#if defined(USE_BH1750)
// Luminosity always goes out as a Lacrosse type-5 frame: Oregon has no light
// frame, and a Lacrosse frame carrying luminosity carries nothing else.
#include <LacrosseWS7000.h>
#include <LightSensor.h>
LightSensor lightSensor(&hal);
LacrosseWS7000 lightEncoder(&hal);
#define LIGHT_FRAME_REPEAT 2  // sent twice for reception reliability
#endif

#if defined(BATTERY_VOLTAGE_X10_ID) or defined(ANALOG1_X10_ID)
#include <x10rf.h>
x10rf x10encoder(&hal, 2);
#endif

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

// Temperature, humidity and pressure from the fitted environment sensors.
static Measurements ReadEnvironment() {
  Measurements m;
#if defined(USE_BME280) || defined(USE_BMP280)
  bmx280.Begin();
  hal.Delay30ms();
  m.SetTemperature(bmx280.GetTemperature());
#if defined(USE_BME280)
  m.SetHumidity(bmx280.GetHumidity());
  m.SetPressure(bmx280.GetPressure() / 100);
#endif
  bmx280.Shutdown();
#endif

#if defined(USE_DS18B20)
  ds18b20sensor.Convert();
  int16_t temp = ds18b20sensor.Read();
  if (temp != 0) {  // 0 = read error
    m.SetTemperature((float)temp);
  }
#endif
  return m;
}

int main(void) {
  // Initialize hardware once at startup
  hal.Init();

  uint16_t batteryVoltageInMv = 0;
  bool lowBattery = false;

#if defined(HAS_ENVIRONMENT_ENCODER)
  EnvironmentEncoder environmentEncoder(&hal);
  ConfigureEncoder(environmentEncoder);
#endif

#if defined(USE_BH1750)
  lightEncoder.SetAddress(LACROSSE_ID);
#endif

  while (1) {
    hal.PowerOnSensors();

    Measurements measurements = ReadEnvironment();

#if defined(BATTERY_VOLTAGE_X10_ID) or defined(ANALOG1_X10_ID)
    uint16_t vccMv = hal.ComputeVccMv(INTERNAL_1v1);
#endif

#if defined(BATTERY_VOLTAGE_X10_ID)
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
      measurements.SetLuminosity(lightSensor.Read());
      SerialPrintInfo("Luminosity", measurements.luminosity, " Lux");
      lightEncoder.SetLuminosity(measurements.luminosity);
      for (uint8_t i = 0; i < LIGHT_FRAME_REPEAT; i++) {
        TRANSMIT_WITH_LED(lightEncoder.Send());
      }
    }
#endif

#if defined(HAS_ENVIRONMENT_ENCODER)
    bool sendEnvironment =
        ApplyMeasurements(environmentEncoder, measurements, lowBattery);

    if (measurements.Has(Measurements::TEMPERATURE)) {
      SerialPrintInfo("Temp.", measurements.temperature * 10, " °C");
    }
    if (measurements.Has(Measurements::HUMIDITY)) {
      SerialPrintInfo("RH ", measurements.humidity, " %");
    }
    if (measurements.Has(Measurements::PRESSURE)) {
      SerialPrintInfo("Pressure ", measurements.pressure, " hPA");
    }

    if (sendEnvironment) {
      TRANSMIT_WITH_LED(environmentEncoder.Send());
    }
#endif

    hal.PowerOffSensors();
    hal.Hibernate((uint16_t)SLEEP_TIME_IN_SECONDS);
  }
}
