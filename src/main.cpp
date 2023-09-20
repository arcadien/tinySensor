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
#include <conversionTools.h>

#if defined(USE_BH1750)
#include <BH1750.h>
#endif

#if defined(USE_DS18B20)
#include <ds18b20.h>
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

#if defined(BATTERY_VOLTAGE_X10_ID) or defined(ANALOG1_X10_ID)
#include <x10rf.h>
x10rf x10encoder(&hal, 2);
#endif

static const char *MILLIVOLT = " mV";
static const char *ANALOG = " ADC";
static const char *LUX = " Lux";
static const char *SECOND = " s";
static const char *CELCIUS = " °C";
static const char *PERCENT = " %";
static const char *HECTOPASCAL = " hPA";

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

  while (1) {

#if defined(USE_LACROSSE)
    LacrosseWS7000 environmentEncoder(&hal);
    environmentEncoder.SetAddress(LACROSSE_ID);
#elif defined(USE_OREGON)
    OregonV3 environmentEncoder(&hal);
    environmentEncoder.SetChannel(OREGON_CHANNEL);
    environmentEncoder.SetRollingCode(OREGON_RCODE);
#endif

#if defined(USE_SERIAL_LOG)
    //  swSerial.begin();
    //  swSerial.print("------------");
    //  swSerial.println();
#endif

    hal.Init();
    hal.PowerOnSensors();

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

#if defined(BATTERY_VOLTAGE_X10_ID)

    // SerialPrintInfo("Raw Vcc", hal.GetRawInternal11Ref(), ANALOG);

    uint16_t vccMv = hal.ComputeVccMv(INTERNAL_1v1);
    //SerialPrintInfo("1v1", hal.GetRawInternal11Ref(), ANALOG);

    uint16_t batteryVoltageInMv = 0;

#if defined(BATTERY_IS_VCC)
    batteryVoltageInMv = vccMv;
#else
    uint16_t rawBattery = hal.GetRawBattery();
    //SerialPrintInfo("Raw batt", rawBattery, ANALOG);

    batteryVoltageInMv = hal.ConvertAnalogValueToMv(rawBattery, vccMv);
#endif

    SerialPrintInfo("Vbatt", batteryVoltageInMv, MILLIVOLT);

    hal.LedOn();
    x10encoder.RFXmeter(BATTERY_VOLTAGE_X10_ID, 0x00,
                        ConversionTools::dec16ToHex(batteryVoltageInMv));
    hal.LedOff();
    hal.Delay30ms();
    hal.Delay30ms();
#endif

#if defined(ANALOG1_X10_ID)

    uint16_t rawSolar = hal.GetRawAnalogSensor();
    SerialPrintInfo("Raw solar", rawSolar, ANALOG);

    uint16_t solar_power = hal.ConvertAnalogValueToMv(rawSolar, vccMv);
    SerialPrintInfo("Vsolar", solar_power, MILLIVOLT);

    hal.LedOn();
    x10encoder.RFXmeter(ANALOG1_X10_ID, 0x00,
                        ConversionTools::dec16ToHex(solar_power));
    hal.LedOff();
    hal.Delay30ms();
#endif

#if defined(USE_BH1750)
    LacrosseWS7000 lightEncoder(&hal);
    lightEncoder.SetAddress(LACROSSE_ID);
    BH1750 lightMeter(0x23);
    lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
    _delay_ms(150); // around 120ms, be generous
    uint16_t lux = (uint16_t)lightMeter.readLightLevel();

    SerialPrintInfo("Luminosity", lux, LUX);

    lightEncoder.SetLuminosity(lux);

    hal.LedOn();
    lightEncoder.Send();
    hal.LedOff();
    hal.Delay30ms();

    hal.LedOn();
    lightEncoder.Send();
    hal.LedOff();
    hal.Delay30ms();
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
    SerialPrintInfo("Temp.", temperature * 10, CELCIUS);
    SerialPrintInfo("RH ", humidity, PERCENT);
    SerialPrintInfo("Pressure ", pressure, HECTOPASCAL);
#endif

    // at least temperature should be set
    // to use this encoder
    if (temperature != NOT_SET) {

#if defined(USE_OREGON)
      if (batteryVoltageInMv > LOW_BATTERY_VOLTAGE) {
        environmentEncoder.SetBatteryLow(false);
      }
#endif
      hal.LedOn();
      environmentEncoder.Send();
      hal.LedOff();
      hal.Delay30ms();

      hal.LedOn();
      environmentEncoder.Send();
      hal.LedOff();
    }
#endif

#if defined(USE_SERIAL_LOG)
    // SerialPrintInfo("Going to sleep", (uint16_t)SLEEP_TIME_IN_SECONDS,
    // SECOND);
#endif

    hal.PowerOffSensors();
    hal.Hibernate((uint16_t)SLEEP_TIME_IN_SECONDS);

#if defined(USE_SERIAL_LOG)
    //  swSerial.begin();
    //  swSerial.print("Wakeup!");
    //  swSerial.println();
#endif
  }
}
