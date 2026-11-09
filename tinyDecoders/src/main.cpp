#include <avr/interrupt.h>

#include <Receiver.h>

// Receiver firmware entry point (CONF-DECODE-001). File scope so the static
// RAM check counts both objects.
Attiny84aReceiverHal hal;
Receiver receiver(&hal);

int main() {
  hal.Init();
  sei();
  for (;;) receiver.Poll();
}
