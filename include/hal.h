#ifndef __HAL_H__
#define __HAL_H__

/*
 * Interface definition for lower level hardware function
 *
 */
class Hal
{
public:
    // Radio related
    virtual void LedOn() const = 0;
    virtual void DelayMs() const = 0;

    // Radio related
    virtual void RadioGoLow() const = 0;
    virtual void RadioGoHigh() const = 0;
};

#endif