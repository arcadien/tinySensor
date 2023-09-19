#include "SoftwareSerial.h"

#define __DELAY_ROUND_CLOSEST__
#include <util/delay.h>

#define BIT_PERIOD ((1000000L / 9600) - 25)

SoftwareSerial::SoftwareSerial(Hal *hal) : _hal(hal) {}

/******************************************************************************
 * User API
 ******************************************************************************/

void SoftwareSerial::begin() {
  _hal->SerialGoHigh();
  _delay_us(BIT_PERIOD);
}

void SoftwareSerial::print(uint8_t b) {

  uint8_t mask;

  _hal->SerialGoLow();
  _delay_us(BIT_PERIOD);

  for (mask = 0x01; mask; mask <<= 1) {
    if (b & mask) {         // choose bit
      _hal->SerialGoHigh(); // send 1
    } else {
      _hal->SerialGoLow(); // send 0
    }
    _delay_us(BIT_PERIOD);
  }

  _hal->SerialGoHigh();
  _delay_us(BIT_PERIOD);
}

void SoftwareSerial::print(const char *s) {
  while (*s)
    print(*s++);
}

void SoftwareSerial::print(char c) { print((uint8_t)c); }

void SoftwareSerial::print(int n) { print((long)n); }

void SoftwareSerial::print(unsigned int n) { print((unsigned long)n); }

void SoftwareSerial::print(long n) {
  if (n < 0) {
    print('-');
    n = -n;
  }
  printNumber(n, 10);
}

void SoftwareSerial::print(unsigned long n) { printNumber(n, 10); }

void SoftwareSerial::print(long n, int base) {
  if (base == 0)
    print((char)n);
  else if (base == 10)
    print(n);
  else
    printNumber(n, base);
}

void SoftwareSerial::println(void) {
  print('\r');
  print('\n');
}

void SoftwareSerial::println(char c) {
  print(c);
  println();
}

void SoftwareSerial::println(const char c[]) {
  print(c);
  println();
}

void SoftwareSerial::println(uint8_t b) {
  print(b);
  println();
}

void SoftwareSerial::println(int n) {
  print(n);
  println();
}

void SoftwareSerial::println(long n) {
  print(n);
  println();
}

void SoftwareSerial::println(unsigned long n) {
  print(n);
  println();
}

void SoftwareSerial::println(long n, int base) {
  print(n, base);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void SoftwareSerial::printNumber(unsigned long n, uint8_t base) {
  unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
  unsigned long i = 0;

  if (n == 0) {
    print('0');
    return;
  }

  while (n > 0) {
    buf[i++] = n % base;
    n /= base;
  }

  for (; i > 0; i--)
    print((char)(buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
}