/*
 * ============================================================
 *  Example 14 — Simple Servo usage
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmpServo servo(2); // Change the digit to 1 or 2 to select servo1 or servo2

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Servo Example");

    servo.begin();
    Serial.println("Servo initialized.");
}

void loop() {
    Serial.println("Move servo2 to 0");
    servo.setAngle(0);
    delay(1000);

    Serial.println("Move servo2 to 90");
    servo.setAngle(90);
    delay(1000);

    Serial.println("Move servo2 to 180");
    servo.setAngle(180);
    delay(1000);

    Serial.println("Sweep servo2");
    for (int a = 0; a <= 180; a += 10) { servo.setAngle(a); delay(50); }
    for (int a = 180; a >= 0; a -= 10) { servo.setAngle(a); delay(50); }

    delay(1000);
}
