#pragma once

#include <Arduino.h>

namespace amperia
{
namespace detail
{
    class DCMotor
    {
    public:
        DCMotor(uint8_t pinA, uint8_t pinB);

        void begin();
        void setSpeed(int16_t speed);
        void stop();

        bool isRunning() const;
        int16_t getSpeed() const;

    private:
        uint8_t m_pinA;
        uint8_t m_pinB;
        int16_t m_speed;
        bool m_running;
    };
}
}
