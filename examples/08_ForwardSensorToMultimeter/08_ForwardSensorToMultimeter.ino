/*
 * ============================================================
 *  AmperiaSTEM — FORWARD SENSOR TO MULTIMETER
 *
 *  Приймаємо показання від зовнішнього сенсора і
 *  безпосередньо пересилаємо їх на Amperia Multimeter.
 *
 *  Receive readings from an external sensor and forward them
 *  directly to the Amperia Multimeter.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino UNO R4 WiFi і EdPro Amperia Shield (зібрані разом)
 *  - EdPro Sensor (підключений до EdPro Amperia Shield через
 *    роз'єм SENSOR)
 *  - Amperia Multimeter (підключений до EdPro Amperia Shield через
 *    роз'єм MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino UNO R4 WiFi with EdPro Amperia Shield
 *  - EdPro Sensor connected to the shield's SENSOR connector
 *  - Amperia Multimeter connected to the shield's MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmperiaSensorClient sourceSensor;
AmperiaBridge bridge(sourceSensor);

void setup()
{
    Serial.begin(115200);

    if (!sourceSensor.begin()) {
        Serial.println("Failed to start the external sensor.");
        return;
    }

    delay(5000);

    // Зчитуємо опис доступних сенсорів і перевіряємо, чи пристрій надіслав дані.

    // Discover the available logical sensors and confirm the device is present.
    if (!sourceSensor.discover() || sourceSensor.sensorCount() == 0) {
        Serial.println("External sensor was not detected.");
        return;
    }

    if (!bridge.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    if (bridge.update()) {
        bridge.publish();
    }
}

void loop()
{
    // Якщо дані готові, передаємо їх на мультиметр.

    // If fresh data is ready, forward it to the multimeter.
    if (bridge.updateIfReady()) {
        bridge.publish();
    }
}
