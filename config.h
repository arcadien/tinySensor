/*
 * config.h
 *
 * Created: 19/11/2018 22:14:40
 *  Author: aurelien
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <inttypes.h>

// sleep 32s between each sample and
// emission
#define SLEEP_TIME_IN_SECONDS (8 * 4)

/*
 * 1-wire interface is on PA3
 * Power for radio is on PA2 (sensor_vcc)
 * LED pin is PB3
 * PWM pin is PA7
 * SDA pin is PA6
 * SCL pin is PA4
 * MISO pin is PA5
 * TX radio pin is PB0
 * BAT sensor pin is PA1
 * analog sensor pin is PA0
 */
#define LED_PIN PB3
#define PWM_PIN PA7
#define TX_RADIO_PIN PB0
#define RADIO_POWER_PIN PA2
#define BAT_SENSOR_PIN PA1

// Sensors
//#define USE_BMP280
//#define USE_BME280
#define USE_DS18B20 1
// Protocols
#define USE_OREGON 1

// 0x5A, 0x5D pressure, temp, hygro
// 0xEA, 0x4C temp only
const uint8_t OREGON_TYPE[] = {0xEA, 0x4C};
const uint8_t OREGON_ID = 0xCA;

/*
 * MODE_0 0 // Temperature only [THN132N]
 * MODE_1 1 // Temperature + Humidity [THGR2228N]
 * MODE_2 2 // Temperature + Humidity + Baro() [BTHR918N]
 */
#define OREGON_MODE MODE_0

#endif /* CONFIG_H_ */
