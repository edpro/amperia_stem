#pragma once

#include <Arduino.h>
#include <Servo.h>

namespace amperia
{
namespace detail
{
    class ServoMotor
    {
    public:
        explicit ServoMotor(uint8_t pin);

        void begin();
        void setAngle(int degrees);
        void rotate(int degrees);
        int getAngle() const;

    private:
        Servo m_servo;
        uint8_t m_pin;
        int m_currentAngle;
    };
}
}
