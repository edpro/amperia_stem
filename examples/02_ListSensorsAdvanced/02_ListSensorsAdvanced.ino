/*
 * ============================================================
 *  AmperiaSTEM — ADVANCED SENSOR LIST
 *
 *  Розширений приклад перегляду метаданих зовнішнього EdPro Sensor.
 *  Для першого знайомства див. простіший 02_ListSensors.
 *
 *  Extended example for inspecting metadata reported by an external
 *  EdPro Sensor. Start with 02_ListSensors for a simpler introduction.
 * ============================================================
 */

#include <AmperiaSTEM.h>

AmperiaSensorClient sensorClient;

void printSensorDetails()
{
    Serial.print("Device: ");
    Serial.println(sensorClient.deviceName());

    Serial.print("Hardware version: ");
    Serial.println(sensorClient.hardwareVersion());
    Serial.print("Firmware version: ");
    Serial.println(sensorClient.firmwareVersion());
    Serial.print("Protocol version: ");
    Serial.println(sensorClient.protocolVersion());
    Serial.print("Operation mode: ");
    Serial.println(sensorClient.operationMode());

    Serial.print("Reported sensors: ");
    Serial.println(sensorClient.reportedSensorCount());
    Serial.print("Available sensors: ");
    Serial.println(sensorClient.sensorCount());

    if (sensorClient.sensorListTruncated()) {
        Serial.print("Warning: only the first ");
        Serial.print(amperia::protocol::kMaxLogicalSensors);
        Serial.println(" logical sensors are supported.");
    }

    for (size_t i = 0; i < sensorClient.sensorCount(); ++i) {
        auto& sensor = sensorClient.sensor(i);

        Serial.println();
        Serial.print("Sensor ");
        Serial.println(i);
        Serial.print("  name: ");
        Serial.println(sensor.name());
        Serial.print("  unit: ");
        Serial.println(sensor.unit());
        Serial.print("  components: ");
        Serial.println(sensor.dimension());
        Serial.print("  period: ");
        Serial.print(sensor.dataPeriodMs());
        Serial.println(" ms");
        Serial.print("  value type: ");
        Serial.println(static_cast<unsigned int>(sensor.valueType()));
        Serial.print("  resolution: ");
        Serial.println(sensor.valueResolution(), 6);
        Serial.print("  minimum: ");
        Serial.println(sensor.valueMin(), 6);
        Serial.print("  maximum: ");
        Serial.println(sensor.valueMax(), 6);
        Serial.print("  flags: 0x");
        Serial.println(sensor.flags(), HEX);

        if (sensor.usesDiscouragedDimension()) {
            Serial.println(
                "  Note: dimension 4 is supported, but the current "
                "multimeter UI displays it poorly.");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    if (!sensorClient.begin()) {
        Serial.println("Failed to initialize the external-sensor client.");
        return;
    }

    delay(5000);

    if (!sensorClient.discover()) {
        Serial.print("Discovery failed while reading register 0x");
        Serial.println(sensorClient.failedRegister(), HEX);
        return;
    }

    printSensorDetails();
}

void loop()
{
}
