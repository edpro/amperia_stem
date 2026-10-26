/*
 * ============================================================
 *  AmperiaSTEM — VECTOR CALIBRATION
 *
 *  Читаємо магнітометр, зберігаємо поточне значення як "нульовий"
 *  рівень за допомогою кнопки BTN1 на D10 і передаємо
 *  відкалібровані дані на Amperia Multimeter.
 *
 *  Read the magnetometer, store the current value as a zero reference
 *  with the BTN1 button on D10, and forward the calibrated data to the
 *  Amperia Multimeter.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro Magnetometer (підключений до EdPro Amperia Shield через
 *    роз'єм SENSOR)
 *  - Кнопка BTN1 на D10
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro Magnetometer connected to the shield's SENSOR connector
 *  - BTN1 button on D10
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>
#include <math.h> // Для математичних функцій (sqrt)

                  // Used for sqrt()

constexpr uint8_t buttonPin = 10;

/*
 * ============================================================
 * ГЛОБАЛЬНІ ЗМІННІ СТАНУ (State Variables)
 * 
 * GLOBAL STATE VARIABLES
 * 
 * Тут зберігаємо наш "нульовий рівень".
 * 
 * Stores the zero-reference values.
 * ============================================================
 */
float tareX = 0.0;
float tareY = 0.0;
float tareZ = 0.0;

AmperiaSensorClient sensors;

NDimSensor calibratedMag(
    "Calib. Magnet",
    "Axis",
    "mkT",
    4
);

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * 
 * HELPER FUNCTIONS
 *
 * Знаємо назву логічного сенсора саме тут, у прикладі роботи
 * з магнітометром. Універсальний клієнт про конкретні моделі
 * сенсорів нічого не знає.
 * 
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

    calibratedMag.publish(diffX, diffY, diffZ, diffMag);

.
    Serial.print("Відносне поле [mkT] -> X: "); Serial.print(diffX);
    Serial.print(" | Y: "); Serial.print(diffY);
    Serial.print(" | Z: "); Serial.print(diffZ);
    Serial.print(" | Магнітуда: "); Serial.println(diffMag);

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    pinMode(buttonPin, INPUT_PULLUP);

    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    delay(5000);

    if (!sensors.discover() || sensors.findSensor("VecXYZ") == nullptr) {
        Serial.println("Magnetometer was not detected.");
        return;
    }

    if (!calibratedMag.begin()) {
        Serial.println("Failed to start the custom NDimSensor.");
        return;
    }

    if (sensors.update()) {
        doMeasurement();
    }

    Serial.println("System Ready! Press BTN1 on D10 to calibrate (tare) the magnetometer.");
}

void loop() 
{
    if (sensors.updateIfReady()) {
        doMeasurement();
    }
}