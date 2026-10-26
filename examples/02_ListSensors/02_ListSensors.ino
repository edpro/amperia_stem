/*
 * ============================================================
 *  AmperiaSTEM — LIST SENSORS
 *
 *  Отримуємо перелік датчиків, доступних через EdPro Sensor:
 *  назву, одиниці виміру та кількість компонентів.
 *
 *  List the logical sensors reported by an EdPro Sensor, including
 *  their names, units, and component counts.
 *
 *  Розширений варіант з усіма метаданими див. у
 *  02_ListSensorsAdvanced.
 *  See 02_ListSensorsAdvanced for the complete metadata view.
 *
 *  Обладнання для проведення експерименту:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - EdPro Sensor (підключений до роз'єму SENSOR)
 *  - Amperia Multimeter (підключений до роз'єму MULTIMETER)
 *
 *  Equipment used in this example:
 *  - Arduino Uno R4 WiFi
 *  - EdPro Amperia Shield
 *  - EdPro Sensor connected to the SENSOR connector
 *  - Amperia Multimeter connected to the MULTIMETER connector
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmperiaSensorClient sensorClient;

void printSensorList()
{
    Serial.print("Device: ");
    Serial.println(sensorClient.deviceName());

    for (size_t i = 0; i < sensorClient.sensorCount(); ++i) {
        auto& sensor = sensorClient.sensor(i);

        Serial.print(i);
        Serial.print(": ");
        Serial.print(sensor.name());
        Serial.print(" | unit: ");
        Serial.print(sensor.unit());
        Serial.print(" | components: ");
        Serial.println(sensor.dimension());
    }
}
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("AmperiaSTEM sensor list");

    if (!sensorClient.begin()) {
        Serial.println("Failed to initialize the external-sensor client.");
        return;
    }

    // Деяким зовнішнім датчикам EdPro потрібен час, щоб завершити власний запуск.

    // Some external EdPro sensors need time to finish their own startup.
    delay(5000);

    if (!sensorClient.discover()) {
        Serial.println("Failed to read the sensor list.");
        return;
    }

    if (sensorClient.sensorCount() == 0) {
        Serial.println("No logical sensors were reported.");
        return;
    }

    printSensorList();
}

void loop()
{
    // Метадані виявляються після setup(). Оновлення update() або
    // updateIfReady(), коли потрібні значення вимірювань.

    // Metadata is discovered once in setup(). Call update() or
    // updateIfReady() when measurement values are needed.
}
