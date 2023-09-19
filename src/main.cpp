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

#include <SoftwareSerial.h>

#if defined(USE_BH1750)
#include <BH1750.h>
#endif

#if defined(AVR)
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
BMx280 bmx280(&hal);
#endif

#if defined(BATTERY_VOLTAGE_X10_ID) or defined(ANALOG1_X10_ID)
x10rf x10encoder(&hal, 5);
#endif

int main(void) {

#if defined(USE_LACROSSE)
  LacrosseWS7000 encoder(&hal);
  encoder.SetAddress(LACROSSE_ID);
#elif defined(USE_OREGON)
  OregonV3 encoder(&hal);
  encoder.SetChannel(OREGON_CHANNEL);
  encoder.SetRollingCode(OREGON_RCODE);
#endif

  /*
   * The firmware force emission of
   * a signal at boot and every quarter of an hour
   * even if sensor data does not change,
   * just to be sure sensor is not out of
   * power or bugged (900s)
   */
  volatile uint16_t secondCounter = 901;

  SoftwareSerial Serial(&hal);

  while (1) {

    Serial.begin();
    Serial.print("------------");
    Serial.println();

    hal.Init();
    hal.PowerOnSensors();
    hal.Delay30ms();

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

    Serial.print("Raw vcc: ");
    Serial.print(hal.GetRawInternal11Ref());
    Serial.println();

    uint16_t vccMv = hal.ComputeVccMv(INTERNAL_1v1);

    Serial.print("Vcc: ");
    Serial.print(vccMv);
    Serial.print("mV");
    Serial.println();

    // if (secondCounter > 900) {
    secondCounter = 0;
    uint16_t batteryVoltageInMv = 0;

#if defined(BATTERY_IS_VCC)
    batteryVoltageInMv = vccMv;
#else
    uint16_t rawBattery = hal.GetRawBattery();

    Serial.print("Raw batt: ");
    Serial.print(rawBattery);
    Serial.println();

    batteryVoltageInMv = hal.ConvertAnalogValueToMv(rawBattery, vccMv);
#endif
    Serial.print("Vbatt: ");
    Serial.print(batteryVoltageInMv);
    Serial.print("mV");
    Serial.println();

    hal.LedOn();
    x10encoder.RFXmeter(BATTERY_VOLTAGE_X10_ID, 0x00,
                        ConversionTools::dec16ToHex(batteryVoltageInMv));
    hal.LedOff();
    hal.Delay30ms();
    hal.Delay30ms();
    //}
#endif

#if defined(ANALOG1_X10_ID)

    uint16_t rawSolar = hal.GetRawAnalogSensor();
    uint16_t solar_power = hal.ConvertAnalogValueToMv(rawSolar, vccMv);

    Serial.print("Raw solar: ");
    Serial.print(rawSolar);
    Serial.println();

    Serial.print("Vsolar: ");
    Serial.print(solar_power);
    Serial.print("mV");
    Serial.println();

    hal.LedOn();
    x10encoder.RFXmeter(ANALOG1_X10_ID, 0x00,
                        ConversionTools::dec16ToHex(solar_power));
    hal.LedOff();
    hal.Delay30ms();
    hal.Delay30ms();
#endif

#if defined(USE_BH1750)
    LacrosseWS7000 lightEncoder(&hal);
    lightEncoder.SetAddress(LACROSSE_ID);
    BH1750 lightMeter(0x23);
    lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
    _delay_ms(150); // around 120ms, be generous
    uint16_t lux = (uint16_t)lightMeter.readLightLevel();

    Serial.print("Lux: ");
    Serial.print(lux);
    Serial.println();

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
      encoder.SetTemperature(temperature);
      // x10encoder.RFXsensor(TEMP_SENSOR_ID, 't', 't', temperature);
    }
    if (humidity != NOT_SET) {
      encoder.SetHumidity(humidity);
      // x10encoder.RFXsensor(HUM_SENSOR_ID, 'a', 'h', humidity);
    }
    if (pressure != NOT_SET) {
      encoder.SetPressure(pressure);
      // x10encoder.RFXsensor(PRESSURE_SENSOR_ID, 'a', 'p', (pressure/10));
    }
    // at least temperature should be set
    // to use this encoder
    if (temperature != NOT_SET) {
      hal.LedOn();
      encoder.Send();
      hal.LedOff();
      hal.Delay30ms();

      hal.LedOn();
      encoder.Send();
      hal.LedOff();
    }
#endif

    hal.PowerOffSensors();
    hal.Hibernate((uint16_t)SLEEP_TIME_IN_SECONDS);
    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}
