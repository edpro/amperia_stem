#include <AmperiaSTEM.h>

constexpr uint16_t measurementPeriodMs = 100;

NDimSensor pinStateSensor(
    "4-pin tester",
    "Inputs",
    "logic",
    4
);

int32_t readLogicLevel(uint8_t pin)
{
    if (digitalRead(pin) == HIGH) {
        return 1;
    }

    return 0;
}

void setup()
{
    // D0 and D1 must not be used by Serial1 while this example is running.
    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);

    pinStateSensor.setDataPeriodMs(measurementPeriodMs);
    if (!pinStateSensor.begin()) {
        while (true) {
            delay(1000);
        }
    }
}

void loop()
{
    pinStateSensor.set(0, readLogicLevel(0));
    pinStateSensor.set(1, readLogicLevel(1));
    pinStateSensor.set(2, readLogicLevel(A0));
    pinStateSensor.set(3, readLogicLevel(A1));
    pinStateSensor.publish();

    delay(measurementPeriodMs);
}
