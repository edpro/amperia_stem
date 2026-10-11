/*
 * ============================================================
 *  AmperiaSTEM — 12_KinematicsSpeed
 *  Кінематика: Обчислення швидкості.
 *  Читаємо час прольоту з фотоворіт EdPro, обчислюємо швидкість 
 *  і відправляємо на мультиметр як новий віртуальний сенсор.
 *  Kinematics: speed calculation.
 *  Read travel time from the EdPro gate sensor, calculate speed,
 *  and expose the result as a new virtual sensor.
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Константа: довжина об'єкта (наприклад, кульки), що перетинає ворота, у метрах.
// 0.05 м = 5 см.
// Length of the object crossing the gate, in meters.
// 0.05 m = 5 cm.
constexpr float objectDistance = 0.05; 

/*
 * ============================================================
 * ОГОЛОШЕННЯ ОБ'ЄКТІВ
 * OBJECT DECLARATIONS
 * ============================================================
 */
AmperiaSensorClient sensors;            // Читання фотоворіт з EdPro Sensor
                                        // Reads the EdPro gate sensor

SimpleSensor speedSensor(
    "Kinematics",
    "Speed",
    "m/s"
);

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * HELPER FUNCTIONS
 *
 * Конкретна назва логічного сенсора фотоворіт залишається
 * в цьому прикладі, а не в універсальному класі бібліотеки.
 * The gate sensor's logical-sensor name belongs to this example
 * rather than the generic library client.
 * ============================================================
 */
float getTravelTime()
{
    auto* sensor = sensors.findSensor("Travel (S)");

    if (sensor == nullptr) {
        return -1000.0f; // Від'ємний час прольоту є неможливим
                         // Sentinel value: a negative travel time is impossible
    }

    return sensor->value();
}

/*
 * ============================================================
 * ТВОЯ ЛОГІКА ВИМІРЮВАНЬ
 * MEASUREMENT LOGIC
 * ============================================================
 */
bool doMeasurement() 
{
    // Отримуємо час прольоту між воротами (в секундах).
    // Read the travel time in seconds.
    float travelTime = getTravelTime();

    // Захист від ділення на нуль та обробка лише валідних вимірювань.
    // Avoid division by zero and process only valid measurements.
    if (travelTime > 0.0) {

        // Обчислення швидкості: v = d / t.
        // Calculate speed: v = d / t.
        float speed = objectDistance / travelTime;

        // Публікуємо значення нашого віртуального сенсора.
        // Publish the virtual speed sensor.
        speedSensor.publish(speed);

        // Вивід у Serial Monitor для зручності.
        // Print the result to Serial Monitor.
        Serial.print("Час прольоту: "); 
        Serial.print(travelTime, 4); 
        Serial.print(" с | Швидкість: ");
        Serial.print(speed, 4);
        Serial.println(" м/с");
    }

    return true;
}

/*
 * ============================================================
 *  SETUP 
 * ============================================================
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    // Ініціалізуємо читання фотоворіт.
    // Initialize the gate-sensor interface.
    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    // Даємо зовнішньому сенсору час завершити запуск.
    // Give the external sensor time to finish startup.
    delay(5000);

    // Зчитуємо опис сенсорів і перевіряємо наявність часу прольоту.
    // Discover sensor metadata and verify that travel time is available.
    if (!sensors.discover() || sensors.findSensor("Travel (S)") == nullptr) {
        Serial.println("Gate timing sensor was not detected.");
        return;
    }

    // Ініціалізуємо власний сенсор швидкості.
    // Initialize the custom speed sensor.
    if (!speedSensor.begin()) {
        Serial.println("Помилка ініціалізації SimpleSensor!");
        return;
    }

    // Одразу опрацьовуємо перше доступне вимірювання.
    // Process the first available measurement immediately.
    if (sensors.update()) {
        doMeasurement();
    }

    Serial.println("System Ready! Waiting for object to pass through the gates...");
}

/*
 * ============================================================
 *  LOOP 
 * ============================================================
 */
void loop() {
    // Нове вимірювання обробляємо після сигналу DREADY від фотоворіт.
    // Process a new measurement after the gate sensor asserts DREADY.
    if (sensors.updateIfReady()) {
        doMeasurement();
    }
}