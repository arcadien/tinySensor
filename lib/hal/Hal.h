#pragma once
/*
 * Interface definition for lower level hardware function
 *
 */
class Hal
{
public:
    // Radio related
    virtual void LedOn() const = 0;

    virtual void DelayUs(long us) const = 0;

    // Radio related
    virtual void RadioGoLow() const = 0;
    virtual void RadioGoHigh() const = 0;
};