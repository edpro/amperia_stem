#include <AmperiaSTEM.h>

AmperiaSensorClient sourceSensor;
AmperiaBridge bridge(sourceSensor);

void setup()
{
    Serial.begin(115200);

    if (!sourceSensor.begin()) {
        Serial.println("Failed to start the external sensor.");
        return;
    }

    delay(5000);  // Give the external sensor time to start.

    if (!sourceSensor.discover() || sourceSensor.sensorCount() == 0) {
        Serial.println("External sensor was not detected.");
        return;
    }

    if (!bridge.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    // Publish one initial snapshot without waiting for DREADY.
    if (bridge.update()) {
        bridge.publish();
    }
}

void loop()
{
    if (bridge.updateIfReady()) {
        bridge.publish();
    }
}
