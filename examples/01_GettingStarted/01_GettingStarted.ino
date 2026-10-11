/*
 * ============================================================
 *  AmperiaSTEM — GETTING STARTED
 * 
 *  Читаємо значення з високоточного сенсора температури PT100 та 
 *  відправляємо на Amperia MULTIMETER.
 *
 *  Read the value from the PT100 precision temperature sensor and
 *  forward it to the Amperia Multimeter.
 * 
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro Sensor (підключений до EdPro Amperia Shield через 
 *  роз'єм SENSOR)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через 
 *  роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro Sensor connected to the shield's SENSOR connector
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
*/

#include <AmperiaSTEM.h>

/*
 * ============================================================
 * ОГОЛОШЕННЯ ОБ'ЄКТІВ
 * OBJECT DECLARATIONS
 * 
 * Об'єкти для роботи з сенсорами та мультиметром.
 * Objects used to read the sensor and forward data to the multimeter.
 * ============================================================
 */
AmperiaSensorClient sensors; // Об'єкт для читання сенсорів Amperia STEM
                             // Reads sensors connected through Amperia STEM

AmperiaBridge display(sensors); // Передає прочитані дані на мультиметр Amperia
                                // Forwards measurements to the Amperia Multimeter

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * HELPER FUNCTIONS
 * ============================================================
 */
float getPreciseTemperature()
{
    auto* sensor = sensors.findSensor("PT100");

    if (sensor == nullptr) {
        return -1000.0f; // Значення поза робочим діапазоном PT100
                         // Sentinel value outside the PT100 operating range
    }

    return sensor->value();
}

/*
 * ============================================================
 * ЛОГІКА ВИМІРЮВАНЬ
 * MEASUREMENT LOGIC
 * 
 * Функція, яка викликається з loop(), коли є нові дані від EdPro Sensor.
 * Called from loop() when new data is available from the EdPro Sensor.
 * ============================================================
 */
bool doMeasurement() 
{
    // Отримуємо значення з високоточного сенсора температури PT100.
    // Read the value from the PT100 precision temperature sensor.
    float preciseTemp = getPreciseTemperature();

    // Виводимо значення у Serial Monitor для зручності.
    // Print the value to Serial Monitor for diagnostics.
    Serial.print("Precise Temperature: "); 
    Serial.println(preciseTemp);

    // Надсилаємо прочитані значення на екран мультиметра Amperia.
    // Publish the current values to the Amperia Multimeter.
    display.publish();

    return true;
}

/*
 * ============================================================
 *  SETUP
 * 
 * Функція виконується один раз при старті програми. 
 * Тут відбувається ініціалізація сенсорів та мультиметра. 
 * Runs once at startup and initializes the sensor and multimeter output.
 * ============================================================
 */
void setup() {
    Serial.begin(115200); // Ініціалізація Serial Monitor для виводу даних
                          // Initialize Serial Monitor output
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    // Ініціалізуємо інтерфейс для читання зовнішнього сенсора.
    // Initialize the interface used to read the external sensor.
    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    // Даємо зовнішньому сенсору час завершити запуск.
    // Give the external sensor time to finish startup.
    delay(5000);

    // Зчитуємо опис підключених логічних сенсорів.
    // Discover the logical sensors reported by the external device.
    if (!sensors.discover() || sensors.findSensor("PT100") == nullptr) {
        Serial.println("PT100 sensor was not detected.");
        return;
    }

    // Налаштовуємо передавання прочитаних сенсорів на мультиметр.
    // Configure forwarding of the discovered sensors to the multimeter.
    if (!display.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    Serial.println("System Ready!");

    // Одразу читаємо і показуємо перше доступне вимірювання.
    // Read and publish the first available measurement immediately.
    if (display.update()) {
        doMeasurement();
    }
}

/*
 * ============================================================
 *  LOOP
 * 
 * Функція виконується циклічно після завершення setup().
 * Тут відбувається перевірка наявності нових даних від EdPro 
 * Sensor та виклик функції doMeasurement() для обробки цих даних.
 *
 * Runs repeatedly after setup(), checks for new EdPro Sensor data,
 * and calls doMeasurement() to process it.
 * ============================================================
 */
void loop() {
    if (display.updateIfReady()) {
        doMeasurement();
    }
}