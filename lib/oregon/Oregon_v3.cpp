#include <Oregon_v3.h>
#include <Hal.h>

const uint8_t OregonV3::POSTAMBLE[1] = {0x0};
const uint8_t OregonV3::PREAMBLE[3] = {0xFF, 0xFF, 0xFF};

void OregonV3::SendMSB(const uint8_t data)
{
	(BitRead(data, 4)) ? SendOne() : SendZero();
	(BitRead(data, 5)) ? SendOne() : SendZero();
	(BitRead(data, 6)) ? SendOne() : SendZero();
	(BitRead(data, 7)) ? SendOne() : SendZero();
}

void OregonV3::SendLSB(const uint8_t data)
{
	(BitRead(data, 1)) ? SendOne() : SendZero();
	(BitRead(data, 2)) ? SendOne() : SendZero();
	(BitRead(data, 3)) ? SendOne() : SendZero();
}

void OregonV3::SendData(const uint8_t *data, uint8_t size)
{
	for (uint8_t i = 0; i < size; ++i)
	{
		SendLSB(data[i]);
		SendMSB(data[i]);
	}
}

void OregonV3::DelayHalfPeriod()
{
	OREGON_DELAY_US(HALF_DELAY_US)
}

void OregonV3::DelayPeriod()
{
	OREGON_DELAY_US(DELAY_US)
}

void OregonV3::SendZero()
{
	_hal->RadioGoHigh();
	DelayHalfPeriod();
	_hal->RadioGoLow();
	DelayPeriod();
	_hal->RadioGoHigh();
	DelayHalfPeriod();
}

void OregonV3::SendOne()
{
	_hal->RadioGoLow();
	DelayHalfPeriod();
	_hal->RadioGoHigh();
	DelayPeriod();
	_hal->RadioGoLow();
	DelayHalfPeriod();
}

void OregonV3::SetHumidity(int humidity)
{
	messageStatus |= 1 << 1;

	// humidity nibbles are spread on bytes 6 and 7
	// bit 6 also contains the negative temp flag (lsb)
	message[7] = (humidity / 10);
	message[6] |= (humidity - message[7] * 10) << 4;
}

void OregonV3::SetTemperature(float temperature)
{

	messageStatus |= 1;
	if (temperature < 0)
	{
		message[6] = 0x08;
		temperature *= -1;
	}
	else
	{
		message[6] = 0x00;
	}

	// Determine decimal and float part

	// ex. for 27.4 degrees C
	int tempIntegerPart = (int)temperature; // 27

	// (int)(27/10) = 2
	int temperatureDozen = (int)(tempIntegerPart / 10);

	// 2.7 - 20 = (int)7.4=7
	int temperatureUnit = (int)round(
		(float)((float)tempIntegerPart / 10.0 - (float)temperatureDozen) *
		10.0);

	int temperatureDecimal = (int)round(
		(float)(temperature - (float)tempIntegerPart) // 27.4 - 27 = 0.4
		* 10										  // 0.4 * 10 = 4
	);

	message[4] |= (temperatureDecimal << 4);
	message[5] = (temperatureDozen << 4);
	message[5] |= temperatureUnit;
}
void OregonV3::Send()
{
	uint8_t messageLength = 8;

	if ((messageStatus & 0b00000001) == 0b00000001)
	{
		messageLength++;
	}
	if ((messageStatus & 0b00000010) == 0b00000010)
	{
		messageLength += 2;
	}

	if (messageLength == 8)
	{
		// THGN122N (Temperature)
		message[0] = 0x0A;
		message[1] = 0x4D;
		int s = ((Sum(6, message) + (message[6] & 0xF) - 0xa) & 0xff);
		message[6] |= (s & 0x0F) << 4;
		message[7] = (s & 0xF0) >> 4;
	}

	if (messageLength == 9)
	{
		// THGN122N (Temperature + Humidity)
		message[0] = 0x1A;
		message[1] = 0x2D;
		message[8] = ((Sum(8, message) - 0xa) & 0xFF);
	}

	if (messageLength == 11)
	{
		// BTHR918N (temp, humidity, pressure)
		message[0] = 0x5A;
		message[1] = 0x6D;
		message[10] = ((Sum(10, message) - 0xa) & 0xFF);
	}

	// The specification document says that the SYNC must be sent.
	// With the RFLINK decoder, which is the reference for this library,
	// the SYNC is not needed.

	SendData(PREAMBLE, 3);
	SendData(message, MESSAGE_SIZE_IN_BYTES);
	SendData(POSTAMBLE, 1);
}
