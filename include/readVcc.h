#include <stddef.h>
#include <util/delay.h>
#include <avr/io.h>

/*!
 *
 * Reads Vcc using internal 1.1v tension reference
 *
 */
uint16_t readVcc(void);

/*!
 *
 * Reads voltage on ADC1 pin, relative to Vcc
 *
 */
uint16_t readBatteryVoltage(void);