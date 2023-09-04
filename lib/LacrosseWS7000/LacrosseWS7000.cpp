#include <LacrosseWS7000.h>

static const uint8_t TEMPERATURE_AVAILABLE = 1;
static const uint8_t HUMIDITY_AVAILABLE = (1 << 1);
static const uint8_t PRESSURE_AVAILABLE = (1 << 2);
static const uint8_t LUMINOSITY_AVAILABLE = (1 << 3);

static bool BitRead(uint8_t value, uint8_t bit) {
  return (((value) >> (bit)) & 0x01);
}

LacrosseWS7000::LacrosseWS7000(Hal *hal) : _hal(hal), availableData(0) {}

void LacrosseWS7000::SetAddress(LacrosseWS7000::Address address) {
  this->address = address;
}
void LacrosseWS7000::SetHumidity(uint8_t humidity) {
  availableData |= HUMIDITY_AVAILABLE;
  if (humidity > 100) {
    humidity = 100;
  }
  this->humidity.Set(humidity);
}
void LacrosseWS7000::SetPressure(float pressure) {
  availableData |= PRESSURE_AVAILABLE;

  // pressure should be between 870 and 1085 (recorded extreme values)
  // we need to substract 200 to avoid "10" hundreds in further encoding.
  // decoders shall be aware of that.
  if(pressure < 850) pressure = 850;
  if(pressure > 1100) pressure = 1100;
  pressure -= 200;
  this->pressure.Set(pressure);
}
void LacrosseWS7000::SetLight(uint16_t luminosity) {
  availableData |= LUMINOSITY_AVAILABLE;
  this->luminosity = luminosity;
}
void LacrosseWS7000::SetTemperature(float temperature) {
  availableData |= TEMPERATURE_AVAILABLE;
  this->temperature.Set(temperature);
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
  (BitRead(nibble, 0)) ? SendOne() : SendZero();
  (BitRead(nibble, 1)) ? SendOne() : SendZero();
  (BitRead(nibble, 2)) ? SendOne() : SendZero();
  (BitRead(nibble, 3)) ? SendOne() : SendZero();
  SendOne();
}

void LacrosseWS7000::SendPreamble() {
  for (uint8_t counter = 10; counter > 0; counter--) {
    SendZero();
  }
}

uint8_t LacrosseWS7000::SendType() {
  uint8_t nibble = 0x0F;
  if (1 == availableData) { // temp
    nibble = 0x00;

  } else if (3 == availableData) { // temp + humi
    nibble = 0b00000001;

  } else if (7 == availableData) { // temp + humi + baro
    nibble = 0b00000100;

  } else if (8 == availableData) {
    nibble = 0b00000101;

  } else {
    nibble = 0x0F;
  }
  SendNibble(nibble);
  return nibble;
}

uint8_t LacrosseWS7000::SendAddressAndTemperatureSign() {
  uint8_t addressAndTempSign = address;
  if (temperature.isNegative) {
    addressAndTempSign |= (1 << 3);
  }
  SendNibble(addressAndTempSign);
  return addressAndTempSign;
}
void LacrosseWS7000::SendTemperature(uint8_t *nibbles) {

  SendNibble(temperature.decimals);
  SendNibble(temperature.units);
  SendNibble(temperature.dozens);

  nibbles[0] = (temperature.decimals & 0xF);
  nibbles[1] = (temperature.units & 0xF);
  nibbles[2] = (temperature.dozens & 0xF);
}

void LacrosseWS7000::SendLuminosity(uint8_t *nibbles) {}

void LacrosseWS7000::SendHumidity(uint8_t *nibbles) {
  SendNibble(0x00);
  SendNibble(humidity.units);
  SendNibble(humidity.dozens);

  nibbles[0] = 0x00;
  nibbles[1] = (humidity.units & 0xF);
  nibbles[2] = (humidity.dozens & 0xF);
}
void LacrosseWS7000::SendPressure(uint8_t *nibbles) {

  SendNibble(pressure.units);
  SendNibble(pressure.dozens);
  SendNibble(pressure.hundreds);
  SendNibble(pressure.decimals);

  nibbles[0] = (pressure.units & 0xF);
  nibbles[1] = (pressure.dozens & 0xF);
  nibbles[2] = (pressure.hundreds & 0xF);
  nibbles[3] = (pressure.decimals & 0xF);
}
void LacrosseWS7000::Send() {

  uint8_t checkXor = 0;
  uint8_t checkSum = 5;
  uint8_t temporaryNibbles[7];

  SendPreamble(); // 10 bits
  SendOne();      // dead bit

  uint8_t typeNibble = SendType();
  checkXor ^= typeNibble;
  checkSum += typeNibble;

  uint8_t addressAndSignNibble = SendAddressAndTemperatureSign();
  checkXor ^= addressAndSignNibble;
  checkSum += addressAndSignNibble;

  if (LUMINOSITY_AVAILABLE == (availableData & LUMINOSITY_AVAILABLE)) {
    SendLuminosity(temporaryNibbles);

    checkXor ^= temporaryNibbles[0];
    checkSum += temporaryNibbles[0];

    checkXor ^= temporaryNibbles[1];
    checkSum += temporaryNibbles[1];

    checkXor ^= temporaryNibbles[2];
    checkSum += temporaryNibbles[2];

    checkXor ^= temporaryNibbles[3];
    checkSum += temporaryNibbles[3];

    checkXor ^= temporaryNibbles[4];
    checkSum += temporaryNibbles[4];

    checkXor ^= temporaryNibbles[5];
    checkSum += temporaryNibbles[5];

    checkXor ^= temporaryNibbles[6];
    checkSum += temporaryNibbles[6];

    checkXor ^= temporaryNibbles[7];
    checkSum += temporaryNibbles[7];
  } else {

    if (TEMPERATURE_AVAILABLE == (availableData & TEMPERATURE_AVAILABLE)) {

      SendTemperature(temporaryNibbles);

      checkXor ^= temporaryNibbles[0];
      checkSum += temporaryNibbles[0];

      checkXor ^= temporaryNibbles[1];
      checkSum += temporaryNibbles[1];

      checkXor ^= temporaryNibbles[2];
      checkSum += temporaryNibbles[2];
    }

    if (HUMIDITY_AVAILABLE == (availableData & HUMIDITY_AVAILABLE)) {
      SendHumidity(temporaryNibbles);

      checkXor ^= temporaryNibbles[0];
      checkSum += temporaryNibbles[0];

      checkXor ^= temporaryNibbles[1];
      checkSum += temporaryNibbles[1];

      checkXor ^= temporaryNibbles[2];
      checkSum += temporaryNibbles[2];
    }

    if (PRESSURE_AVAILABLE == (availableData & PRESSURE_AVAILABLE)) {

      SendPressure(temporaryNibbles);

      checkXor ^= temporaryNibbles[0];
      checkSum += temporaryNibbles[0];

      checkXor ^= temporaryNibbles[1];
      checkSum += temporaryNibbles[1];

      checkXor ^= temporaryNibbles[2];
      checkSum += temporaryNibbles[2];

      checkXor ^= temporaryNibbles[3];
      checkSum += temporaryNibbles[3];
    }

    checkXor &= 0x0F;

    checkSum += checkXor;
    checkSum &= 0x0F;

    SendNibble(checkXor);
    SendNibble(checkSum);
  }
}