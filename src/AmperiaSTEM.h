#pragma once

#if !defined(ARDUINO_UNOR4_WIFI)
#error "AmperiaSTEM supports only the Arduino Uno R4 WiFi board. Please select 'Arduino Uno R4 WiFi' in Arduino IDE."
#endif

#include <Arduino.h>
#include <Wire.h>
#include "client/AmperiaSensorClient.h"
#include "MySensor.h"
#include "AmperiaBridge.h"

#include <Stepper.h>

// Board pin definitions and internal actuator implementations.
#include "boards/UnoR4WifiShieldPins.h"
#include "internal/motors/DCMotor.h"
#include "internal/motors/ServoMotor.h"

/* ============================================================
 *   AmpStepper  --  Stepper motor wrapper
 * ============================================================
*/
class AmpStepper 
{
public:
    /**
     * @param in1..in4    Arduino pins connected to IN1-IN4 of the driver
     * @param stepsPerRev Steps per full revolution (check your motor datasheet)
     * @param rpm         Default speed in RPM (can be changed later with setSpeed)
     */
    AmpStepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4,
               int stepsPerRev = 200, int rpm = 60)
        : _motor(stepsPerRev, in1, in2, in3, in4),
          _stepsPerRev(stepsPerRev)
    {
        _motor.setSpeed(rpm);
    }

    /** Constructor using shield pin defaults */
    AmpStepper()
        : AmpStepper(
            amperia::board::unoR4Wifi::kStepperIn1Pin,
            amperia::board::unoR4Wifi::kStepperIn3Pin,
            amperia::board::unoR4Wifi::kStepperIn2Pin,
            amperia::board::unoR4Wifi::kStepperIn4Pin)
    {}

    /** Move `steps` steps forward (+) or backward (−). Blocking. */
    void step(int steps) { _motor.step(steps); }

    /** Move by angle (degrees). */
    void stepDegrees(float degrees) 
    {
        int steps = (int)((degrees / 360.0f) * _stepsPerRev);
        _motor.step(steps);
    }

    /** Change speed. */
    void setSpeed(int rpm) { _motor.setSpeed(rpm); }

private:
    Stepper _motor;
    int     _stepsPerRev;
};

/* ============================================================
 *   AmpServo  --  Servo motor wrapper
 * ============================================================
*/
/**
 * AmpServo -- simple servo API for the Amperia shield.
 */
class AmpServo {
public:
    /**
     * Construct by index or pin:
     *  - `AmpServo s(1);` selects SRV1 (shield pin)
     *  - `AmpServo s(2);` selects SRV2 (shield pin)
     *  - `AmpServo s(pinNumber);` for other Arduino pins
     */
    AmpServo(int servoIndex = 1)
        : _motor((servoIndex == 2) ? amperia::board::unoR4Wifi::kServo2Pin
                                    : (servoIndex == 1) ? amperia::board::unoR4Wifi::kServo1Pin
                                                         : (uint8_t)servoIndex)
    {}

    void begin() { _motor.begin(); }
    void setAngle(int degrees) { _motor.setAngle(degrees); }
    void rotate(int degrees) { _motor.rotate(degrees); }
    int getAngle() const { return _motor.getAngle(); }

private:
    amperia::detail::ServoMotor _motor;
};

/**
 * AmpServos -- holder for two on-shield servos (SRV1 / SRV2)
 */
class AmpServos {
public:
    AmpServos()
        : servo1(1),
          servo2(2)
    {}

    void begin() { servo1.begin(); servo2.begin(); }

    AmpServo servo1;
    AmpServo servo2;
};

/* ============================================================
 *   AmpDC  --  DC motor wrapper
 * ============================================================
 */
class AmpDC {
public:
    /**
     * @param pinA PWM/control pin A
     * @param pinB PWM/control pin B
     */
    explicit AmpDC(uint8_t pinA = amperia::board::unoR4Wifi::kDcMotor1ControlPin,
                   uint8_t pinB = amperia::board::unoR4Wifi::kDcMotor2ControlPin)
        : _motor(pinA, pinB) {}

    void begin() { _motor.begin(); }

    void setSpeed(int16_t speed) { _motor.setSpeed(speed); }
    void stop() { _motor.stop(); }

    bool isRunning() const { return _motor.isRunning(); }
    int16_t getSpeed() const { return _motor.getSpeed(); }

private:
    amperia::detail::DCMotor _motor;
};
