/*
 * ============================================================
 *  Example 15 — Simple Stepper usage (uses AmpStepper)
 * ============================================================
 */

#include <AmperiaSTEM.h>

// Use default AmpStepper which uses the shield stepper pins
AmpStepper stepper;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("Stepper Example — using default shield pins");

    // Set initial speed (RPM)
    stepper.setSpeed(60);
}

void loop() {
    Serial.println("Step 200 forward");
    stepper.step(200);
    delay(1000);

    Serial.println("Step 200 backward");
    stepper.step(-200);
    delay(1000);

    Serial.println("Rotate 90 degrees");
    stepper.stepDegrees(90.0);
    delay(1000);

    Serial.println("Rotate -90 degrees");
    stepper.stepDegrees(-90.0);
    delay(2000);
}
