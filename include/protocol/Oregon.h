/*
* connectingStuff, Oregon Scientific v2.1 Emitter
* http://connectingstuff.net/blog/encodage-protocoles-oregon-scientific-sur-arduino/
*
* Copyright (C) 2013 olivier.lebrun@gmail.com
*
* Refactored to class by Aurélien Labrosse <https://github.com/arcadien>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
* USA.
*/
#ifndef OREGON_H_
#define OREGON_H_

/*
--------------------------------------------------------
|  Sensor Name      |  Code   | Type                   |
--------------------------------------------------------
|  Oregon-THR128    |         |                        |
|  Oregon-THR138    | 0x0A4D  | Inside Temperature     |
|  Oregon-THC138    |         |                        |
--------------------------------------------------------
| Oregon-THC238     |         |                        |
| Oregon-THC268     |         |                        |
| Oregon-THN132N    |         |                        |
| Oregon-THWR288A   | 0xEA4C  | Outside/Water Temp     |
| Oregon-THRN122N   |         |                        |
| Oregon-THN122N    |         |                        |
| Oregon-AW129      |         |                        |
| Oregon-AW131      |         |                        |
--------------------------------------------------------
| Oregon-THWR800    | 0xCA48  | Water Temp             |
--------------------------------------------------------
| Oregon-THGN122N   |         |                        |
| Oregon-THGN123N   |         |                        |
| Oregon-THGR122NX  | 0x1A2D  | Inside Temp-Hygro      |
| Oregon-THGR228N   |         |                        |
| Oregon-THGR238    |         |                        |
| Oregon-THGR268    |         |                        |
--------------------------------------------------------
| Oregon-THGR810    | 0xFA28  | Inside Temp-Hygro      |
| Oregon-RTGR328N   | 0x*ACC  | Outside Temp-Hygro     |
| Oregon-THGR328N   | 0xCA2C  | Outside Temp-Hygro     |
| Oregon-WTGR800    | 0xFAB8  | Outside Temp-Hygro     |
--------------------------------------------------------
| Oregon-THGR918    |         |                        |
| Oregon-THGRN228NX | 0x1A3D  | Outside Temp-Hygro     |
| Oregon-THGN500    |         |                        |
--------------------------------------------------------
| Huger - BTHR918   | 0x5A5D  | Inside Temp-Hygro-Baro |
--------------------------------------------------------
| Oregon-BTHR918N   |         |                        |
| Oregon-BTHR968    | 0x5A6D  | Inside Temp-Hygro-Baro |
--------------------------------------------------------
| Oregon-RGR126     |         |                        |
| Oregon-RGR682     | 0x2A1D  | Rain Gauge             |
| Oregon-RGR918     |         |                        |
--------------------------------------------------------
| Oregon-PCR800     | 0x2A19  | Rain Gauge             |
| Oregon-WTGR800    | 0x1A99  | Anemometer             |
| Oregon-WGR800     | 0x1A89  | Anemometer             |
--------------------------------------------------------
| Huger-STR918      |         |                        |
| Oregon-WGR918     | 0x3A0D  | Anemometer             |
--------------------------------------------------------
| Oregon-UVN128     |         |                        |
| Oregon-UV138      | 0xEA7C  | UV sensor              |
--------------------------------------------------------
| Oregon-UVN800     | 0xDA78  | UV sensor              |
| Oregon-RTGR328N   | 0x*AEC  | Date & RF_BIT_TIME     |
--------------------------------------------------------
| cent-a-meter      |         |                        |
| OWL CM113         | 0xEAC0  | Ampere meter           |
| Electrisave       |         |                        |
--------------------------------------------------------
| OWL CM119         | 0x1A*                            |
|                   | 0x2A*   | Power meter            |
|                   | 0x3A**  |                        |
--------------------------------------------------------
*/

#include "config.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define HIGH 0x1
#define LOW 0x0

class Oregon
{
	public:
	
	#if OREGON_MODE == MODE_0
	uint8_t _oregonMessageBuffer[8];
	#elif OREGON_MODE == MODE_1
	uint8_t _oregonMessageBuffer[9];
	#else
	uint8_t _oregonMessageBuffer[11];
	#endif

	

	/*!
	* Convenient define for channel codes (1,2,3)
	*/
	struct Channel
	{
		static const uint8_t ONE = 0x10;
		static const uint8_t TWO = 0x20;
		static const uint8_t THREE = 0x30;
	};

	#define MODE_0 0 // Temperature only [THN132N]
	#define MODE_1 1 // Temperature + Humidity [THGR2228N]
	#define MODE_2 2 // Temperature + Humidity + Baro() [BTHR918N]

	private:
	static uint8_t PREAMBLE[];
	static uint8_t POSTAMBLE[];

	public:
	static const uint16_t TIME = 512;
	static const uint16_t TWOTIME = TIME * 2;

	void txPinLow();
	void txPinHigh();

	/**
	* \brief    Send logical "0" over RF
	* \details  a zero bit be represented by an off-to-on transition
	* \         of the RF signal at the middle of a clock period.
	* \         Remember, the Oregon v2.1 protocol add an inverted bit first
	*/
	inline void sendZero(void);

	/**
	* \brief    Send logical "1" over RF
	* \details  a one bit be represented by an on-to-off transition
	* \         of the RF signal at the middle of a clock period.
	* \         Remember, the Oregon v2.1 protocol add an inverted bit first
	*/
	inline void sendOne(void);

	/**
	* Send a bits quarter (4 bits = MSB from 8 bits value) over RF
	*
	* @param data Source data to process and sent
	*/

	/**
	* \brief    Send a bits quarter (4 bits = MSB from 8 bits value) over RF
	* \param    data   Data to send
	*/
	inline void sendQuarterMSB(const uint8_t data);

	/**
	* \brief    Send a bits quarter (4 bits = LSB from 8 bits value) over RF
	* \param    data   Data to send
	*/
	inline void sendQuarterLSB(const uint8_t data);

	/**
	* \brief    Send a buffer over RF
	* \param    data   Data to send
	* \param    size   size of data to send
	*/
	inline void sendData(uint8_t *data, uint8_t size);

	/**
	* \brief    Send an Oregon message
	* \param    data   The Oregon message
	*/
	void sendOregon(uint8_t *data, uint8_t size);

	/**
	* \brief    Send preamble
	* \details  The preamble consists of 16 "1" bits
	*/
	void sendPreamble(void);

	/**
	* \brief    Send postamble
	* \details  The postamble consists of 8 "0" bits
	*/
	inline void sendPostamble(void);

	/**
	* \brief    Send sync nibble
	* \details  The sync is 0xA. It is not use in this version since the sync
	* nibble \         is include in the Oregon message to send.
	*/
	inline void sendSync(void);

	/**
	* \brief    Set the sensor type
	* \param    data       Oregon message
	* \param    type       Sensor type
	*/
	void setType(uint8_t *data, const uint8_t *type);

	/**
	* \brief    Set the sensor channel
	* \param    data       Oregon message
	* \param    channel    Sensor channel (0x10, 0x20, 0x30)
	*/
	void setChannel(uint8_t *data, uint8_t channel);

	/**
	* \brief    Set the sensor ID
	* \param    data       Oregon message
	* \param    ID         Sensor unique ID
	*/
	void setId(uint8_t *data, uint8_t ID);

	/**
	* \brief    Set the sensor battery level
	* \param    data       Oregon message
	* \param    level      Battery level (0 = low, 1 = high)
	*/
	void setBatteryLevel(uint8_t *data, uint8_t level);

	/**
	* \brief    Set the sensor temperature
	* \param    data       Oregon message
	* \param    temp       the temperature
	*/
	void setTemperature(uint8_t *data, float temp);

	/**
	* \brief    Set the sensor temperature
	* \param    data       Oregon message
	* \param    temp       the temperature
	*/
	void setPressure(uint8_t *data, float pres);
	/**
	* \brief    Set the sensor humidity
	* \param    data       Oregon message
	* \param    hum        the humidity
	*/
	void setHumidity(uint8_t *data, uint8_t hum);

	/**
	* \brief    Sum data for checksum
	* \param    count      number of bit to sum
	* \param    data       Oregon message
	*/
	inline int Sum(uint8_t count, const uint8_t *data);

	/**
	* \brief    Calculate checksum
	* \param    data       Oregon message
	*/
	void calculateAndSetChecksum(uint8_t *data);
};

#endif /* OREGON_H_ */
