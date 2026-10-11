/*
 * ============================================================
 *  AmperiaSTEM — COMBINED MPU6050
 *
 *  CombinedSensor дозволяє одному пристрою містити логічні
 *  сенсори різної розмірності. Тут передаємо на мультиметр
 *  вектор прискорення X/Y/Z і його модуль.
 *  CombinedSensor lets one device expose logical sensors with different
 *  dimensions. This example publishes the X/Y/Z acceleration vector and
 *  its magnitude.
 * ============================================================
 */

#include <AmperiaSTEM.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <math.h>

constexpr uint16_t measurementPeriodMs = 100;
constexpr float accelerationScale = 16384.0f;

MPU6050 imu;

CombinedSensor measurements("MPU6050");

int acceleration = -1;
int magnitude = -1;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Wire.begin();
    imu.initialize();

    // Один логічний сенсор має три однорідні компоненти X/Y/Z.
    // One logical sensor contains the three X/Y/Z components.
    acceleration =
        measurements.addSensor("Acceleration", "g", "X", "Y", "Z");

    // Другий логічний сенсор є звичайним одновимірним значенням.
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
