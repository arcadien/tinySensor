#include <LacrosseWS7000.h>

#define TEMPERATURE_AVAILABLE 0x01
#define HUMIDITY_AVAILABLE 0x02
#define PRESSURE_AVAILABLE 0x04
#define LUMINOSITY_AVAILABLE 0x08
#define BITREAD(value, bit) (((value) >> (bit)) & 0x01)
#define UPDATE_CHECKSUM(nibble) do { checkXor ^= (nibble); checkSum += (nibble); } while(0)

LacrosseWS7000::LacrosseWS7000(Hal *hal) : _hal(hal), availableData(0) {}

void LacrosseWS7000::SetAddress(LacrosseWS7000::Address address) {
  this->address = address;
}

void LacrosseWS7000::SetHumidity(float humidity) {
  availableData |= HUMIDITY_AVAILABLE;
  if (humidity >= 100) humidity = 99.9;
  NumericalValueSplitter::Split(humidity, &this->humidity);
}

void LacrosseWS7000::SetPressure(float pressure) {
  availableData |= PRESSURE_AVAILABLE;
  if (pressure < 200) pressure = 200;
  if (pressure < 850) pressure = 850;
  if (pressure > 1100) pressure = 1100;
  pressure -= 200;
  NumericalValueSplitter::Split(pressure, &this->pressure);
}

void LacrosseWS7000::SetLuminosity(uint16_t luminosity) {
  availableData |= LUMINOSITY_AVAILABLE;
  NumericalValueSplitter::Split(luminosity, &this->luminosity);
}

void LacrosseWS7000::SetTemperature(float temperature) {
  availableData |= TEMPERATURE_AVAILABLE;
  if (temperature > 99) temperature = 99;
  if (temperature < -99) temperature = -99;
  NumericalValueSplitter::Split(temperature, &this->temperature);
}

void LacrosseWS7000::SendOne() {
  _hal->RadioGoHigh();
  _hal->Delay400Us();
  _hal->RadioGoLow();
  _hal->Delay400Us();
  _hal->Delay400Us();
}

void LacrosseWS7000::SendZero() {
  _hal->RadioGoHigh();
  _hal->Delay400Us();
  _hal->Delay400Us();
  _hal->RadioGoLow();
  _hal->Delay400Us();
}

void LacrosseWS7000::SendNibble(uint8_t nibble) {
  BITREAD(nibble, 0) ? SendOne() : SendZero();
  BITREAD(nibble, 1) ? SendOne() : SendZero();
  BITREAD(nibble, 2) ? SendOne() : SendZero();
  BITREAD(nibble, 3) ? SendOne() : SendZero();
  SendOne();
}

void LacrosseWS7000::SendPreamble() {
  for (uint8_t counter = 10; counter > 0; counter--) {
    SendZero();
  }
}

void LacrosseWS7000::SendType(uint8_t *nibbles) {
  uint8_t nibble = 0x0F;
  if (1 == availableData) nibble = 0x00;
  else if (3 == availableData) nibble = 0x01;
  else if (7 == availableData) nibble = 0x04;
  else if (8 == availableData) nibble = 0x05;
  SendNibble(nibble);
  nibbles[0] = nibble;
}

void LacrosseWS7000::SendAddressAndTemperatureSign(uint8_t *nibbles) {
  uint8_t addressAndTempSign = address;
  if ((availableData & TEMPERATURE_AVAILABLE) && temperature.isNegative) {
    addressAndTempSign |= (1 << 3);
  }
  SendNibble(addressAndTempSign);
  nibbles[0] = addressAndTempSign;
}

void LacrosseWS7000::SendTemperature(uint8_t *nibbles) {
  SendNibble(temperature.decimals);
  SendNibble(temperature.units);
  SendNibble(temperature.dozens);
  nibbles[0] = temperature.decimals;
  nibbles[1] = temperature.units;
  nibbles[2] = temperature.dozens;
}

void LacrosseWS7000::SendLuminosity(uint8_t *nibbles) {
  SendNibble(luminosity.units);
  SendNibble(luminosity.dozens);
  SendNibble(luminosity.hundreds);
  SendNibble(0x00);
  SendNibble(0x00);
  SendNibble(0x00);
  SendNibble(0x00);
  
  nibbles[0] = luminosity.units;
  nibbles[1] = luminosity.dozens;
  nibbles[2] = luminosity.hundreds;
  nibbles[3] = 0x00;
  nibbles[4] = 0x00;
  nibbles[5] = 0x00;
  nibbles[6] = 0x00;
}

void LacrosseWS7000::SendHumidity(uint8_t *nibbles) {
  SendNibble(humidity.decimals);
  SendNibble(humidity.units);
  SendNibble(humidity.dozens);
  nibbles[0] = humidity.decimals;
  nibbles[1] = humidity.units;
  nibbles[2] = humidity.dozens;
}

void LacrosseWS7000::SendPressure(uint8_t *nibbles) {
  SendNibble(pressure.units);
  SendNibble(pressure.dozens);
  SendNibble(pressure.hundreds);
  SendNibble(pressure.decimals);
  nibbles[0] = pressure.units;
  nibbles[1] = pressure.dozens;
  nibbles[2] = pressure.hundreds;
  nibbles[3] = pressure.decimals;
}

static void UpdateChecks(uint8_t &checkXor, uint8_t &checkSum, const uint8_t *nibbles, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    UPDATE_CHECKSUM(nibbles[i]);
  }
}

void LacrosseWS7000::Send() {
  uint8_t checkXor = 0;
  uint8_t checkSum = 5;
  uint8_t temporaryNibbles[8];

  SendPreamble();
  SendOne();

  SendType(temporaryNibbles);
  UPDATE_CHECKSUM(temporaryNibbles[0]);

  SendAddressAndTemperatureSign(temporaryNibbles);
  UPDATE_CHECKSUM(temporaryNibbles[0]);

  if (availableData & LUMINOSITY_AVAILABLE) {
    SendLuminosity(temporaryNibbles);
    UpdateChecks(checkXor, checkSum, temporaryNibbles, 7);
  } else {
    if (availableData & TEMPERATURE_AVAILABLE) {
      SendTemperature(temporaryNibbles);
      UpdateChecks(checkXor, checkSum, temporaryNibbles, 3);
    }
    if (availableData & HUMIDITY_AVAILABLE) {
      SendHumidity(temporaryNibbles);
      UpdateChecks(checkXor, checkSum, temporaryNibbles, 3);
    }
    if (availableData & PRESSURE_AVAILABLE) {
      SendPressure(temporaryNibbles);
      UpdateChecks(checkXor, checkSum, temporaryNibbles, 4);
    }
  }

  checkXor &= 0x0F;
  checkSum += checkXor;
  checkSum &= 0x0F;

  SendNibble(checkXor);
  SendNibble(checkSum);
}
