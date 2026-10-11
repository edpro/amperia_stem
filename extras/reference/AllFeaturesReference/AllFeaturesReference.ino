/*
 * ============================================================
 *  AmperiaSTEM — REFERENCE SKETCH
 *  Закоментуй або видали те, що не потрібно у своєму проекті.
 *  Comment out or remove anything you do not need in your project.
 * ============================================================
 *
 *  Підключені сенсори (визначаються автоматично з EdPro Sensor):
 *  Connected sensors (discovered automatically from the EdPro Sensor):
 *    • RGB + Clear         [mW/m2]
 *    • Illuminance         [lux]
 *    • VecXYZ + Magnitude  [mkT]
 *    • Level + Frequency   [] / [Hz]
 *    • CO2Sens: CO2        [ppm] ; Temp/Hum   [C/%]
 *    • UVSens: UV Index    [] ; Irradiance    [mW/cm^2] ; Ambient Light  [lux]
 *    • PTHSens: Temp       [C] ; Humidity  [%]  ; Pressure  [hPa]
 *    • Precise Temperature [C]
 *    • Travel Time         [S]
 *    • Between Time        [S]
 *    • Number of objects 1 [count]
 *    • Number of objects 2 [count]
 *
 *  Актуатори (підключаються вручну до конекторів на верхній платі Amperia Shield):
 *  Actuators (connected manually to the Amperia Shield):
 *    • AmpStepper  -- кроковий мотор
 *    • AmpStepper  -- stepper motor
 *    • AmpServo    -- сервомотор
 *    • AmpServo    -- servo motor
 * ============================================================
 */

#include <AmperiaSTEM.h>
// Розкоментуй якщо використовуєш сервомотор:
// Uncomment when using a servo motor:
// #define USE_SERVO

/*
 * ============================================================
 * ОГОЛОШЕННЯ ОБ'ЄКТІВ
 * OBJECT DECLARATIONS
 * Залиш тільки ті, що потрібні у твоєму проекті.
 * Keep only the objects needed by your project.
 * ============================================================
 */
AmperiaSensorClient sensors;            // читання сенсорів з EdPro Sensor
                                        // Reads sensors from the EdPro Sensor

AmperiaBridge display(sensors);         // екран Мультиметра Amperia (видали якщо не будеш підключати мультиметр)
                                        // Forwards data to the Amperia Multimeter; remove it if no multimeter is used

AmpStepper motor;                     // кроковий мотор на роз'ємі STEP MOTOR
                                      // Stepper motor on the STEP MOTOR connector

#ifdef USE_SERVO
AmpServo servo;                         // сервомотор на роз'ємі SERVO 1
                                        // Servo motor on the SERVO 1 connector
#endif

/*
 * ============================================================
 * ДОПОМІЖНІ ФУНКЦІЇ
 * HELPER FUNCTIONS
 *
 * Конкретні назви логічних сенсорів залишаємо у цьому прикладі.
 * Універсальний AmperiaSensorClient нічого не знає про моделі
 * RGB, CO2, фотоворіт тощо.
 * Concrete logical-sensor names belong to this reference sketch.
 * The generic AmperiaSensorClient does not know specific RGB, CO2,
 * gate-sensor, or other product models.
 * ============================================================
 */
float readSensorValue(const char* sensorName, int component = 0)
{
    auto* sensor = sensors.findSensor(sensorName);

    if (sensor == nullptr) {
        // Значення далеко поза робочими діапазонами сенсорів
        // з цього довідкового прикладу.
        // Sentinel value well outside the operating ranges used by
        // this reference sketch.
        return -10000.0f;
    }

    return sensor->value(component);
}

/*
 * ============================================================
 * ТВОЯ ЛОГІКА ВИМІРЮВАНЬ
 * MEASUREMENT LOGIC
 * ============================================================
 */
bool doMeasurement() {

    // RGB сенсор
    // RGB sensor
    // float r = readSensorValue("RGB", 0);         // червоний канал, mW/m2
    //                                              // Red channel, mW/m2
    // float g = readSensorValue("RGB", 1);         // зелений канал, mW/m2
    //                                              // Green channel, mW/m2
    // float b = readSensorValue("RGB", 2);         // синій канал, mW/m2
    //                                              // Blue channel, mW/m2
    // float clear = readSensorValue("Clear");      // загальна інтенсивність (без фільтра), mW/m2
    //                                              // Unfiltered intensity, mW/m2
    // Serial.print("Red: "); Serial.println(r);
    // Serial.print("Green: "); Serial.println(g);
    // Serial.print("Blue: "); Serial.println(b);
    // Serial.print("Clear: "); Serial.println(clear);

    // // Сенсор освітленості
    // // Illuminance sensor
    // float lux = readSensorValue("Illuminance");  // lux
    // Serial.print("Light: "); Serial.println(lux);

    // // Магнетометр
    // // Magnetometer
    // float mx = readSensorValue("VecXYZ", 0);     // вісь X, mkT
    //                                              // X axis, mkT
    // float my = readSensorValue("VecXYZ", 1);     // вісь Y, mkT
    //                                              // Y axis, mkT
    // float mz = readSensorValue("VecXYZ", 2);     // вісь Z, mkT
    //                                              // Z axis, mkT
    // float mag = readSensorValue("Magnitude");    // скалярна величина, mkT
    //                                              // Scalar magnitude, mkT
    // Serial.print("X: "); Serial.println(mx);
    // Serial.print("Y: "); Serial.println(my);
    // Serial.print("Z: "); Serial.println(mz);
    // Serial.print("Magnitude: "); Serial.println(mag);

    // // Сенсор звуку
    // // Sound sensor
    // float level = readSensorValue("Level");      // амплітуда (б/р)
    //                                              // Dimensionless amplitude
    // float freq = readSensorValue("Frequency");   // частота, Hz
    //                                              // Frequency, Hz
    // Serial.print("Sound level: "); Serial.println(level);
    // Serial.print("Frequency: "); Serial.println(freq);

    // // CO2 + Температура/Вологість
    // // CO2 + temperature/humidity
    // float co2 = readSensorValue("CO2");          // ppm
    // float temp = readSensorValue("Temp/Hum", 0); // °C
    // float hum = readSensorValue("Temp/Hum", 1);  // %
    // Serial.print("CO2: "); Serial.println(co2);
    // Serial.print("Temperature: "); Serial.println(temp);
    // Serial.print("Humidity: "); Serial.println(hum);

    // // Сенсор температури
    // // Precision temperature sensor
    // float preciseTemp = readSensorValue("PT100");
    // Serial.print("Precise Temperature: "); Serial.println(preciseTemp);

    // // Ворота
    // // Gate sensor
    // float travelTime  = readSensorValue("Travel (S)", 0); // Час прольоту між воротами, секунди
    //                                                       // Travel time, seconds
    // float betweenTime = readSensorValue("Travel (S)", 1); // Час між об'єктами, секунди
    //                                                       // Time between objects, seconds
    // int objCount = (int)readSensorValue("Number 1");       // Кількість об'єктів, які пройшли через перші ворота
    //                                                        // Objects through gate 1
    // int objCount2 = (int)readSensorValue("Number 2");      // Кількість об'єктів, які пройшли через другі ворота
    //                                                        // Objects through gate 2
    // Serial.print("Gate Travel: "); Serial.print(travelTime, 4); Serial.print(" s | ");
    // Serial.print("Gate Between: "); Serial.print(betweenTime, 4); Serial.print(" s | ");
    // Serial.print("Total Objects (Gate 1): "); Serial.println(objCount);
    // Serial.print("Total Objects (Gate 2): "); Serial.println(objCount2);

    // // Датчик тиску, температури, вологості
    // // Pressure, temperature, and humidity sensor
    // float temp2 = readSensorValue("PressHumSensor", 0); // °C
    // float hum2  = readSensorValue("PressHumSensor", 1); // %
    // float press = readSensorValue("PressHumSensor", 2); // hPa
    // Serial.print("Temperature: "); Serial.println(temp2);
    // Serial.print("Humidity: ");    Serial.println(hum2);
    // Serial.print("Pressure: ");    Serial.println(press);

    // // Якщо є два сенсори одного типу -- шукай їх за частиною назви.
    // // If two sensors have the same type, search by a distinguishing name fragment.
    // float co2Indoor  = readSensorValue("Indoor");
    // float co2Outdoor = readSensorValue("Outdoor");

    // // Будь-який сенсор за іменем (запасний варіант)
    // // Any sensor by name (fallback example)
    // float val = readSensorValue("SCD41: CO2", 0);

    // // AmpStepper
    // motor.step(200);            // рухатись на 200 кроків вперед
    //                             // Move 200 steps forward
    // motor.step(-200);           // рухатись на 200 кроків назад
    //                             // Move 200 steps backward
    // motor.stepDegrees(90);      // повернутись на 90°
    //                             // Rotate by 90 degrees
    // motor.stepDegrees(-45);     // повернутись на -45°
    //                             // Rotate by -45 degrees
    // motor.setSpeed(60);         // змінити швидкість (RPM)
    //                             // Change speed in RPM

    // // AmpServo (тільки якщо зверху є #define USE_SERVO) 
    // // AmpServo (only when USE_SERVO is defined above)
    // #ifdef USE_SERVO
    // servo.setAngle(90);         // поставити на 90°
    //                             // Move to 90 degrees
    // servo.rotate(45);           // повернутись відносно поточного кута на +45°
    //                             // Rotate +45 degrees from the current angle
    // servo.rotate(-30);          // повернутись відносно поточного кута на -30°
    //                             // Rotate -30 degrees from the current angle
    // int angle = servo.getAngle(); // отримати поточний кут
    //                               // Read the current angle
    // #endif

    // Надіслати всі значення на екран Мультиметра.
    // Publish all current values to the multimeter.
    display.publish();          // завжди викликай в кінці doMeasurement() !!!
                                // Publish once after updating all values

    return true;
}

/*
 * ============================================================
 *  SETUP — не змінюй без потреби
 *  SETUP — do not change unless necessary
 * ============================================================
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting Amperia STEM Framework...");

    // Ініціалізація I2C та читання опису сенсорів з EdPro Sensor.
    // Initialize I2C access to the external EdPro Sensor.
    if (!sensors.begin()) {
        Serial.println("Failed to start the Amperia STEM sensor.");
        return;
    }

    delay(5000);                // даємо зовнішньому сенсору час завершити запуск
                                // Give the external sensor time to finish startup

    if (!sensors.discover()) {
        Serial.println("Failed to read the sensor list.");
        return;
    }

    // Реєстрація прочитаних сенсорів на екрані мультиметра.
    // (видали цей рядок, якщо мультиметра немає).
    // Configure forwarding of the discovered sensors to the multimeter.
    if (!display.begin()) {
        Serial.println("Failed to start the multimeter output.");
        return;
    }

    motor.setSpeed(70);         // початкова швидкість мотора (RPM)
                                // Initial motor speed in RPM

    #ifdef USE_SERVO
    servo.begin();              // підключити сервомотор до піна
                                // Initialize the servo output
    #endif

    // Одразу опрацьовуємо перше доступне вимірювання.
    // Process the first available measurement immediately.
    if (display.update()) {
        doMeasurement();
    }

    Serial.println("System Ready!");
}

/*
 * ============================================================
//  LOOP — не змінюй
//  LOOP — do not change
 * ============================================================
 */
void loop() {
    // Перевіряємо сигнал DREADY і читаємо нові значення сенсорів.
    // Check DREADY and read new sensor values.
    if (display.updateIfReady()) {
        doMeasurement();
    }
}