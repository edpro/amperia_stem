/*
 * ============================================================
 *  AmperiaSTEM — STEPPER MOTOR USAGE
 *
 *  Показуємо, як керувати кроковим двигуном: поступальний хід,
 *  зворотній хід, поворот на заданий кут і поворот назад.
 *
 *  Demonstrates how to control a stepper motor: move forward,
 *  move backward, rotate by a specified angle, and rotate back.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - Кроковий двигун, підключений до стандартних крокових пінів
 *    на EdPro Amperia Shield
 *  - Amperia Multimeter (опційно, для візуалізації стану)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - Stepper motor connected to the default stepper pins on the shield
 *  - Amperia Multimeter (optional, for visualizing status)
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Ініціалізуємо кроковий двигун.

// Initialize the stepper motor.
AmpStepper stepper;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Stepper Example — using default shield pins");

    // Встановлюємо початкову швидкість у обертах за хвилину.

    // Set the initial speed in RPM.
    stepper.setSpeed(60);
}

void loop() {
    // Крок вперед на 200 кроків.

    // Step 200 forward.
    Serial.println("Step 200 forward");
    stepper.step(200);
    delay(1000);

    // Крок назад на 200 кроків.

    // Step 200 backward.
    Serial.println("Step 200 backward");
    stepper.step(-200);
    delay(1000);

    // Поворот на 90°.

    // Rotate 90 degrees.
    Serial.println("Rotate 90 degrees");
    stepper.stepDegrees(90.0);
    delay(1000);

    // Поворот назад на 90°.

    // Rotate -90 degrees.
    Serial.println("Rotate -90 degrees");
    stepper.stepDegrees(-90.0);
    delay(2000);
}
