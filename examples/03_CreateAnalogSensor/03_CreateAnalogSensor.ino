/*
 * ============================================================
 *  AmperiaSTEM — CREATE ANALOG SENSOR
 *
 *  Під'єднуємо датчик з аналоговим виходом до A0. Arduino читає
 *  сигнал і передає його в Amperia Multimeter через EdPro Amperia
 *  Shield за допомогою SimpleSensor.
 *
 *  Connect a sensor with an analog output to A0. Arduino reads the
 *  signal and exposes it to the Amperia Multimeter through SimpleSensor.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - Сенсор з аналоговим виходом
 *  - Amperia Multimeter (підключений до роз'єму MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - Sensor with an analog output
 *  - Amperia Multimeter connected to the MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

constexpr uint8_t sensorPin = A0;
constexpr uint32_t measurementPeriodMs = 160;
constexpr float adcMaximum = 4095.0f;

SimpleSensor analogSensor(
    "Student sensor",
    "Analog level",
    "%"
);


void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(sensorPin, INPUT);
    analogReadResolution(12);
    analogSensor.setDataPeriodMs(measurementPeriodMs);

    if (!analogSensor.begin()) {
        Serial.println("Failed to start the Amperia sensor interface.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("Amperia analog sensor is ready.");
}

void loop()
{
    const int rawValue = analogRead(sensorPin);
    const float levelPercent = 100.0f * rawValue / adcMaximum;

    analogSensor.publish(levelPercent);

    Serial.print("Analog level: ");
    Serial.print(levelPercent);
    Serial.println(" %");

    delay(measurementPeriodMs);
}
