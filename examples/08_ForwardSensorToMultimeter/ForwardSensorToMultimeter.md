# Восьмий приклад: пересилання даних сенсора на мультиметр

Цей приклад показує, як прийняти показання від зовнішнього сенсора через `AmperiaSensorClient` і безпосередньо передати їх на Amperia Multimeter за допомогою `AmperiaBridge`.

Це корисний шаблон, коли треба "просто переслати" вже існуючі показання сенсора на мультиметр, не розробляючи окремий сенсорний об'єкт або власну логіку обробки.

## Потрібне обладнання

Для запуску прикладу потрібне таке обладнання:

1. Arduino UNO R4 WiFi,
2. EdPro Amperia Shield,
3. Qwiic-кабель для з'єднання Arduino UNO R4 WiFi з EdPro Amperia Shield,
4. зовнішній EdPro Sensor,
5. USB-кабель для живлення/завантаження коду,
6. Amperia Multimeter.

## Як підключити обладнання

1. З'єднайте Arduino UNO R4 WiFi та EdPro Amperia Shield згідно з інструкцією.
2. З'єднайте Qwiic-роз'єми Arduino UNO R4 WiFi та EdPro Amperia Shield кабелем Qwiic.
3. Підключіть зовнішній EdPro Sensor до роз'єму `SENSOR` на EdPro Amperia Shield.
4. Підключіть Amperia Multimeter до роз'єму `MULTIMETER` на EdPro Amperia Shield.
5. Підключіть Arduino до ноутбука чи комп'ютера за допомогою USB-кабеля.
   - Не змінюйте підключення обладнання, поки воно підключене до живлення.
   - Після підключення Arduino з'явиться в системі як послідовний порт, наприклад `COM8` у Windows або `/dev/ttyACM0` у Linux.

## Що робить цей приклад

1. Ініціалізує з'єднання з зовнішнім сенсором через `sourceSensor.begin()`.
2. Виконує пошук доступних сенсорів через `sourceSensor.discover()`.
3. Перевіряє, що пристрій має хоча б один сенсор через `sourceSensor.sensorCount()`.
4. Створює `AmperiaBridge bridge(sourceSensor)` для пересилання даних на мультиметр.
5. Запускає вихідний канал через `bridge.begin()`.
6. У `setup()` виконує один початковий `bridge.update()` і `bridge.publish()` без очікування `DREADY`.
7. У `loop()` перевіряє `bridge.updateIfReady()`.
8. Якщо дані готові, викликає `bridge.publish()` і передає їх на мультиметр.