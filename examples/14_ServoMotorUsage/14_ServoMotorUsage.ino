/*
 * ============================================================
 *  AmperiaSTEM — SERVO MOTOR USAGE
 *
 *  Показуємо, як переміщувати сервомотор у задані кути та
 *  виконувати плавний розмах у межах 0..180°.
 *
 *  Demonstrates how to move a servo to specific angles and sweep it
 *  smoothly across the 0..180° range.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - Сервомотор, підключений до каналу SERVO на EdPro Amperia Shield
 *  - Amperia Multimeter (опційно, для візуалізації стану)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - Servo motor connected to the SERVO channel on the Amperia Shield
 *  - Amperia Multimeter (optional, for visualizing status)
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmpServo servo(2); // Змініть цифру на 1 або 2, щоб вибрати servo1 або servo2

                   // Change the digit to 1 or 2 to select servo1 or servo2

void setup() {
    Serial.begin(115200);

    delay(500);

    Serial.println("Servo Example");

    // Ініціалізуємо сервомотор.

    // Initialize the servo motor.
    servo.begin();
    Serial.println("Servo initialized.");
}

void loop() {
    // Переміщуємо сервомотор у кут 0°.

    // Move the servo to 0°.
    Serial.println("Move servo2 to 0");
    servo.setAngle(0);
    delay(1000);

    // Переміщуємо сервомотор у кут 90°.

    // Move the servo to 90°.
    Serial.println("Move servo2 to 90");
    servo.setAngle(90);
    delay(1000);

    // Переміщуємо сервомотор у кут 180°.

    // Move the servo to 180°.
    Serial.println("Move servo2 to 180");
    servo.setAngle(180);
    delay(1000);

    // Виконуємо плавний розмах в обидва боки.

    // Sweep the servo smoothly through the range.
    Serial.println("Sweep servo2");
    for (int a = 0; a <= 180; a += 10) { servo.setAngle(a); delay(50); }
    for (int a = 180; a >= 0; a -= 10) { servo.setAngle(a); delay(50); }

    delay(1000);
}
