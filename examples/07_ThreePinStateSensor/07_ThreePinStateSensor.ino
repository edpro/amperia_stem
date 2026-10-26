/*
 * ============================================================
 *  AmperiaSTEM — 3-PIN STATE SENSOR
 *
 *  Цей приклад перевіряє стан трьох логічних входів і передає
 *  їхні значення на Amperia Multimeter як три незалежні величини.
 *
 *  This example reads the state of three digital/analog input pins and
 *  publishes them to the Amperia Multimeter as three independent values.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - три логічні входи, підключені до джерела сигналу або кнопок
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - three logic inputs connected to a signal source or buttons
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

constexpr uint16_t measurementPeriodMs = 100;

// Створюємо сенсор з трьома каналами логічного стану.

// Create a sensor with three logical state channels.
NDimSensor pinStateSensor(
    "3-pin tester",
    "Inputs",
    "logic",
    3
);

// Читаємо стан окремого піну: HIGH = 1, LOW = 0.

// Read the state of one pin: HIGH = 1, LOW = 0.
int32_t readLogicLevel(uint8_t pin)
{
    if (digitalRead(pin) == HIGH) {
        return 1;
    }

    return 0;
}

void setup()
{
    pinMode(2, INPUT_PULLUP);
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);

    pinStateSensor.setDataPeriodMs(measurementPeriodMs);
    if (!pinStateSensor.begin()) {
        while (true) {
            delay(1000);
        }
    }
}

void loop()
{
    // Передаємо на мультиметр стан кожного входу окремо.

    // Publish the state of each input as a separate value.
    pinStateSensor.set(0, readLogicLevel(2));
    pinStateSensor.set(1, readLogicLevel(A0));
    pinStateSensor.set(2, readLogicLevel(A1));
    pinStateSensor.publish();

    delay(measurementPeriodMs);
}
