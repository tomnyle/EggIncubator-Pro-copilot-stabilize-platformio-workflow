#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

/******************************************************
 * PID Controller for Temperature & Humidity Control
 ******************************************************/

class PIDController
{
public:

    /******************************************************
     * Constructor
     ******************************************************/
    PIDController(
        float kp = 1.0f,
        float ki = 0.1f,
        float kd = 0.05f);

    /******************************************************
     * Configuration
     ******************************************************/
    void setTunings(float kp, float ki, float kd);

    void setSetpoint(float setpoint);

    void setOutputLimits(float minOut, float maxOut);

    void setIntegralLimits(float minInt, float maxInt);

    /******************************************************
     * Calculation
     ******************************************************/
    float calculate(float input);

    float calculate(float input, uint32_t now);

    /******************************************************
     * Reset
     ******************************************************/
    void reset();

    /******************************************************
     * Getters
     ******************************************************/
    float getSetpoint() const;

    float getError() const;

    float getOutput() const;

    float getP() const;

    float getI() const;

    float getD() const;

private:

    // Tuning parameters
    float kp;
    float ki;
    float kd;

    // Set point
    float setpoint = 0.0f;

    // Previous error for derivative
    float prevError = 0.0f;

    // Integral accumulator
    float integral = 0.0f;

    // Output limits
    float minOutput = -100.0f;
    float maxOutput = 100.0f;

    // Integral anti-windup limits
    float minIntegral = -100.0f;
    float maxIntegral = 100.0f;

    // Last calculation time
    unsigned long lastTime = 0;

    // Calculation results
    float error = 0.0f;
    float output = 0.0f;

    float pTerm = 0.0f;
    float iTerm = 0.0f;
    float dTerm = 0.0f;

    /******************************************************
     * Clamp value between min and max
     ******************************************************/
    static float clamp(float value, float min, float max);
};

#endif
