#include "internal/motors/ServoMotor.h"

namespace amperia
{
namespace detail
{
    ServoMotor::ServoMotor(uint8_t pin)
        : m_pin(pin), m_currentAngle(90)
    {}

    void ServoMotor::begin()
    {
        m_servo.attach(m_pin);
        m_servo.write(m_currentAngle);
    }

    void ServoMotor::setAngle(int degrees)
    {
        degrees = constrain(degrees, 0, 180);
        m_servo.write(degrees);
        m_currentAngle = degrees;
    }

    void ServoMotor::rotate(int degrees)
    {
        setAngle(m_currentAngle + degrees);
    }

    int ServoMotor::getAngle() const
    {
        return m_currentAngle;
    }
}
}
