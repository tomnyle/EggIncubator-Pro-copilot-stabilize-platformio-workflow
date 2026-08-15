#include "bts7960.h"

#define RPWM_CHANNEL 0
#define LPWM_CHANNEL 1

#define PWM_FREQ       20000
#define PWM_RESOLUTION 8

BTS7960::BTS7960(uint8_t rpwm,
                 uint8_t lpwm,
                 uint8_t enable)
{
    _rpwm = rpwm;
    _lpwm = lpwm;
    _enable = enable;
}

void BTS7960::begin()
{
    pinMode(_enable, OUTPUT);
    digitalWrite(_enable, LOW);

    // Cấu hình PWM
    ledcSetup(RPWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(LPWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

    ledcAttachPin(_rpwm, RPWM_CHANNEL);
    ledcAttachPin(_lpwm, LPWM_CHANNEL);

    stop();
}

void BTS7960::enable()
{
    digitalWrite(_enable, HIGH);
}

void BTS7960::disable()
{
    stop();
    digitalWrite(_enable, LOW);
}

void BTS7960::forward(uint8_t speed)
{
    enable();

    ledcWrite(LPWM_CHANNEL, 0);
    ledcWrite(RPWM_CHANNEL, speed);
}

void BTS7960::backward(uint8_t speed)
{
    enable();

    ledcWrite(RPWM_CHANNEL, 0);
    ledcWrite(LPWM_CHANNEL, speed);
}

void BTS7960::stop()
{
    ledcWrite(RPWM_CHANNEL, 0);
    ledcWrite(LPWM_CHANNEL, 0);
}