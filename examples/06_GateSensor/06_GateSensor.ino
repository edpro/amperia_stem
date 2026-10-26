/*
 * ============================================================
 *  AmperiaSTEM — GATE SENSOR
 *
 *  Читаємо два лічильники зовнішнього Gate Sensor і
 *  передаємо їхні значення на Amperia Multimeter.
 *
 *  Read the two counters from the external Gate Sensor and
 *  forward them to the Amperia Multimeter.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro Gate Sensor (підключений до EdPro Amperia Shield через
 *    роз'єм SENSOR)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro Gate Sensor connected to the shield's SENSOR connector
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Параметр періоду вимірювань для мультиметра. Може бути змінений.

// Measurement period for the multimeter output. Can be changed.
constexpr uint16_t measurementPeriodMs = amperia::kDefaultDataPeriodMs;

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

    delay(5000);

    if (!gateSensor.discover()) {
        Serial.println("Gate Sensor was not detected.");
        return;
    }

    // Вибираємо два окремі лічильники за іменами, а не за індексами датчиків.

    // Select the two dedicated counter records by name instead of relying on sensor indexes.
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
