/*
 * TinySensor.cpp
 *
 * Created: 02/10/2018 21:23:28
 * Author : aurelien
 */

#include <TinySensor.h>
#include <config.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <string.h>

#if defined(USE_OREGON)
#include <protocol/Oregon.h>
Oregon oregon;
#endif

#if defined(USE_bmX280) or defined(USE_BMP280)
#include <sensors/bme280/SparkFunBME280.h>
BME280 bmX280;
#endif

/*
 * The firmware force emission of
 * a signal every quarter of an hour
 * even if sensor data does not change,
 * just to be sure sensor is not out of
 * power or bugged (900s)
 */
volatile uint16_t secondCounter;

/*!
 * Interrupt routine called each 8 seconds in the
 * default setup, which is the longest timeout period
 * accepted by the hardware watchdog
 */
volatile uint8_t sleep_interval;
ISR(WATCHDOG_vect) {
  wdt_reset();
  ++sleep_interval;
  // Re-enable WDT interrupt
  _WD_CONTROL_REG |= (1 << WDIE);
}

void setup() {
  wdt_disable();

  secondCounter = 0;

  //
  //// Watchdog setup - 8s sleep time
  //
  _WD_CONTROL_REG |= (1 << WDCE) | (1 << WDE);
  _WD_CONTROL_REG = (1 << WDP3) | (1 << WDP0) | (1 << WDIE);

  //
  //// unused pins are input + pullup, as stated
  //// in Atmel's doc "AVR4013: picoPower Basics"
  //

  // all pins as input to avoid power draw
  DDRA = 0;
  DDRB = 0;

  // pull-up all unused pins by default
  //
  PORTA |= 0b01111001;
  PORTB |= 0b11111100;

  PRR &= ~_BV(PRTIM1);    // no timer1
  PRR &= ~_BV(PRADC);     // no adc
  ADCSRA &= ~(1 << ADEN); // p. 146

  ACSR |= (1 << ACD); // no analog comparator (p. 129)

  // deactivate brownout detection during sleep (p.36)
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR |= (1 << BODS);
  MCUSR &= ~(1 << BODSE);

  // don't use ADC buffers (p. 131)
  DIDR0 |= (1 << ADC2D) | (1 << ADC1D);

  // set output pins
  DDRB |= _BV(TX_RADIO_PIN);
  DDRA |= _BV(RADIO_POWER_PIN);
  DDRB |= _BV(LED_PIN);

  TinyI2C.init();
}

int avr_main(void) {

  setup();

#if defined(USE_OREGON)
  oregon.setType(oregon._oregonMessageBuffer, OREGON_TYPE);
  oregon.setChannel(oregon._oregonMessageBuffer, Oregon::Channel::ONE);
  oregon.setId(oregon._oregonMessageBuffer, OREGON_ID);
#endif

#if defined(USE_bmX280) || defined(USE_BMP280)
  bmX280.setI2CAddress(0x76);
  bmX280.beginI2C();
  bmX280.setStandbyTime(5);
#endif

#if defined(USE_OREGON)
// Buffer for Oregon message
#if OREGON_MODE == MODE_0
  uint8_t _lastOregonMessageBuffer[8] = {};
#elif OREGON_MODE == MODE_1
  uint8_t _lastOregonMessageBuffer[9] = {};
#elif OREGON_MODE == MODE_2
  uint8_t _lastOregonMessageBuffer[11] = {};
#else
#error mode unknown
#endif
#endif

  while (1) {

#if defined(USE_OREGON) && (defined(USE_bmX280) || defined(USE_BMP280))
    oregon.setBatteryLevel(oregon._oregonMessageBuffer, 1);
    oregon.setTemperature(oregon._oregonMessageBuffer, bmX280.readTempC());

#if defined(USE_bmX280)
    oregon.setHumidity(oregon._oregonMessageBuffer, bmX280.readFloatHumidity());
    oregon.setPressure(oregon._oregonMessageBuffer,
                       (bmX280.readFloatPressure() / 100));
#endif
    oregon.calculateAndSetChecksum(oregon._oregonMessageBuffer);
#endif

#if defined(USE_DS18B20)

    ds18b20convert(&PORTA, &DDRA, &PINA, (1 << 3), nullptr);

    // Delay (sensor needs time to perform conversion)
    _delay_ms(1000);

    // Read temperature (without ROM matching)
    int16_t temperature = 0;
    ds18b20read(&PORTA, &DDRA, &PINA, (1 << 3), nullptr, &temperature);

    oregon.setBatteryLevel(oregon._oregonMessageBuffer, 1);
    oregon.setTemperature(oregon._oregonMessageBuffer, temperature / 16);
    oregon.calculateAndSetChecksum(oregon._oregonMessageBuffer);

#endif

    bool shouldEmitData = false;
#if defined(USE_OREGON)

#if (OREGON_MODE == MODE_0)
    // temp update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6]);

#elif (OREGON_MODE == MODE_1)
    // temp or humidity update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7]);
#elif (OREGON_MODE == MODE_2)
    // temp, humidity or pressure update?
    shouldEmitData =
        (oregon._oregonMessageBuffer[4] != _lastOregonMessageBuffer[4] ||
         oregon._oregonMessageBuffer[5] != _lastOregonMessageBuffer[5] ||
         oregon._oregonMessageBuffer[6] != _lastOregonMessageBuffer[6] ||
         oregon._oregonMessageBuffer[7] != _lastOregonMessageBuffer[7] ||
         oregon._oregonMessageBuffer[8] != _lastOregonMessageBuffer[8] ||
         oregon._oregonMessageBuffer[9] != _lastOregonMessageBuffer[9]);
#else
#error OREGON_MODE has an unknown value
#endif

    // absolute counter for emission ~ each 15 minutes
    if (secondCounter > 900) {
      secondCounter = 0;
      shouldEmitData = true;
    }

    PORTB |= _BV(LED_PIN); // led on

    if (shouldEmitData) {

      // Activate radio power
      PORTA |= _BV(RADIO_POWER_PIN);
      _delay_ms(2);

      // led off here, it will allow a short
      // blink when actually emitting new data
      PORTB &= ~_BV(LED_PIN);

// Buffer for Oregon message
#if OREGON_MODE == MODE_0
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 8);
#elif OREGON_MODE == MODE_1
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 9);
#elif OREGON_MODE == MODE_2
      memcpy(_lastOregonMessageBuffer, oregon._oregonMessageBuffer, 11);
#else
#error mode unknown
#endif

      oregon.txPinLow();
      _delay_us(oregon.TWOTIME * 8);

      oregon.sendOregon(oregon._oregonMessageBuffer,
                        sizeof(oregon._oregonMessageBuffer));

      // pause before new transmission
      oregon.txPinLow();
      _delay_us(oregon.TWOTIME * 8);

      // second emission with led on
      PORTB |= _BV(LED_PIN);

      oregon.sendOregon(oregon._oregonMessageBuffer,
                        sizeof(oregon._oregonMessageBuffer));

      // radio off
      PORTA &= ~_BV(RADIO_POWER_PIN);
    }
    // led off
    PORTB &= ~_BV(LED_PIN);

#endif
    sleep(SLEEP_TIME_IN_SECONDS);
    secondCounter += SLEEP_TIME_IN_SECONDS;
  }
}

/*
 * Puts MCU to sleep for specified number of seconds
 *
 * As the hardware watchdog is set for timeout after 8s,
 * the value used here will be divided by 8 (and rounded if needed).
 * It is better to use a multiple of 8 as value.
 */
 
void sleep(uint8_t s) {
  s >>= 3; // or s/8
  if (s == 0)
    s = 1;
  sleep_interval = 0;
  while (sleep_interval < s) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();

    uint8_t PRR_backup = PRR;
    uint8_t porta_backup = PORTA;
    uint8_t portb_backup = PORTB;
    uint8_t ddra_backup = DDRA;
    uint8_t ddrb_backup = DDRB;
    
    PRR |= (1 << PRUSI);
    PRR |= (1 << PRTIM0);
    PRR |= (1 << PRTIM1);

    // all pins as input to avoid power draw
    DDRA = 0;
    DDRB = 0;

    // pull-up all unused pins by default
    PORTA |= 0b01110000;
	
    // pullup reset only
    // ( only 4 lasts are available - p. 67)
    PORTB |= 0b00001000; 

    sleep_mode();
    // here the system is sleeping

    // here the system wakes up
    sleep_disable();

    // restore
    PRR = PRR_backup;
    DDRA = ddra_backup;
    DDRB = ddrb_backup;
    PORTA = porta_backup;
    PORTB = portb_backup;
  }
}
