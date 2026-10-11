# AmperiaSTEM

> **English:** AmperiaSTEM is an Arduino library for working with EdPro Amperia STEM educational equipment. It supports reading Amperia STEM sensors, creating custom sensors for the Amperia Multimeter, processing measurements, and controlling actuators through the EdPro Amperia Shield. [Read the full English version →](README.en.md)

**AmperiaSTEM** — Arduino-бібліотека для роботи з навчальним обладнанням EdPro Amperia STEM за допомогою **Arduino UNO R4 WiFi** та **EdPro Amperia Shield**. Плата **Arduino UNO R4 WiFi** входить до складу набору Amperia STEM.

За допомогою бібліотеки Arduino може:

- читати підключені сенсори Amperia STEM;
- створювати власні сенсори й передавати їхні вимірювання на мультиметр Amperia;
- опрацьовувати, перетворювати та комбінувати вимірювання;
- керувати виконавчими пристроями, підключеними до EdPro Amperia Shield.

Бібліотека орієнтована на навчальне використання: типові задачі мають вимагати мінімум службового коду. Для складніших застосувань доступний нижчий рівень API для роботи з метаданими, типами даних і окремими логічними сенсорами.

## Пов'язані продукти EdPro

- **[EdPro Amperia](https://edpro.ua/amperia)** — лінійка навчальних наборів для лабораторних і демонстраційних робіт з фізики, зокрема з електрики та магнетизму й механіки.
- **[EdPro Amperia STEM](https://edpro.ua/amperia/amp-stem)** — набір «Електроніка, робототехніка та сенсори», який доповнює екосистему Amperia сенсорами, електронними компонентами та засобами для роботи з Arduino і робототехнікою. Саме для роботи з цим обладнанням розробляється AmperiaSTEM.

## Підтримувана Arduino-платформа

На цей момент бібліотека підтримує **Arduino UNO R4 WiFi** з **EdPro Amperia Shield**.

| ![EdPro Amperia Shield](photos/AmperiaShield.png) | ![Arduino UNO R4 WiFi](photos/ArduinoUno.jpg) |
| --- | --- |
| EdPro Amperia Shield | Arduino UNO R4 WiFi |

Інші Arduino-сумісні плати наразі не підтримуються, їх підтримка в процесі розробки.

## Як усе з'єднано

Типова конфігурація дозволяє Arduino одночасно читати зовнішній сенсор Amperia STEM та інші під'єднані сенсори, керувати виконавчими пристроями, наприклад двигунами, опрацьовувати вимірювання та передавати результати на мультиметр.

| ![SCHEME](./photos/Scheme.jpg) |
| --- |
| Типове підключення. |

На Arduino UNO R4 WiFi з EdPro Amperia Shield:

- `Wire` використовується для читання зовнішнього сенсора Amperia STEM;
- `A3` — сигнал DREADY від зовнішнього сенсора;
- `Wire1` використовується для зв'язку з мультиметром Amperia;
- `A2` — сигнал DREADY до мультиметра.

Через Qwiic-з'єднання між Arduino UNO R4 WiFi та EdPro Amperia Shield до шилда підводиться шина `Wire1` (I²C), яка використовується для зв'язку з мультиметром. Сигнали DREADY передаються окремими лініями: `A3` — від зовнішнього сенсора до Arduino, `A2` — від Arduino до мультиметра.

| ![Qwiic](./photos/Qwiic.png) |
| --- |
| Qwiic-роз'єм Arduino UNO R4 WiFi. Qwiic-роз'єми Arduino UNO R4 WiFi та EdPro Amperia Shield **потрібно з'єднати кабелем Qwiic**. |


## Механічне з'єднання плат

Перед встановленням від'єднайте Arduino від живлення. Сумістіть штирьові роз'єми EdPro Amperia Shield із відповідними гніздовими роз'ємами Arduino UNO R4 WiFi. Обережно встановіть шилд без перекосу та рівномірно притисніть його до повного з'єднання роз'ємів. Після цього з'єднайте Qwiic-роз'єми шилда та Arduino UNO R4 WiFi кабелем Qwiic. Це з'єднання підводить до шилда шину `Wire1`, потрібну для обміну даними з мультиметром Amperia.

| ![HOWTO](./photos/HowTo.png) | 
| --- | 
| З'єднання плат. |

| ![SETUP](./photos/SetupPhoto.jpg) | 
| --- | 
| Фото зібраної схеми для проведення першого експерименту. |

## Будова EdPro Amperia Shield

| ![EdPro Amperia Shield](photos/AmperiaShield.png) | 
| --- | 
| EdPro Amperia Shield. |

На платі розміщено такі основні функціональні блоки:

| ![TERMINAL1](./photos/RectA0A1.png) |  ![TERMINAL2](./photos/RectM1M2.png) |
| :---: | :---: |
| **Аналогові клеми (`A0`, `A1`)**: великі круглі магнітні контакти, підключені до аналогових входів `A0` і `A1` Arduino. Призначені для зручного підключення вимірювальних схем та елементів набору. | **Клеми DC-двигуна (`M+`, `M−`)**: великі круглі магнітні контакти для підключення двигуна постійного струму до виходу драйвера двигуна. |
| ![SENSOR](./photos/RectSensor.png) | ![MULTIMETER](./photos/RectMult.png) |
| **`SENSOR`**: роз'єм для підключення зовнішнього сенсора Amperia STEM.  |**`MULTIMETR`**: роз'єм для підключення мультиметра Amperia та обміну даними з ним. |
| ![ACTUATORS](./photos/RectAct.png) | ![BUTTONS](./photos/RectButtons.png) |
| Роз'єми крокового двигуна та двох серводвигунів. | **Кнопки користувача (`BTN1`, `BTN2`)**: дві кнопки для ручного керування, наприклад запуску та зупинки збору даних під час експерименту (підключені до `D10` та `D11`). Також на платі є кнопка **RESET** для перезавантаження Arduino UNO R4 WiFi. |

**Роз'єми для підключення двигунів**:
   * **DC MOTOR**: роз'єм для підключення двигуна постійного струму (керується через `D3/DC1`, `D5/DC2`).
   * **STEP MOTOR**: роз'єм для підключення крокового двигуна (керується через `D6/IN1`, `D7/IN2`, `D8/IN3`, `D9/IN4`).
   * **SERVO 1 / SERVO 2**: роз'єми для підключення серводвигунів (керуються через `D2/SRV1`, `D4/SRV2`).



| ![POWER](./photos/Rect5GND.png) | 
| --- | 
| **Клеми живлення (`5V`, `GND`)**: магнітні контакти для підключення живлення та спільного проводу до зовнішніх схем. |


## Встановлення

Після публікації AmperiaSTEM в **Arduino Library Manager** бібліотеку можна буде встановити безпосередньо з Arduino IDE, знайшовши `AmperiaSTEM` у менеджері бібліотек.

Поки бібліотека розробляється, її можна також встановити з цього репозиторію:

1. Встановіть Arduino IDE або Arduino CLI.
2. Встановіть пакет плат **Arduino UNO R4 Boards**.
3. Завантажте цей репозиторій як ZIP і додайте його через **Sketch → Include Library → Add .ZIP Library...**, або скопіюйте репозиторій до каталогу Arduino `libraries`.
4. Виберіть плату **Arduino UNO R4 WiFi**.

Основна бібліотека використовує стандартні Arduino-бібліотеки `Stepper` і `Servo`. Окремі приклади можуть потребувати додаткових бібліотек; зокрема `04_TemperatureSensor`, `05_AgentTemperatureSensor`, `09_ForwardSensorWithDHT11` та `16_MultipleDHTValues` використовують `DHT sensor library`, а `17_CombinedMPU6050` — бібліотеки `I2Cdev` і `MPU6050`. [`TestingSensors`](extras/manual-tests/TestingSensors/TestingSensors.ino), залежно від вибраного сенсора, може потребувати відповідної бібліотеки для нього.

## Перший власний сенсор

Розглянемо скорочений варіант [`03_CreateAnalogSensor`](examples/03_CreateAnalogSensor/03_CreateAnalogSensor.ino). Arduino читає аналоговий сигнал з `A0`, перетворює його на відсотки та передає результат на мультиметр Amperia.

```cpp
#include <AmperiaSTEM.h>

SimpleSensor analogSensor(
    "Student sensor",
    "Analog level",
    "%"
);

void setup()
{
    analogReadResolution(12);
    analogSensor.begin();
}

void loop()
{
    float level = 100.0f * analogRead(A0) / 4095.0f;

    analogSensor.publish(level);

    delay(160);
}
```

Повний приклад містить також перевірку ініціалізації та діагностичний вивід через `Serial`.

Подавати аналоговий сигнал можна, наприклад, за допомогою реостату з комплекту Amperia.

## Огляд бібліотеки

Бібліотека має три основні напрями роботи:

- Передавання на мультиметр вимірювань і результатів їх опрацювання Arduino.
- Читання сенсорів EdPro Amperia STEM, під'єднаних до шилда.
- Керування виконавчими пристроями, під'єднаними до шилда.

### Arduino → мультиметр Amperia

Для передавання на мультиметр вимірювань і результатів їх опрацювання Arduino бібліотека надає такі класи:

- `SimpleSensor` — один логічний сенсор з одним значенням.
- `AgentSensor` — одновимірний сенсор, вимірювання для якого виконує задана функція, спрощений варіант попереднього.
- `NDimSensor` — один логічний сенсор з кількома однорідними компонентами, наприклад X/Y/Z.
- `NSensors` — кілька одновимірних логічних сенсорів.
- `CombinedSensor` — кілька логічних сенсорів, зокрема різної розмірності.

Для даних, які Arduino передає на мультиметр, `set(...)` змінює підготовлене значення без публікації на мультиметрі, а `publish()` публікує поточний стан. Для `SimpleSensor` коротка форма `publish(value)` виконує обидві дії одразу.

У простих навчальних задачах зазвичай достатньо `SimpleSensor` або `NDimSensor`.

Якщо один пристрій має кілька одновимірних величин з різними одиницями, використовуйте `NSensors`:

```cpp
NSensors measurements("Weather");

int temperature = measurements.addSensor("Temperature", "C");
int humidity = measurements.addSensor("Humidity", "%");

measurements.begin();

measurements.set(temperature, 23.5f);
measurements.set(humidity, 48.0f);
measurements.publish();
```

Якщо разом потрібно передавати сенсори різної розмірності, використовуйте `CombinedSensor`:

```cpp
CombinedSensor measurements("Experiment");

int acceleration =
    measurements.addSensor("Acceleration", "g", "X", "Y", "Z");
int magnitude = measurements.addSensor("Magnitude", "g");

measurements.begin();

measurements.set(acceleration, 0, 0.10f);
measurements.set(acceleration, 1, 0.20f);
measurements.set(acceleration, 2, 0.95f);
measurements.set(magnitude, 0.98f);
measurements.publish();
```

`addSensor()` повертає ідентифікатор (handle) логічного сенсора, який потім можна передавати `set()`. До виклику `begin()` через `setDataPeriodMs(handle, periodMs)` можна вказати період оновлення, який сенсор повідомляє мультиметру. Саму періодичність вимірювань задає код програми. `begin()` автоматично завершує конфігурацію. 

Повні приклади: [`16_MultipleDHTValues`](examples/16_MultipleDHTValues/16_MultipleDHTValues.ino) та [`17_CombinedMPU6050`](examples/17_CombinedMPU6050/17_CombinedMPU6050.ino).

### Сенсор Amperia STEM → Arduino

`AmperiaSensorClient` використовується, коли Arduino читає зовнішній сенсор Amperia STEM. Він виявляє доступні логічні сенсори, зчитує їхні метадані та поточні значення без прив'язки до конкретної моделі сенсора.

Для конкретних навчальних задач бібліотека також може надавати зручніші спеціалізовані операції поверх цього механізму.

### Виконавчі пристрої

Для керування виконавчими пристроями, підключеними до EdPro Amperia Shield, бібліотека надає кілька простих класів:

- `AmpDC` — керування напрямком і швидкістю обертання двигуна постійного струму.
- `AmpServo` — керування сервоприводом, зокрема встановлення абсолютного кута та поворот відносно поточного положення.
- `AmpStepper` — керування кроковим двигуном у кроках або градусах із заданою швидкістю.

За замовчуванням ці класи використовують відповідні роз'єми та піни EdPro Amperia Shield, тому для типових навчальних задач додатково задавати піни не потрібно.

Приклади використання: [`13_DCMotorUsage`](examples/13_DCMotorUsage/13_DCMotorUsage.ino), [`14_ServoMotorUsage`](examples/14_ServoMotorUsage/14_ServoMotorUsage.ino) та [`15_StepperMotorUsage`](examples/15_StepperMotorUsage/15_StepperMotorUsage.ino).


## Приклади

Приклади розташовані в каталозі [`examples`](examples/). Варто почати з простих прикладів і переходити до універсального API та комбінування сенсорів лише за потреби.

| Приклад | Що демонструє | Рівень |
| --- | --- | --- |
| `01_GettingStarted` | базова робота з готовим сенсором Amperia STEM | початковий |
| `02_ListSensors` | автоматичне виявлення логічних сенсорів і читання їхніх метаданих | середній |
| `03_CreateAnalogSensor` | створення власного одновимірного сенсора | початковий |
| `04_TemperatureSensor` | DHT11 як власний сенсор температури для мультиметра | початковий |
| `05_AgentTemperatureSensor` | автоматичне вимірювання через `AgentSensor` і callback-функцію | розширений |
| `06_GateSensor` | читання сенсора воріт і передавання вибраних вимірювань | початковий |
| `07_FourPinStateSensor` | власний багатокомпонентний сенсор | початковий |
| `08_ForwardSensorToMultimeter` | універсальне передавання зовнішнього сенсора на мультиметр | розширений |
| `09_ForwardSensorWithDHT11` | комбінування зовнішнього сенсора Amperia STEM з локальним DHT11 | розширений |
| `10_IfElseCondition` | реакція Arduino на виміряне значення за допомогою `if` / `else` | початковий |
| `11_VectorCalibration` | калібрування та опрацювання векторного вимірювання | середній |
| `12_KinematicsSpeed` | обчислення нової фізичної величини з виміряних даних | початковий |
| `13_DCMotorUsage` | керування DC-двигуном | початковий |
| `14_ServoMotorUsage` | керування сервоприводом | початковий |
| `15_StepperMotorUsage` | керування кроковим двигуном | початковий |
| `16_MultipleDHTValues` | кілька одновимірних величин через `NSensors` | середній |
| `17_CombinedMPU6050` | вектор і скаляр в одному пристрої через `CombinedSensor` | середній |

Додаткові матеріали винесені з `examples/`: [`TestingSensors`](extras/manual-tests/TestingSensors/TestingSensors.ino) призначений для ручної перевірки різних конфігурацій сенсорів, а [`AllFeaturesReference`](extras/reference/AllFeaturesReference/AllFeaturesReference.ino) — як розширений довідковий скетч. Вони не входять до звичайного набору навчальних прикладів.

Рекомендований маршрут для першого знайомства:

```text
01_GettingStarted
        ↓
03_CreateAnalogSensor
        ↓
04_TemperatureSensor
        ↓
06_GateSensor / 07_FourPinStateSensor
        ↓
10_IfElseCondition / 12_KinematicsSpeed
        ↓
08_ForwardSensorToMultimeter / 09_ForwardSensorWithDHT11
        ↓
16_MultipleDHTValues / 17_CombinedMPU6050
```

Приклади `16_MultipleDHTValues` і `17_CombinedMPU6050` показують більш низькорівневий універсальний API для кількох логічних сенсорів, до них варто переходити після простіших `SimpleSensor` і `NDimSensor`.

Приклади `13_DCMotorUsage`–`15_StepperMotorUsage` можна проходити окремо, коли потрібна робота з виконавчими пристроями.

## Розробка і перевірка

Для локальної перевірки бібліотеки використовуються три скрипти:

- `scripts/compile-examples.sh` — компілює всі або вибрані навчальні приклади,
- `scripts/compile-testing-sensors-matrix.sh` — компілює `TestingSensors` для всіх підтримуваних конфігурацій і після перевірки відновлює файл,
- `scripts/validate-library.sh` — запускає повну передрелізну перевірку: приклади, матрицю `TestingSensors`, `AllFeaturesReference`, Arduino Lint і `git diff --check`.

Основні команди:

```bash
./scripts/compile-examples.sh --clean
./scripts/compile-examples.sh --clean 17_CombinedMPU6050
./scripts/compile-testing-sensors-matrix.sh
./scripts/validate-library.sh
```

На чистому середовищі залежності для прикладів можна встановити автоматично:

```bash
./scripts/validate-library.sh --install-deps
```

Додаткові параметри кожного скрипта доступні через `--help`.

## Поточні обмеження

- підтримується лише Arduino UNO R4 WiFi з EdPro Amperia Shield;
- один пристрій Amperia може описувати до **5 логічних сенсорів**;
- один логічний сенсор може містити до **4 компонентів**;
- для поточного інтерфейсу мультиметра рекомендовано не більше **3 компонентів** на логічний сенсор, оскільки четвертий компонент відображається менш зручно;
- API та приклади ще розвиваються і можуть змінюватися до першого стабільного випуску.

## Структура бібліотеки

Для звичайного використання достатньо підключити один заголовок:

```cpp
#include <AmperiaSTEM.h>
```

Основні частини репозиторію:

- `src/` — реалізація бібліотеки;
- `src/client/` — читання зовнішніх сенсорів Amperia STEM;
- `src/protocol/` — опис протоколу й кодування регістрів;
- `src/device/` — представлення Arduino як пристрою Amperia;
- `src/internal/motors/` — внутрішня реалізація роботи з виконавчими пристроями;
- `examples/` — навчальні приклади;
- `extras/manual-tests/` — ручні тестові скетчі;
- `extras/reference/` — розширені довідкові скетчі;
- `scripts/` — допоміжні скрипти для розробки та перевірки.

## Ліцензія

Copyright 2026 EdPro.

AmperiaSTEM поширюється за умовами [Apache License 2.0](LICENSE).
