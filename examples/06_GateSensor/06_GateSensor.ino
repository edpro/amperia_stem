#include <AmperiaSTEM.h>

constexpr uint16_t measurementPeriodMs = amperia::kDefaultDataPeriodMs;

// Read the external Gate Sensor and forward its two counters to the multimeter.
AmperiaSensorClient gateSensor;
const AmperiaSensorClient::Sensor* firstCounter = nullptr;
const AmperiaSensorClient::Sensor* secondCounter = nullptr;

NDimSensor gateOutput(
    "Gate Sensor",
    "Gate counts",
    "count",
    2
);

void setup()
{
    Serial.begin(115200);

    if (!gateSensor.begin()) {
        Serial.println("Failed to start the Gate Sensor.");
        return;
    }

    delay(5000);  // Give the external sensor time to start.

    if (!gateSensor.discover()) {
        Serial.println("Gate Sensor was not detected.");
        return;
    }

    // The Gate Sensor also reports timing measurements. Select the two
    // dedicated counter records by name instead of relying on sensor indexes.
    firstCounter = gateSensor.findSensor("Number 1");
    secondCounter = gateSensor.findSensor("Number 2");

    if (firstCounter == nullptr || secondCounter == nullptr) {
        Serial.println("Gate Sensor counters were not detected.");
        return;
    }

    gateOutput.setDataPeriodMs(measurementPeriodMs);
    if (!gateOutput.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }
}

void loop()
{
    if (gateSensor.update()) {
        gateOutput.publish(
            firstCounter->value(),
            secondCounter->value());
    }

    delay(measurementPeriodMs);
}
