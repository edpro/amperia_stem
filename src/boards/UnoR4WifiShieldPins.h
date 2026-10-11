#pragma once

#include <Arduino.h>

namespace amperia
{
namespace board
{
namespace unoR4Wifi
{
    // Confirmed shield connections for the current hardware revision.
    // Recheck them against the final schematic before a public hardware
    // release. The protocol layer must not depend on these constants; they
    // are defaults for the UNO R4 WiFi adapter only.
    constexpr uint8_t kMultimeterDataReadyPin = A2;
    constexpr uint8_t kSensorDataReadyPin     = A3;

    constexpr uint8_t kServo1Pin              = 2;
    constexpr uint8_t kServo2Pin              = 4;

    // Control inputs of the on-board DRV8833 DC motor driver.
    constexpr uint8_t kDcMotor1ControlPin     = 3;
    constexpr uint8_t kDcMotor2ControlPin     = 5;

    constexpr uint8_t kStepperIn1Pin          = 6;
    constexpr uint8_t kStepperIn2Pin          = 7;
    constexpr uint8_t kStepperIn3Pin          = 8;
    constexpr uint8_t kStepperIn4Pin          = 9;
}
}
}
