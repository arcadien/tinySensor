#include <Arduino.h>

#include <Receiver.h>

// Pro Micro receiver entry point (CONF-DECODE-003): the Arduino main runs
// setup() then loop() and keeps the USB stack running.
ProMicroReceiverHal hal;
Receiver receiver(&hal);

void setup() {
  hal.Init();
}

void loop() {
  receiver.Poll();
}
