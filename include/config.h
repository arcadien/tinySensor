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

#if !defined(SLEEP_TIME_IN_SECONDS)
#define SLEEP_TIME_IN_SECONDS 32
#endif

#if !defined(LOW_BATTERY_VOLTAGE)
#define LOW_BATTERY_VOLTAGE 2000
#endif

#if defined(USE_BMP280) or defined(USE_BME280)
#define USE_I2C
#endif
