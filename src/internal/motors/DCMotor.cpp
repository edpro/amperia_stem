#include "internal/motors/DCMotor.h"

namespace amperia
{
namespace detail
{
    DCMotor::DCMotor(uint8_t pinA, uint8_t pinB)
        : m_pinA(pinA), m_pinB(pinB), m_speed(0), m_running(false)
    {}

    void DCMotor::begin()
    {
        pinMode(m_pinA, OUTPUT);
        pinMode(m_pinB, OUTPUT);
        stop();
    }

    void DCMotor::setSpeed(int16_t speed)
    {
        if (speed > 255) {
            speed = 255;
        }
        if (speed < -255) {
            speed = -255;
        }

        if (speed == 0) {
            stop();
            return;
        }

        m_speed = speed;
        m_running = true;

        if (speed > 0) {
            analogWrite(m_pinB, 0);
            digitalWrite(m_pinB, LOW);
            analogWrite(m_pinA, static_cast<uint8_t>(speed));
        } else {
            analogWrite(m_pinA, 0);
            digitalWrite(m_pinA, LOW);
            analogWrite(m_pinB, static_cast<uint8_t>(-speed));
        }
    }

    void DCMotor::stop()
    {
        analogWrite(m_pinA, 0);
        analogWrite(m_pinB, 0);
        digitalWrite(m_pinA, LOW);
        digitalWrite(m_pinB, LOW);
        m_speed = 0;
        m_running = false;
    }

    bool DCMotor::isRunning() const
    {
        return m_running;
    }

    int16_t DCMotor::getSpeed() const
    {
        return m_speed;
    }
}
}
