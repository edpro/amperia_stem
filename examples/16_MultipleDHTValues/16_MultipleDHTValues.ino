/*
 * ============================================================
 *  AmperiaSTEM — MULTIPLE DHT VALUES
 *
 *  DHT11 вимірює дві різні фізичні величини. NSensors дозволяє
 *  передати їх на мультиметр як два окремі логічні сенсори.
 *  DHT11 measures two different physical quantities. NSensors exposes
 *  them to the multimeter as two separate logical sensors.
 * ============================================================
 */

#include <AmperiaSTEM.h>
#include <DHT.h>

constexpr uint8_t dhtDataPin = A1;
constexpr uint16_t measurementPeriodMs = 2000;

DHT dht(dhtDataPin, DHT11);

NSensors measurements("DHT11");

int temperature = -1;
int humidity = -1;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    dht.begin();

    // addSensor() повертає handle. Його використовуємо пізніше в set().
    // addSensor() returns a handle used later by set().
    temperature = measurements.addSensor("Temperature", "C");
    humidity = measurements.addSensor("Humidity", "%");

    if (temperature < 0 || humidity < 0) {
        Serial.println("Failed to add logical sensors.");
        while (true) {
            delay(1000);
        }
    }

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
