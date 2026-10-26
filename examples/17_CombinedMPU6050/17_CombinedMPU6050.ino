/*
 * ============================================================
 *  AmperiaSTEM — COMBINED MPU6050
 *
 *  CombinedSensor дозволяє одному пристрою містити логічні
 *  сенсори різної розмірності. Тут передаємо на мультиметр
 *  вектор прискорення X/Y/Z і його модуль.
 *
 *  CombinedSensor lets one device expose logical sensors with different
 *  dimensions. This example publishes the X/Y/Z acceleration vector and
 *  its magnitude.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - MPU6050 (підключений через I2C до Arduino)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - MPU6050 connected via I2C to the Arduino
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Підключаємо I2C-інтерфейс для обміну даними з MPU6050.

// Connect the I2C interface used to communicate with the MPU6050.
#include <Wire.h>

// Допоміжна бібліотека для роботи з I2C- пристроями.

// Helper library for working with I2C-based devices.
#include <I2Cdev.h>

// Основна бібліотека MPU6050 для читання акселерометра та гіроскопа.

// Main MPU6050 library for reading accelerometer and gyroscope data.
#include <MPU6050.h>

// Бібліотека математики для sqrt(), яка потрібна для обчислення модуля вектора.

// Math library for sqrt(), used to calculate the vector magnitude.
#include <math.h>

constexpr uint16_t measurementPeriodMs = 100;
constexpr float accelerationScale = 16384.0f;

MPU6050 imu;

CombinedSensor measurements("MPU6050"); // Об'єкт для комбінованого сенсора

                                         // Object used to expose combined sensor data

int acceleration = -1;
int magnitude = -1;

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Wire.begin();
    imu.initialize();

    // Один логічний сенсор має три компоненти X/Y/Z.

    // One logical sensor contains the three X/Y/Z components.
    acceleration =
        measurements.addSensor("Acceleration", "g", "X", "Y", "Z");

    // Другий логічний сенсор є скалярним значенням.

    // The second logical sensor is a scalar value.
    magnitude = measurements.addSensor("Magnitude", "g");

    if (acceleration < 0 || magnitude < 0) {
        Serial.println("Failed to add logical sensors.");
        while (true) {
            delay(1000);
        }
    }

    if (!measurements.setDataPeriodMs(acceleration, measurementPeriodMs) ||
        !measurements.setDataPeriodMs(magnitude, measurementPeriodMs)) {
        Serial.println("Failed to configure sensor periods.");
        while (true) {
            delay(1000);
        }
    }

    if (!measurements.begin()) {
        Serial.println("Failed to start the Amperia sensor interface.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("MPU6050 combined-sensor output is ready.");
}

void loop()
{
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    imu.getAcceleration(&ax, &ay, &az);

    const float accelX = ax / accelerationScale;
    const float accelY = ay / accelerationScale;
    const float accelZ = az / accelerationScale;
    const float accelMagnitude =
        sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

    // Оновлюємо складений вектор X/Y/Z та його модуль.

    // Update the combined vector X/Y/Z and its magnitude.
    if (!measurements.set(acceleration, 0, accelX) ||
        !measurements.set(acceleration, 1, accelY) ||
        !measurements.set(acceleration, 2, accelZ) ||
        !measurements.set(magnitude, accelMagnitude)) {
        Serial.println("Failed to update sensor values.");
        delay(measurementPeriodMs);
        return;
    }

    measurements.publish();

    Serial.print("Acceleration [g] -> X: ");
    Serial.print(accelX);
    Serial.print(" | Y: ");
    Serial.print(accelY);
    Serial.print(" | Z: ");
    Serial.print(accelZ);
    Serial.print(" | Magnitude: ");
    Serial.println(accelMagnitude);

    delay(measurementPeriodMs);
}
