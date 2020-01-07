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

#if defined(__AVR_ATtiny84A__)
#define USI_DDR_PORT DDRA
#define USI_SCK_PORT DDRA
#define USCK_DD_PIN DDA4
#define DO_DD_PIN DDA5
#define DI_DD_PIN DDA6
#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define PORT_USI_SDA PORTA6
#define PORT_USI_SCL PORTA4
#define PIN_USI_SDA PINA6
#define PIN_USI_SCL PINA4
#define USI_START_VECTOR USI_START_vect
#define USI_OVERFLOW_VECTOR USI_OVF_vect
#define DDR_USI_CL DDR_USI
#define PORT_USI_CL PORT_USI
#define PIN_USI_CL PIN_USI
#ifndef USI_START_COND_INT
#define USI_START_COND_INT USISIF
#endif
#endif
