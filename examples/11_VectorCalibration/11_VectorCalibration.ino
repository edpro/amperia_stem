/*
 * ============================================================
 *  AmperiaSTEM — 11_VectorCalibration
 *  Фізичний експеримент: Вектори і Калібрування.
 *  Читаємо магнітометр, використовуємо кнопку на D10 (BTN1) 
 *  для збереження поточних значень як "нульового" рівня.
 *  Відкалібровані дані виводяться на мультиметр через NDimSensor.
 *  Physical experiment: vectors and calibration.
 *  Read a magnetometer and use a button on D10 (BTN1) to store the current
 *  values as the zero reference. Publish the calibrated data through
 *  NDimSensor.
 * ============================================================
 *
 *  Підключення:
 *    - EdPro Magnetometer
 *    - Тактова кнопка: BTN1 (контакт D10)
 *  Connections:
 *    - EdPro Magnetometer
 *    - Push button: BTN1 (contact D10)
 * ============================================================
 */

#include <AmperiaSTEM.h>
#include <math.h> // Для математичних функцій (sqrt)
                  // Used for sqrt()

constexpr uint8_t buttonPin = 10;

/*
 * ============================================================
 * ГЛОБАЛЬНІ ЗМІННІ СТАНУ (State Variables)
 * GLOBAL STATE VARIABLES
 * Тут зберігаємо наш "нульовий рівень".
 * Stores the zero-reference values.
 * ============================================================
 */
float tareX = 0.0;
float tareY = 0.0;
float tareZ = 0.0;

/*
 * ============================================================
 * ОГОЛОШЕННЯ ОБ'ЄКТІВ
 * OBJECT DECLARATIONS
 * ============================================================
 */
AmperiaSensorClient sensors;             // читання магнітометра з EdPro Sensor
                                         // Reads the EdPro magnetometer

NDimSensor calibratedMag(
    "Calib. Magnet",
    "Axis",
    "mkT",
    4
);

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * HELPER FUNCTIONS
 *
 * Знаємо назву логічного сенсора саме тут, у прикладі роботи
 * з магнітометром. Універсальний клієнт про конкретні моделі
 * сенсорів нічого не знає.
 * The logical sensor name belongs to this magnetometer example.
 * The generic client does not know concrete sensor models.
 * ============================================================
 */
float getMagneticAxis(int axis)
{
    auto* sensor = sensors.findSensor("VecXYZ");

    if (sensor == nullptr) {
        return -10000.0f; // Значення далеко поза діапазоном цього магнітометра
                          // Sentinel value well outside this magnetometer's range
    }

    return sensor->value(axis);
}

/*
 * ============================================================
 * ТВОЯ ЛОГІКА ВИМІРЮВАНЬ
 * MEASUREMENT LOGIC
 * ============================================================
 */
bool doMeasurement() 
{
    // Отримуємо сирі значення з магнітометра.
    // Read the raw magnetometer components.
    float mx = getMagneticAxis(0);      // вісь X, mkT
                                        // X axis, mkT
    float my = getMagneticAxis(1);      // вісь Y, mkT
                                        // Y axis, mkT
    float mz = getMagneticAxis(2);      // вісь Z, mkT
                                        // Z axis, mkT

    // Перевіряємо стан кнопки BTN1 на D10.
    // Check the BTN1 calibration button on D10.
    if (digitalRead(buttonPin) == LOW) {
        // Зберігаємо поточні значення як новий нуль.
        // Store the current values as the new zero reference.
        tareX = mx;
        tareY = my;
        tareZ = mz;

        Serial.println("--- КАЛІБРУВАННЯ ВИКОНАНО ---");

        // Простий debounce (захист від брязкоту контактів)
        // Simple debounce
        delay(200); 
    }

    // Обчислюємо векторну різницю (відносні значення).
    // Compute the relative vector components.
    float diffX = mx - tareX;
    float diffY = my - tareY;
    float diffZ = mz - tareZ;

    // Обчислення модуля нового (відносного) вектора магнітного поля.
    // Compute the magnitude of the relative magnetic-field vector.
    float diffMag = sqrt((diffX * diffX) + (diffY * diffY) + (diffZ * diffZ));

    // Передаємо всі компоненти вектора на мультиметр.
    // Publish all vector components to the multimeter.
    calibratedMag.publish(diffX, diffY, diffZ, diffMag);

    // Вивід у Serial Monitor для зручності відлагодження.
    // Print the result to Serial Monitor for diagnostics.
    Serial.print("Відносне поле [mkT] -> X: "); Serial.print(diffX);
    Serial.print(" | Y: "); Serial.print(diffY);
    Serial.print(" | Z: "); Serial.print(diffZ);
    Serial.print(" | Магнітуда: "); Serial.println(diffMag);

    return true;
}

/*
 * ============================================================
 *  SETUP — не змінюй без потреби
 *  SETUP — do not change unless necessary
 * ============================================================
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    pinMode(buttonPin, INPUT_PULLUP);

    // Ініціалізуємо читання зовнішнього магнітометра.
    // Initialize the external magnetometer interface.
    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    // Даємо зовнішньому сенсору час завершити запуск.
    // Give the external sensor time to finish startup.
    delay(5000);

    // Зчитуємо опис сенсорів і перевіряємо наявність вектора магнітного поля.
    // Discover sensor metadata and verify the magnetic-field vector.
    if (!sensors.discover() || sensors.findSensor("VecXYZ") == nullptr) {
        Serial.println("Magnetometer was not detected.");
        return;
    }

    // Ініціалізуємо власний сенсор відкаліброваного магнітного поля.
    // Initialize the custom calibrated magnetic-field sensor.
    if (!calibratedMag.begin()) {
        Serial.println("Failed to start the custom NDimSensor.");
        return;
    }

    // Одразу опрацьовуємо перше доступне вимірювання.
    // Process the first available measurement immediately.
    if (sensors.update()) {
        doMeasurement();
    }

    Serial.println("System Ready! Press BTN1 on D10 to calibrate (tare) the magnetometer.");
}

/*
 * ============================================================
//  LOOP — не змінюй
//  LOOP — do not change
 * ============================================================
 */
void loop() {
    // Нове вимірювання обробляємо після сигналу DREADY від сенсора.
    // Process a new measurement after the sensor asserts DREADY.
    if (sensors.updateIfReady()) {
        doMeasurement();
    }
}