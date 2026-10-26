# Шостий приклад: Gate Sensor

Цей приклад показує, як зчитувати два окремі лічильники з зовнішнього `Gate Sensor` і передавати їхні значення на Amperia Multimeter через `AmperiaSensorClient` і `NDimSensor`.

- `gateSensor.begin()` запускає інтерфейс для читання пристрою.
- `gateSensor.discover()` знаходить доступні сенсори, які віддає зовнішній пристрій.
- `findSensor("Number 1")` і `findSensor("Number 2")` вибирають два окремі лічильники за іменем.
- `gateOutput.publish(...)` передає обидва значення на мультиметр як двовимірний вивід.

## Потрібне обладнання

Для запуску прикладу потрібне таке обладнання:

1. Arduino UNO R4 WiFi,
2. EdPro Amperia Shield,
3. Qwiic-кабель для з'єднання Arduino UNO R4 WiFi з EdPro Amperia Shield,
4. EdPro Gate Sensor,
5. USB-кабель для живлення/завантаження коду,
6. Amperia Multimeter.

## Як підключити обладнання

1. З'єднайте Arduino UNO R4 WiFi та EdPro Amperia Shield згідно з інструкцією.
2. З'єднайте Qwiic-роз'єми Arduino UNO R4 WiFi та EdPro Amperia Shield кабелем Qwiic.
3. Підключіть EdPro Gate Sensor до роз'єму `SENSOR` на EdPro Amperia Shield.
4. Підключіть Amperia Multimeter до роз'єму `MULTIMETER` на EdPro Amperia Shield.
5. Підключіть Arduino до ноутбука чи комп'ютера за допомогою USB-кабеля.
   - Не змінюйте підключення обладнання, поки воно підключене до живлення.
   - Після підключення Arduino з'явиться в системі як послідовний порт, наприклад `COM8` у Windows або `/dev/ttyACM0` у Linux.

## Що робить цей приклад

1. Ініціалізує з'єднання з зовнішнім `Gate Sensor` через `gateSensor.begin()`.
2. Виконує пошук доступних логічних сенсорів через `gateSensor.discover()`.
3. Вибирає два окремі лічильники за їхніми іменами: `"Number 1"` і `"Number 2"`.
4. Налаштовує двовимірний вихід `NDimSensor gateOutput` з двома каналами даних.
5. У `loop()` викликає `gateSensor.update()` і перевіряє, чи з'явилися нові значення.
6. Якщо нові дані є, викликає `gateOutput.publish(firstCounter->value(), secondCounter->value())`.
7. Повторює оновлення на заданому періоді `measurementPeriodMs`.

## Важлива особливість

`Gate Sensor` може видавати не тільки лічильники, а й дані про часові вимірювання. Тому для надійної роботи важливо використовувати саме імена `"Number 1"` і `"Number 2"`, а не покладатися на порядок датчиків у масиві.
