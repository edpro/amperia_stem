#include <AmperiaSTEM.h>
#include <DHT.h>

constexpr uint8_t dhtDataPin = 12;
constexpr uint16_t humidityMeasurementPeriodMs = 2000;

AmperiaSensorClient sourceSensor;
AmperiaBridge bridge(sourceSensor);
DHT humiditySensor(dhtDataPin, DHT11);

int humidityOutput = -1;
uint32_t previousHumidityMeasurementMs = 0;

bool updateHumidity()
{
    const float humidity = humiditySensor.readHumidity();

    if (isnan(humidity)) {
        Serial.println("Failed to read humidity from DHT11.");
        return false;
    }

    return bridge.set(humidityOutput, humidity);
}

void setup()
{
    Serial.begin(115200);
    humiditySensor.begin();

    if (!sourceSensor.begin()) {
        Serial.println("Failed to start the external sensor.");
        return;
    }

    // This also gives the DHT11 enough time before its first measurement.
    delay(5000);

    if (!sourceSensor.discover() || sourceSensor.sensorCount() == 0) {
        Serial.println("External sensor was not detected.");
        return;
    }

    // Adding a local sensor reserves one of the five logical-sensor slots.
    // The bridge forwards as many external sensors as fit in the remaining
    // slots and hides that bookkeeping from the sketch.
    humidityOutput = bridge.addSensor("DHT11 humidity", "%");
    if (humidityOutput < 0 ||
        !bridge.setDataPeriodMs(humidityOutput, humidityMeasurementPeriodMs) ||
        !bridge.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    if (!bridge.update()) {
        Serial.println("Failed to read the initial external measurements.");
        return;
    }

    if (!updateHumidity()) {
        return;
    }

    bridge.publish();
    previousHumidityMeasurementMs = millis();
}

void loop()
{
    bool changed = bridge.updateIfReady();
    const uint32_t now = millis();

    if (now - previousHumidityMeasurementMs >= humidityMeasurementPeriodMs) {
        previousHumidityMeasurementMs = now;

        if (updateHumidity()) {
            changed = true;
        }
    }

    if (changed) {
        bridge.publish();
    }
}
