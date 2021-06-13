#pragma once

#include <Hal.h>

/*
* 1-wire interface is on PA3
* Power for radio is on PA2 (sensor_vcc)
* LED pin is PB1
* PWM pin is PA7
* SDA pin is PA6
* SCL pin is PA4
* MISO pin is PA5
* TX radio pin is PB0
* BAT sensor pin is PA1
* analog sensor pin is PA0
*/

#define LED_PIN PB1
#define PWM_PIN PA7
#define TX_RADIO_PIN PB0
#define SENSOR_VCC PA2
#define BAT_SENSOR_PIN PA1


class ATtiny84aHal : public Hal
{
    public:

    // Radio related
    void LedOn() const override;

    // Radio related
    void RadioGoLow() const override;
    void RadioGoHigh() const override;

};
