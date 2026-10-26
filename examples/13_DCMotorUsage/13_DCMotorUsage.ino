/*
 * ============================================================
 *  AmperiaSTEM — DC MOTOR USAGE
 *
 *  Показуємо, як крутити DC-двигун вперед, назад і зупиняти його.
 *  Demonstrates how to run a DC motor forward, reverse, and stop it.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - DC-двигун, підключений через канал DC MOTOR на EdPro Amperia Shield
 *  - Amperia Multimeter (опційно, для візуалізації стану)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - DC motor connected through the DC MOTOR output on the Amperia Shield
 *  - Amperia Multimeter (optional, for visualizing output)
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmpDC motor; // Об'єкт для керування DC-двигуном

             // Object used to control the DC motor

void setup() {
    Serial.begin(115200);

    delay(500);

    Serial.println("DC Motor Example");

    // Ініціалізуємо двигун.

    // Initialize the motor.
    motor.begin();
    Serial.println("Motor initialized");
}

void loop() {
    // Крутити вперед на повній швидкості.

    // Run forward at full speed.
    Serial.println("Forward");
    motor.setSpeed(200); // 0..255
    delay(2000);

    // Крутити назад.

    // Run in reverse.
    Serial.println("Reverse");
    motor.setSpeed(-200);
    delay(2000);

    // Зупинити двигун.

    // Stop the motor.
    Serial.println("Stop");
    motor.stop();
    delay(2000);
}
