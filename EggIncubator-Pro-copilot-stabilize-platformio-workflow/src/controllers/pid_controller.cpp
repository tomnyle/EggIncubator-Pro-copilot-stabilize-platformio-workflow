#include "pid_controller.h"

/******************************************************
 * Constructor
 ******************************************************/

PIDController::PIDController(float kp, float ki, float kd)
    : kp(kp), ki(ki), kd(kd)
{
    lastTime = millis();
}

/******************************************************
 * Set Tuning Parameters (Kp, Ki, Kd)
 ******************************************************/

void PIDController::setTunings(float kp, float ki, float kd)
{
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

/******************************************************
 * Set Setpoint (Target Value)
 ******************************************************/

void PIDController::setSetpoint(float setpoint)
{
    this->setpoint = setpoint;
}

/******************************************************
 * Set Output Limits (Anti-windup)
 ******************************************************/

void PIDController::setOutputLimits(float minOut, float maxOut)
{
    this->minOutput = minOut;
    this->maxOutput = maxOut;
}

/******************************************************
 * Set Integral Limits
 ******************************************************/

void PIDController::setIntegralLimits(float minInt, float maxInt)
{
    this->minIntegral = minInt;
    this->maxIntegral = maxInt;
}

/******************************************************
 * Calculate PID Output (Current Time)
 ******************************************************/

float PIDController::calculate(float input)
{
    return calculate(input, millis());
}

/******************************************************
 * Calculate PID Output (With Timestamp)
 ******************************************************/

float PIDController::calculate(float input, uint32_t now)
{
    // Calculate time delta (in seconds)
    uint32_t timeDelta = now - lastTime;
    lastTime = now;

    if (timeDelta == 0)
    {
        return output;
    }

    float dt = timeDelta / 1000.0f;  // Convert to seconds

    // Calculate error
    error = setpoint - input;

    // Proportional term
    pTerm = kp * error;

    // Integral term (with anti-windup)
    integral += ki * error * dt;
    integral = clamp(integral, minIntegral, maxIntegral);
    iTerm = integral;

    // Derivative term
    float deltaError = error - prevError;
    dTerm = kd * (deltaError / dt);

    prevError = error;

    // Total output
    output = pTerm + iTerm + dTerm;

    // Clamp output to limits
    output = clamp(output, minOutput, maxOutput);

    return output;
}

/******************************************************
 * Reset PID Controller
 ******************************************************/

void PIDController::reset()
{
    error = 0.0f;
    prevError = 0.0f;
    integral = 0.0f;
    output = 0.0f;

    pTerm = 0.0f;
    iTerm = 0.0f;
    dTerm = 0.0f;

    lastTime = millis();
}

/******************************************************
 * Getters
 ******************************************************/

float PIDController::getSetpoint() const
{
    return setpoint;
}

float PIDController::getError() const
{
    return error;
}

float PIDController::getOutput() const
{
    return output;
}

float PIDController::getP() const
{
    return pTerm;
}

float PIDController::getI() const
{
    return iTerm;
}

float PIDController::getD() const
{
    return dTerm;
}

/******************************************************
 * Clamp Value
 ******************************************************/

float PIDController::clamp(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
