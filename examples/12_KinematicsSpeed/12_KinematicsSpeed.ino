/*
 * ============================================================
 *  AmperiaSTEM — KINEMATICS SPEED
 *
 *  Читаємо час прольоту з фотоворіт EdPro, обчислюємо швидкість
 *  і передаємо її як новий віртуальний сенсор на Amperia Multimeter.
 *
 *  Read the travel time from the EdPro gate sensor, calculate speed,
 *  and expose it as a new virtual sensor on the Amperia Multimeter.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro Gate Sensor (фотоворота, підключені через роз'єм SENSOR)
 *  - Об'єкт, що перетинає ворота, наприклад кулька або пластина
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro Gate Sensor connected to the shield's SENSOR connector
 *  - Object crossing the gate, such as a ball or plate
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Довжина об'єкта, що перетинає ворота, у метрах.

// Length of the object crossing the gate, in meters.

constexpr float objectDistance = 0.05; // 0.05 м = 5 см

                                      // 0.05 m = 5 cm

AmperiaSensorClient sensors;

SimpleSensor speedSensor(
    "Kinematics",
    "Speed",
    "m/s"
);

float getTravelTime()
{
    auto* sensor = sensors.findSensor("Travel (S)");

    if (sensor == nullptr) {
        return -1000.0f; // Сигнальне значення: від'ємний час прольоту неможливий

                         // Sentinel value: a negative travel time is impossible
    }

    return sensor->value();
}

bool doMeasurement()
{
    // Отримуємо час прольоту між воротами в секундах.

    // Read the travel time in seconds.
    float travelTime = getTravelTime();

    // Захист від ділення на нуль і обробка лише валідних вимірювань.

    // Avoid division by zero and process only valid measurements.
    if (travelTime > 0.0) {

        // Обчислюємо швидкість за формулою v = d / t.

        // Calculate speed using v = d / t.
        float speed = objectDistance / travelTime;

        speedSensor.publish(speed);

        Serial.print("Travel time: ");
        Serial.print(travelTime, 4);
        Serial.print(" s | Speed: ");
        Serial.print(speed, 4);
        Serial.println(" m/s");
    }

    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    delay(5000);

    if (!sensors.discover() || sensors.findSensor("Travel (S)") == nullptr) {
        Serial.println("Gate timing sensor was not detected.");
        return;
    }

    if (!speedSensor.begin()) {
        Serial.println("Failed to initialize the SimpleSensor.");
        return;
    }

    if (sensors.update()) {
        doMeasurement();
    }

    Serial.println("System Ready! Waiting for an object to pass through the gate...");
}

void loop()
{
    if (sensors.updateIfReady()) {
        doMeasurement();
    }
}