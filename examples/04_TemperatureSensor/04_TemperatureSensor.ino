/*
 * ============================================================
 *  AmperiaSTEM — TEMPERATURE SENSOR
 * 
 *  Зчитуємо температуру з DHT11 і передаємо її на Amperia 
 *  Multimeter через EdPro Amperia Shield.
 *
 *  Read temperature from a DHT11 and expose it to the Amperia
 *  Multimeter through the EdPro Amperia Shield.
 *
 *  Потрібні бібліотеки:
 *    - DHT sensor library
 *  Required library:
 *    - DHT sensor library
 *
 *  Рекомендоване підключення:
 *    DHT VCC -> 5V (або 3.3V)
 *    DHT GND -> GND
 *    DHT DATA -> A1
 *  Recommended wiring:
 *    DHT VCC -> 5V (or 3.3V)
 *    DHT GND -> GND
 *    DHT DATA -> A1
 *
 *  Пін можна змінити, якщо A1 зайнятий іншим пристроєм.
 *  The data pin can be changed if A1 is already in use.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - DHT11 sensor
 *  - Amperia Multimeter (підключений до роз'єму MULTIMETER)
 *  Equipment used in this example:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - DHT11 sensor
 *  - Amperia Multimeter connected to the MULTIMETER connector
 * ============================================================
 */
#include <AmperiaSTEM.h>
#include <DHT.h>

constexpr uint8_t temperatureSensorPin = A1;
constexpr uint32_t measurementPeriodMs = 1500;

#define DHTTYPE DHT11
DHT dht(temperatureSensorPin, DHTTYPE);

SimpleSensor amperiaTemperature(
    "Thermometer",
    "Temperature",
    "C"
);


void setup()
{
    Serial.begin(115200);
    delay(1000);

    dht.begin();
    amperiaTemperature.setDataPeriodMs(measurementPeriodMs);

    if (!amperiaTemperature.begin()) {
        Serial.println("Failed to start the Amperia sensor interface.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("Amperia DHT temperature sensor is ready.");
}

void loop()
{
    const float temperatureC = dht.readTemperature();

    if (isnan(temperatureC)) {
        Serial.println("DHT is not connected or did not respond.");
        delay(measurementPeriodMs);
        return;
    }

    amperiaTemperature.publish(temperatureC);

    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" C");

    delay(measurementPeriodMs);
}