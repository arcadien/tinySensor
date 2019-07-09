#pragma once

/*
 *
 * TinySensor interface
 *
 */

#include <stdint.h>

/*
 * Initial board setup
 *
 * This is where pins and various registers are set
 * for maximized power saving.
 */
void setup();

/*!
 * Entry point and place for main loop
 *
 */
int avr_main(void);

/*!
 * sleep for specified number of seconds
 *
 */
void sleep(uint8_t s);

/*!
 * Perform sensing operation
 */
void sense();

/*!
 * Transfer sensor information
 */
void emit();
