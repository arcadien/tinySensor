#include <Arduino.h>

#include <Receiver.h>

// Receiver entry point for the Pro Mini and the Pro Micro (CONF-DECODE-003,
// CONF-DECODE-004): the Arduino main runs setup() then loop().
AtmegaReceiverHal hal;
Receiver receiver(&hal);

void setup() {
  hal.Init();
}

void loop() {
  receiver.Poll();
}
