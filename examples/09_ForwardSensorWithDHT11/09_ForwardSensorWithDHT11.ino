/*
 * ============================================================
 *  AmperiaSTEM — FORWARD SENSOR WITH DHT11
 *
 *  Читаємо вологість з DHT11 і додаємо її як локальний сенсор,
 *  після чого передаємо всі значення на Amperia Multimeter.
 *
 *  Read humidity from a DHT11, add it as a local sensor, and then
 *  forward all values to the Amperia Multimeter.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - DHT11 (підключений до Arduino через пін D12)
 *  - EdPro Sensor (підключений до EdPro Amperia Shield через
 *    роз'єм SENSOR)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - DHT11 connected to the Arduino through pin D12
 *  - EdPro Sensor connected to the shield's SENSOR connector
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

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

    delay(5000);

    if (!sourceSensor.discover() || sourceSensor.sensorCount() == 0) {
        Serial.println("External sensor was not detected.");
        return;
    }

    // Додаємо локальний сенсор, щоб зарезервувати один зі слотів логічних сенсорів.

    // Adding a local sensor reserves one of the available logical-sensor slots.
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
