# П'ятий приклад: AgentSensor для DHT11

Цей приклад показує, як передати функцію вимірювання класу `AgentSensor`:

- функція `readTemperature(...)` зчитує температуру з DHT11 і повідомляє, чи вимірювання успішне,
- `AgentSensor` викликає цю функцію під час вимірювання,
- `measureAndPublish(...)` виконує вимірювання та публікує нове значення лише в разі успіху,
- час наступного вимірювання контролюється в `loop()` за допомогою `millis()`.

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

При успішних вимірюваннях приклад виводить:

```text
Amperia AgentSensor DHT11 example is ready.
Temperature measured and published.
Temperature measured and published.
...
```

Якщо DHT11 не відповідає, буде виведено:

```text
DHT is not connected or did not respond.
```

## Що робить цей приклад

1. Ініціалізує DHT11 через `dht.begin()`.
2. Визначає функцію `readTemperature(float& temperatureC)`, яка зчитує DHT11 і повертає `false`, якщо результат некоректний.
3. Передає `readTemperature` конструктору `AgentSensor`.
4. Повідомляє мультиметру період оновлення `1500` мс через `amperiaTemperature.setDataPeriodMs(...)`.
5. Ініціалізує `AgentSensor` через `amperiaTemperature.begin()`.
6. У `loop()` за допомогою `millis()` перевіряє, чи минуло `1500` мс від попередньої спроби вимірювання.
7. Викликає `amperiaTemperature.measureAndPublish(now)`.
8. Якщо `readTemperature(...)` повертає `true`, нове значення публікується на мультиметр.
9. Якщо функція повертає `false`, попереднє значення не публікується і в Serial Monitor з'являється повідомлення про помилку.

## Чим цей приклад відрізняється від попереднього

У `04_TemperatureSensor` функцію `dht.readTemperature()` безпосередньо викликає `loop()`, після чого код окремо перевіряє результат і викликає `publish()`.

У цьому прикладі логіка самого вимірювання винесена у `readTemperature(...)` і передана `AgentSensor`. Тому один виклик `measureAndPublish(...)` виконує вимірювання, перевіряє його успішність через результат callback-функції та публікує значення лише тоді, коли вимірювання успішне.

`AgentSensor` при цьому не задає періодичність викликів автоматично: у цьому прикладі її контролює код у `loop()` за допомогою `millis()`.
