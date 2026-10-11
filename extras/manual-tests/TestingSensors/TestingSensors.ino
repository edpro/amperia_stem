#include <AmperiaSTEM.h>
#include <Wire.h>

// ==============================================================================
// SENSOR SELECTION: Uncomment EXACTLY ONE of the sensors below to test it.
// ==============================================================================
// #define USE_BH1750
// #define USE_BME280
// #define USE_BMP280
// #define USE_MAX9814
// #define USE_KY037
// #define USE_DHT22
#define USE_MPU6050

// ==============================================================================
// SENSOR CONFIGURATIONS
// ==============================================================================
#if defined(USE_DHT22)
    #include <DHT.h>
    constexpr uint8_t localSensorPin = 12;
    DHT localSensorObj(localSensorPin, DHT22);
    constexpr uint32_t localMeasurementPeriodMs = 2000;
    #define SENSOR_ERROR_MSG "Failed to read from DHT22."

#elif defined(USE_BH1750)
    #include <BH1750.h>
    BH1750 localSensorObj;
    constexpr uint32_t localMeasurementPeriodMs = 200;
    #define SENSOR_ERROR_MSG "Failed to read from BH1750."

#elif defined(USE_BME280)
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BME280.h>
    Adafruit_BME280 localSensorObj;
    constexpr uint32_t localMeasurementPeriodMs = 1000;
    #define SENSOR_ERROR_MSG "Failed to read from BME280."

#elif defined(USE_BMP280)
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BMP280.h>
    Adafruit_BMP280 localSensorObj;
    constexpr uint32_t localMeasurementPeriodMs = 1000;
    #define SENSOR_ERROR_MSG "Failed to read from BMP280."

#elif defined(USE_MPU6050)
    #include <I2Cdev.h>
    #include <MPU6050.h>
    MPU6050 localSensorObj;
    constexpr uint32_t localMeasurementPeriodMs = 100;
    #define SENSOR_ERROR_MSG "Failed to read from MPU6050."

#elif defined(USE_MAX9814) || defined(USE_KY037)
    constexpr uint8_t localSensorPin = A0;
    constexpr uint32_t localMeasurementPeriodMs = 50;
    #if defined(USE_MAX9814)
        #define SENSOR_ERROR_MSG "Failed to read from MAX9814."
    #else
        #define SENSOR_ERROR_MSG "Failed to read from KY-037."
    #endif
#endif

// ==============================================================================
// CORE APPLICATION
// ==============================================================================
AmperiaSensorClient sourceSensor;
AmperiaBridge bridge(sourceSensor);

constexpr uint32_t sourceFallbackPollPeriodMs = 500;

uint32_t previousSourcePollMs = 0;
uint32_t previousLocalMeasurementMs = 0;

// BME280 needs the most local logical sensors: temperature, humidity,
// and pressure are different physical quantities with different units.
int localSensorOutputs[3] = {-1, -1, -1};

void printReadError(const char* operation)
{
    Serial.print(operation);
    Serial.print(" failed while reading register 0x");
    Serial.println(sourceSensor.failedRegister(), HEX);
}


bool configureLocalSensors()
{
    // One logical Amperia sensor has one unit. Therefore heterogeneous
    // environmental quantities are exposed as separate logical sensors.
#if defined(USE_DHT22)
    localSensorOutputs[0] = bridge.addSensor("DHT22 Hum", "%");
    return localSensorOutputs[0] >= 0;

#elif defined(USE_BH1750)
    localSensorOutputs[0] = bridge.addSensor("BH1750 Light", "lx");
    return localSensorOutputs[0] >= 0;

#elif defined(USE_BME280)
    localSensorOutputs[0] = bridge.addSensor("BME280 Temp", "C");
    localSensorOutputs[1] = bridge.addSensor("BME280 Hum", "%");
    localSensorOutputs[2] = bridge.addSensor("BME280 Press", "hPa");
    return localSensorOutputs[0] >= 0 &&
           localSensorOutputs[1] >= 0 &&
           localSensorOutputs[2] >= 0;

#elif defined(USE_BMP280)
    localSensorOutputs[0] = bridge.addSensor("BMP280 Temp", "C");
    localSensorOutputs[1] = bridge.addSensor("BMP280 Press", "hPa");
    return localSensorOutputs[0] >= 0 &&
           localSensorOutputs[1] >= 0;

#elif defined(USE_MPU6050)
    // X/Y/Z are homogeneous acceleration components and share one unit,
    // so they correctly form one three-dimensional logical sensor.
    localSensorOutputs[0] = bridge.addSensor("MPU6050 Acc", "g", 3);
    return localSensorOutputs[0] >= 0;

#elif defined(USE_MAX9814)
    localSensorOutputs[0] = bridge.addSensor("MAX9814 Mic", "raw");
    return localSensorOutputs[0] >= 0;

#elif defined(USE_KY037)
    localSensorOutputs[0] = bridge.addSensor("KY-037 Mic", "raw");
    return localSensorOutputs[0] >= 0;
#endif
}

bool configureLocalSensorPeriods()
{
    for (size_t index = 0; index < 3; ++index) {
        if (localSensorOutputs[index] >= 0 &&
            !bridge.setDataPeriodMs(
                localSensorOutputs[index],
                static_cast<uint16_t>(localMeasurementPeriodMs))) {
            return false;
        }
    }

    return true;
}

bool readLocalSensor()
{
    // Execute specific read logic based on the active #define
#if defined(USE_DHT22)
    const float val = localSensorObj.readHumidity();
    if (isnan(val)) return false;
    return bridge.set(localSensorOutputs[0], val);

#elif defined(USE_BH1750)
    const float val = localSensorObj.readLightLevel();
    if (val < 0) return false;
    return bridge.set(localSensorOutputs[0], val);

#elif defined(USE_BME280)
    const float t = localSensorObj.readTemperature();
    const float h = localSensorObj.readHumidity();
    const float p = localSensorObj.readPressure() / 100.0F;
    if (isnan(t) || isnan(h) || isnan(p)) return false;
    return bridge.set(localSensorOutputs[0], t) &&
           bridge.set(localSensorOutputs[1], h) &&
           bridge.set(localSensorOutputs[2], p);

#elif defined(USE_BMP280)
    const float t = localSensorObj.readTemperature();
    const float p = localSensorObj.readPressure() / 100.0F;
    if (isnan(t) || isnan(p)) return false;
    return bridge.set(localSensorOutputs[0], t) &&
           bridge.set(localSensorOutputs[1], p);

#elif defined(USE_MPU6050)
    int16_t ax, ay, az;
    localSensorObj.getAcceleration(&ax, &ay, &az);
    const float accelX = ax / 16384.0f;
    const float accelY = ay / 16384.0f;
    const float accelZ = az / 16384.0f;
    return bridge.set(localSensorOutputs[0], 0, accelX) &&
           bridge.set(localSensorOutputs[0], 1, accelY) &&
           bridge.set(localSensorOutputs[0], 2, accelZ);

#elif defined(USE_MAX9814) || defined(USE_KY037)
    const float val = analogRead(localSensorPin);
    return bridge.set(localSensorOutputs[0], val);
#endif
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Amperia multi-sensor combination test");

    // Initialize the active sensor
#if defined(USE_DHT22)
    localSensorObj.begin();
#elif defined(USE_BH1750)
    Wire.begin();
    localSensorObj.begin();
#elif defined(USE_BME280) || defined(USE_BMP280)
    Wire.begin();
    localSensorObj.begin(0x76);
#elif defined(USE_MPU6050)
    Wire.begin();
    localSensorObj.initialize();
#elif defined(USE_MAX9814) || defined(USE_KY037)
    pinMode(localSensorPin, INPUT);
#endif

    if (!sourceSensor.begin()) {
        Serial.println("Failed to initialize the external-sensor client.");
        return;
    }

    delay(5000);

    if (!sourceSensor.discover()) {
        printReadError("Discovery");
        return;
    }

    if (sourceSensor.sensorCount() == 0) {
        Serial.println("The external device reported no logical sensors.");
        return;
    }

    // Register the selected local sensor(s). The bridge reserves the required
    // logical-sensor slots and forwards as many external sensors as still fit.
    if (!configureLocalSensors() ||
        !configureLocalSensorPeriods() ||
        !bridge.begin()) {
        Serial.println("Failed to initialize the multimeter output.");
        return;
    }

    if (!bridge.update()) {
        printReadError("Initial external measurement read");
        return;
    }

    if (!readLocalSensor()) {
        Serial.println(SENSOR_ERROR_MSG);
        return;
    }

    bridge.publish();

    const uint32_t now = millis();
    previousSourcePollMs = now;
    previousLocalMeasurementMs = now;

    Serial.println("Forwarding the combined measurements.");
}

void loop()
{
    if (!bridge.isStarted()) {
        return;
    }

    const uint32_t now = millis();
    bool outputChanged = false;

    // DREADY is the normal path. Periodic polling remains as a fallback for
    // manual testing of sensors or firmware that do not produce DREADY.
    if (sourceSensor.dataReady()) {
        previousSourcePollMs = now;

        if (bridge.updateIfReady()) {
            outputChanged = true;
        } else {
            printReadError("DREADY external measurement read");
        }
    } else if (now - previousSourcePollMs >= sourceFallbackPollPeriodMs) {
        previousSourcePollMs = now;

        if (bridge.update()) {
            outputChanged = true;
        } else {
            printReadError("Periodic external measurement read");
        }
    }

    if (now - previousLocalMeasurementMs >= localMeasurementPeriodMs) {
        previousLocalMeasurementMs = now;

        if (readLocalSensor()) {
            outputChanged = true;
        } else {
            Serial.println(SENSOR_ERROR_MSG);
        }
    }

    if (outputChanged) {
        bridge.publish();
    }
}
