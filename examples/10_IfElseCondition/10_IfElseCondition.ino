/*
 * ============================================================
 *  AmperiaSTEM — 10_IfElseCondition
 *  Логіка умов: читаємо рівень CO2 з EdPro і порівнюємо
 *  із заданим порогом, щоб увімкнути LED.
 *  Conditional logic: read the CO2 level from an EdPro Sensor and
 *  compare it with a threshold to control an LED.
 * ============================================================
 *
 *  Підключення:
 *    - EdPro CO2 Sensor
 *    - Світлодіод: один контакт до A0, інший до GND.
 *  Connections:
 *    - EdPro CO2 Sensor
 *    - LED: one lead to A0, the other to GND
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Піни для Arduino
// Pins used by this example
constexpr uint8_t ledPin = A0;

/*
 * ============================================================
 * ОГОЛОШЕННЯ ОБ'ЄКТІВ
 * OBJECT DECLARATIONS
 * ============================================================
 */
AmperiaSensorClient sensors;             // читання сенсорів з EdPro Sensor
                                         // Reads sensors from the EdPro Sensor

AmperiaBridge display(sensors);          // передавання даних на мультиметр Amperia
                                         // Forwards data to the Amperia Multimeter

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * HELPER FUNCTIONS
 *
 * Конкретика CO2-сенсора залишається в цьому прикладі, а не
 * в універсальному класі бібліотеки.
 * The concrete CO2 sensor name belongs to this example rather than
 * the generic library client.
 * ============================================================
 */
float getCO2()
{
    auto* sensor = sensors.findSensor("CO2");

    if (sensor == nullptr) {
        return -1000.0f; // Від'ємний рівень CO2 є неможливим
                         // Sentinel value: a negative CO2 level is impossible
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
    // Отримуємо значення CO2 з сенсора EdPro.
    // Read the CO2 value from the EdPro Sensor.
    float co2 = getCO2();

    // Виставляємо порогове значення "тривоги".
    // Set the alarm threshold.
    int threshold = 2000;

    // Логіка умов (Threshold)
    // Compare the measurement with the threshold
    if (co2 > threshold) {
        digitalWrite(ledPin, HIGH);  // Увімкнути тривогу (світлодіод)
                                     // Turn the alarm LED on
    } else {
        digitalWrite(ledPin, LOW);   // Вимкнути тривогу
                                     // Turn the alarm LED off
    }

    // Виводимо дані в Serial Monitor для зручності відлагодження.
    // Print the values to Serial Monitor for diagnostics.
    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm | Threshold: ");
    Serial.print(threshold);
    Serial.println(" ppm");

    // Надсилаємо прочитані значення на екран мультиметра Amperia.
    // Publish the current values to the Amperia Multimeter.
    display.publish();

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

    // Налаштування пінів
    // Configure the LED pin
    pinMode(ledPin, OUTPUT);

    // Ініціалізуємо читання зовнішнього сенсора.
    // Initialize the external-sensor interface.
    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    // Даємо зовнішньому сенсору час завершити запуск.
    // Give the external sensor time to finish startup.
    delay(5000);

    // Зчитуємо опис сенсорів і перевіряємо наявність CO2.
    // Discover sensor metadata and verify that CO2 is available.
    if (!sensors.discover() || sensors.findSensor("CO2") == nullptr) {
        Serial.println("CO2 sensor was not detected.");
        return;
    }

    // Реєструємо прочитані сенсори для виводу на мультиметр.
    // Configure forwarding of the discovered sensors to the multimeter.
    if (!display.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    // Одразу опрацьовуємо перше доступне вимірювання.
    // Process the first available measurement immediately.
    if (display.update()) {
        doMeasurement();
    }

    Serial.println("System Ready!");
}

/*
 * ============================================================
//  LOOP — не змінюй
//  LOOP — do not change
 * ============================================================
 */
void loop() {
    // Обробляємо вимірювання лише тоді, коли сенсор повідомив про нові дані.
    // Process a measurement only after the sensor reports new data.
    if (display.updateIfReady()) {
        doMeasurement();
    }
}