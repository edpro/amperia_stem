/*
 * ============================================================
 *  Example 13 — Simple DC motor usage
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmpDC motor;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("DC Motor Example");

    motor.begin();
    Serial.println("Motor initialized");
}

void loop() {
    // Крутити вперед на повній швидкості.
    // Run forward.
    Serial.println("Forward");
    motor.setSpeed(200); // 0..255
    delay(2000);

    // Крутити назад.
    // Run in reverse.
    Serial.println("Reverse");
    motor.setSpeed(-200);
    delay(2000);

    // Стоп.
    // Stop.
    Serial.println("Stop");
    motor.stop();
    delay(2000);
}
