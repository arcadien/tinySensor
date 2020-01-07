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

/*
 *
 * TinySensor interface
 *
 */

#include <stdint.h>

/*
 * Initial board setup
 *
 * This is where pins and various registers are set
 * for maximized power saving.
 */
void setup();

/*!
 * Entry point and place for main loop
 *
 */
int avr_main(void);

/*!
 * sleep for specified number of seconds
 *
 */
void sleep(uint8_t s);

/*!
 * Perform sensing operation
 */
void sense();

/*!
 * Transfer sensor information
 */
void emit();
