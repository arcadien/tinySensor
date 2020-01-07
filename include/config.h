/*
 * This file is part of the TinySensor distribution (https://github.com/arcadien/TinySensor)
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

#pragma once

#include <inttypes.h>

// sleep 32s between each sample and
// emission
#define SLEEP_TIME_IN_SECONDS (8 * 4)

/*
 * 1-wire interface is on PA3
 * Power for radio is on PA2 (sensor_vcc)
 * LED pin is PB1
 * PWM pin is PA7
 * SDA pin is PA6
 * SCL pin is PA4
 * MISO pin is PA5
 * TX radio pin is PB0
 * BAT sensor pin is PA1
 * analog sensor pin is PA0
 */
#define LED_PIN PB1
#define PWM_PIN PA7
#define TX_RADIO_PIN PB0
#define RADIO_POWER_PIN PA2
#define BAT_SENSOR_PIN PA1

// Sensors
#define USE_BMP280
//#define USE_BME280
//#define USE_DS18B20 
// Protocols
#define USE_OREGON 1

// 0x5A, 0x5D pressure, temp, hygro
// 0xEA, 0x4C temp only
const uint8_t OREGON_TYPE[] = {0xEA, 0x4C};
const uint8_t OREGON_ID = 0xCB;

/*
 * MODE_0 0 // Temperature only [THN132N]
 * MODE_1 1 // Temperature + Humidity [THGR2228N]
 * MODE_2 2 // Temperature + Humidity + Baro() [BTHR918N]
 */
#define OREGON_MODE MODE_0
