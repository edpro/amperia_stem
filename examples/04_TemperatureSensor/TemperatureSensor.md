# Четвертий приклад: температура з DHT11

Цей приклад показує, як створити сенсор температури за допомогою `SimpleSensor`:

- зчитати температуру з DHT11,
- перевірити, чи вдалося отримати значення,
- передати температуру на Amperia Multimeter,
- вивести значення в Serial Monitor.

## Потрібне обладнання

Для запуску прикладу потрібне таке обладнання:

1. Arduino UNO R4 WiFi,
2. EdPro Amperia Shield,
3. Qwiic-кабель для з'єднання Arduino UNO R4 WiFi з EdPro Amperia Shield,
4. DHT11,
5. USB-кабель для живлення/завантаження коду,
6. Amperia Multimeter.

## Потрібна бібліотека

Для компіляції цього прикладу встановіть у Arduino IDE бібліотеку `DHT sensor library`.

## Як підключити обладнання

1. З'єднайте Arduino UNO R4 WiFi та EdPro Amperia Shield згідно з інструкцією.
2. З'єднайте Qwiic-роз'єми Arduino UNO R4 WiFi та EdPro Amperia Shield кабелем Qwiic.
3. Підключіть DHT11:
   - `VCC -> 5V` або `3.3V`,
   - `GND -> GND`,
   - `DATA -> A1`.
4. Підключіть Amperia Multimeter до роз'єму `MULTIMETER` на EdPro Amperia Shield.
5. Підключіть Arduino до ноутбука чи комп'ютера за допомогою USB-кабеля.
   - Не змінюйте підключення обладнання, поки воно підключене до живлення.
   - Після підключення Arduino з'явиться в системі як послідовний порт, наприклад `COM8` у Windows або `/dev/ttyACM0` у Linux.

## Як відкрити проект

1. Відкрийте Arduino IDE.
2. Виберіть `File` → `Open...`.
3. Відкрийте файл: `examples/04_TemperatureSensor/04_TemperatureSensor.ino`.

## Як завантажити код на плату

1. У Arduino IDE виберіть плату:
   - `Arduino UNO R4 WiFi`
2. Виберіть порт:
   - наприклад, `COM8`, `COM4`, `/dev/ttyACM0` тощо.
3. Натисніть `Upload`.
4. Після успішної компіляції та прошивки на платі запуститься код.

## Як відкрити Serial Monitor

Додаткову інформацію Arduino передає через послідовний порт. Щоб побачити її, зробіть наступне:

1. У Arduino IDE відкрийте меню:
   `Tools -> Serial Monitor`
2. Встановіть швидкість передачі:
   - `115200`
3. Після відкриття Serial Monitor зачекайте, поки програма виведе дані.

Приклад виводить дані так:

```text
Amperia DHT temperature sensor is ready.
Temperature: 22.50 C
Temperature: 22.40 C
Temperature: 22.60 C
...
```

Якщо DHT11 не відповідає, замість вимірювання буде виведено:

```text
DHT is not connected or did not respond.
```

## Що робить цей приклад

1. Ініціалізує DHT11 через `dht.begin()`.
2. Повідомляє мультиметру період оновлення `1500` мс через `amperiaTemperature.setDataPeriodMs(...)`.
3. Ініціалізує `SimpleSensor` через `amperiaTemperature.begin()`.
4. У `loop()` зчитує температуру через `dht.readTemperature()`.
5. Перевіряє результат за допомогою `isnan(...)`.
6. Якщо вимірювання успішне, передає температуру на мультиметр через `amperiaTemperature.publish(temperatureC)`.
7. Виводить температуру в Serial Monitor.
8. Чекає `1500` мс перед наступною спробою вимірювання.

У цьому прикладі `1500` мс використовується і як період, який сенсор повідомляє мультиметру, і як фактична затримка між послідовними вимірюваннями.
