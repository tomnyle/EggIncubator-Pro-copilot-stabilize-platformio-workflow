#ifndef BTS7960_H
#define BTS7960_H

#include <Arduino.h>

class BTS7960
{
public:
    BTS7960(uint8_t rpwm,
            uint8_t lpwm,
            uint8_t enable);

    void begin();

    void enable();

    void disable();

    void forward(uint8_t speed);

    void backward(uint8_t speed);

    void stop();

private:
    uint8_t _rpwm;
    uint8_t _lpwm;
    uint8_t _enable;
};

#endif