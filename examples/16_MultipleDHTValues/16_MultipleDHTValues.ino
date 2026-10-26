/*
 * ============================================================
 *  AmperiaSTEM — MULTIPLE DHT VALUES
 *
 *  Один DHT11 вимірює дві різні фізичні величини. NSensors дозволяє
 *  передати їх на мультиметр як два окремі логічні сенсори.
 *
 *  One DHT11 measures two different physical values. NSensors exposes
 *  them to the multimeter as two separate logical sensors.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - DHT11 (підключений до A1)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - DHT11 connected to A1
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>
#include <DHT.h>

constexpr uint8_t dhtDataPin = A1;
constexpr uint16_t measurementPeriodMs = 2000;

DHT dht(dhtDataPin, DHT11);

NSensors measurements("DHT11"); // Об'єкт для двох логічних сенсорів

                                 // Object exposing two logical sensors

int temperature = -1;
int humidity = -1;

void setup()
{
    Serial.begin(115200);

    delay(1000);

    dht.begin();

    // addSensor() повертає handle, який далі використовується в set().

    // addSensor() returns a handle used later by set().
    temperature = measurements.addSensor("Temperature", "C");
    humidity = measurements.addSensor("Humidity", "%");

    if (temperature < 0 || humidity < 0) {
        Serial.println("Failed to add logical sensors.");
        while (true) {
            delay(1000);
        }
    }

    // Налаштовуємо однаковий період оновлення для обох сенсорів.

    // Configure the same update period for both sensors.
    if (!measurements.setDataPeriodMs(temperature, measurementPeriodMs) ||
        !measurements.setDataPeriodMs(humidity, measurementPeriodMs)) {
        Serial.println("Failed to configure sensor periods.");
        while (true) {
            delay(1000);
        }
    }

    // begin() автоматично завершує конфігурацію сенсорів.

    // begin() finalizes the sensor configuration automatically.
    if (!measurements.begin()) {
        Serial.println("Failed to start the Amperia sensor interface.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("DHT11 multi-sensor output is ready.");
}

void loop()
{
    const float temperatureC = dht.readTemperature();
    const float humidityPercent = dht.readHumidity();

    if (isnan(temperatureC) || isnan(humidityPercent)) {
        Serial.println("DHT11 is not connected or did not respond.");
        delay(measurementPeriodMs);
        return;
    }

    // Оновлюємо обидва логічні сенсори з новими значеннями.

    // Update both logical sensors with fresh values.
    if (!measurements.set(temperature, temperatureC) ||
        !measurements.set(humidity, humidityPercent)) {
        Serial.println("Failed to update sensor values.");
        delay(measurementPeriodMs);
        return;
    }

    measurements.publish();

    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print(" C | Humidity: ");
    Serial.print(humidityPercent);
    Serial.println(" %");

    delay(measurementPeriodMs);
}
