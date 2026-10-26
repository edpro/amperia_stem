/*
 * ============================================================
 *  AmperiaSTEM — IF / ELSE CONDITION
 *
 *  Читаємо рівень CO2 з EdPro Sensor і порівнюємо його
 *  з порогом, щоб увімкнути або вимкнути LED.
 *
 *  Read the CO2 level from the EdPro Sensor and compare it with a
 *  threshold to turn the LED on or off.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro CO2 Sensor (підключений до EdPro Amperia Shield через
 *    роз'єм SENSOR)
 *  - LED: один контакт до A0, інший до GND
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro CO2 Sensor connected to the shield's SENSOR connector
 *  - LED: one lead to A0, the other to GND
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

constexpr uint8_t ledPin = A0;

AmperiaSensorClient sensors;

AmperiaBridge display(sensors);

float getCO2()
{
    auto* sensor = sensors.findSensor("CO2");

    if (sensor == nullptr) {
        return -1000.0f; // Сигнальне значення: від'ємний рівень CO2 неможливий

                         // Sentinel value: a negative CO2 level is impossible
    }

    return sensor->value();
}

bool doMeasurement()
{
    float co2 = getCO2();

    // Встановлюємо порогове значення сигналу.

    // Set the alarm threshold.
    const int threshold = 2000;

    // Порівнюємо показання з порогом і керуємо LED.

    // Compare the reading with the threshold and control the LED.
    if (co2 > threshold) {
        digitalWrite(ledPin, HIGH); // Увімкнути LED
                                    // Turn the LED on
    } else {
        digitalWrite(ledPin, LOW);  // Вимкнути LED
                                    // Turn the LED off
    }

    Serial.print("CO2: ");
    Serial.print(co2);
    Serial.print(" ppm | Threshold: ");
    Serial.print(threshold);
    Serial.println(" ppm");

    display.publish();
    return true;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    // Налаштовуємо вивід для LED.

    // Configure the LED pin.
    pinMode(ledPin, OUTPUT);

    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    delay(5000);

    if (!sensors.discover() || sensors.findSensor("CO2") == nullptr) {
        Serial.println("CO2 sensor was not detected.");
        return;
    }

    if (!display.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    if (display.update()) {
        doMeasurement();
    }

    Serial.println("System Ready!");
}

void loop()
{
    if (display.updateIfReady()) {
        doMeasurement();
    }
}