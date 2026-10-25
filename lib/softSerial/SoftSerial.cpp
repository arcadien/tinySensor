#include "SoftSerial.h"

SoftSerial::SoftSerial(Hal *hal) : _hal(hal) {}

/******************************************************************************
 * User API
 ******************************************************************************/

void SoftSerial::begin() {
  _hal->SerialWriteBit(1);  // idle line
}

void SoftSerial::print(uint8_t b) {
  _hal->SerialWriteBit(0);  // start bit

  for (uint8_t mask = 0x01; mask; mask <<= 1) {  // 8 data bits, LSB first
    _hal->SerialWriteBit(b & mask);
  }

  _hal->SerialWriteBit(1);  // stop bit
}

void SoftSerial::print(const char *s) {
  while (*s) print(*s++);
}

void SoftSerial::print(char c) {
  print((uint8_t)c);
}

void SoftSerial::print(int n) {
  print((long)n);
}

void SoftSerial::print(unsigned int n) {
  print((unsigned long)n);
}

void SoftSerial::print(long n) {
  if (n < 0) {
    print('-');
    n = -n;
  }
  printNumber(n, 10);
}

void SoftSerial::print(unsigned long n) {
  printNumber(n, 10);
}

void SoftSerial::print(long n, int base) {
  if (base == 0)
    print((char)n);
  else if (base == 10)
    print(n);
  else
    printNumber(n, base);
}

void SoftSerial::println(void) {
  print('\r');
  print('\n');
}

void SoftSerial::println(char c) {
  print(c);
  println();
}

void SoftSerial::println(const char c[]) {
  print(c);
  println();
}

void SoftSerial::println(uint8_t b) {
  print(b);
  println();
}

void SoftSerial::println(int n) {
  print(n);
  println();
}

void SoftSerial::println(long n) {
  print(n);
  println();
}

void SoftSerial::println(unsigned long n) {
  print(n);
  println();
}

void SoftSerial::println(long n, int base) {
  print(n, base);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void SoftSerial::printNumber(unsigned long n, uint8_t base) {
  unsigned char buf[8 * sizeof(long)];  // Assumes 8-bit chars.
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