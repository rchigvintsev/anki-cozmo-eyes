# Глаза Anki Cozmo для Arduino

Эмоциональные анимированные глаза на OLED-экране, вдохновлённые роботом Anki Cozmo.

Проект основан на выразительной мимике робота Anki «Cozmo»: на маленьком OLED-экране 128x64 пикселя рисуется пара анимированных глаз.

<img src="doc/anki-cozmo-faces-3-1024x576.jpg" />

В отличие от библиотек, которые просто проигрывают набор заранее отрисованных растровых кадров, здесь каждый кадр строится динамически из программно заданных параметров.

Проект является форком репозитория <a href="https://github.com/playfultechnology/esp32-eyes">playfultechnology/esp32-eyes</a> (автор Alastair Aitchison), который, в свою очередь, сильно опирается на <a href="https://github.com/luisllamasbinaburo/ESP32_Faces/">ESP32_Faces</a> (Luis Llamas), но с существенными изменениями:

 - OLED (библиотека <a href="https://github.com/olikraus/u8g2">u8g2</a>) вместо TFT (на базе <a href="https://github.com/Bodmer/TFT_eSPI">TFT_eSPI</a>);
 - горизонтальное расположение глаз вместо вертикального;
 - зеркальные анимации левого и правого глаза;
 - автоматическое или ручное моргание и управление направлением взгляда.

## Аппаратное обеспечение

 - **Arduino Mega** (или любая совместимая плата)
 - **OLED-дисплей SSD1306, 128x64 пикселя, I2C**

Подключение дисплея (аппаратный I2C):

| OLED SSD1306 | Arduino Mega |
| --- | --- |
| VCC | 3.3V / 5V |
| GND | GND |
| SCL | SCL (пин 21) |
| SDA | SDA (пин 20) |

Адрес дисплея на шине I2C — `0x3C`. Если ваш модуль использует другой адрес, измените его в `Face.cpp`:

```cpp
u8g2.setI2CAddress(0x3C<<1);
```

> Обратите внимание: u8g2 использует 8-битный адрес I2C, поэтому 7-битный адрес сдвигается влево на один бит. Именно из-за этого значение `0x3C` в коде записано как `0x3C<<1`.

## Необходимые библиотеки

 - <a href="https://github.com/olikraus/u8g2">U8g2</a> (Oliver Kraus) — драйвер OLED-дисплея и графические примитивы
 - `Wire` — входит в состав Arduino IDE и используется для инициализации шины I2C

## Как это работает

Файл `anki-cozmo-eyes.ino` — точка входа. Основные параметры заданы константами:

```cpp
const int SCREEN_WIDTH  = 128;
const int SCREEN_HEIGHT =  64;
const int EYE_SIZE = 40;
const int FACE_CHANGE_INTERVAL_MILLIS = 3000;
```

В `setup()` создаётся объект `Face`, задаётся нейтральное выражение и отключается вся автоматика библиотеки — поведение полностью управляется вручную из `loop()`:

```cpp
face = new Face(SCREEN_WIDTH, SCREEN_HEIGHT, EYE_SIZE);
face->Expression.GoTo_Normal();

face->RandomBehavior = false;
face->RandomBlink = false;
face->RandomLook = false;

randomSeed(analogRead(A0));
```

`analogRead(A0)` используется только как источник энтропии для `randomSeed()` — это не чтение датчика и не влияет на работу глаз.

`loop()` каждые `FACE_CHANGE_INTERVAL_MILLIS` (по умолчанию 3000 мс, то есть раз в 3 секунды) выполняет одну и ту же последовательность:

 1. моргает — `face->DoBlink();`
 2. случайно смещает взгляд — `face->Look.LookAt(randomFloat(-1.0, 1.0), randomFloat(-1.0, 1.0));`
 3. переключает выражение на случайное из 18 доступных — `goToEmotion(random(0, 18));`

```cpp
void loop() {
  int now = millis();
  if (now - lastChangedAt > FACE_CHANGE_INTERVAL_MILLIS) {
    face->DoBlink();
    face->Look.LookAt(randomFloat(-1.0, 1.0), randomFloat(-1.0, 1.0));
    goToEmotion(random(0, 18));
    lastChangedAt = now;
  }
  face->Update();
}
```

Функция `randomFloat(minVal, maxVal)` возвращает случайное число с плавающей точкой в заданном диапазоне — в Arduino есть только целочисленный `random()`:

```cpp
float randomFloat(float minVal, float maxVal) {
  long randInt = random(0, 1000001);
  float normalized = randInt / 1000000.0;
  return minVal + normalized * (maxVal - minVal);
}
```

Функция `goToEmotion()` — простой `switch` по номеру эмоции, вызывающий соответствующий метод `FaceExpression`. Номера эмоций совпадают с порядком значений перечисления `eEmotions` (`FaceEmotions.hpp`):

| № | Метод | Эмоция |
| --- | --- | --- |
| 0 | `GoTo_Normal()` | Обычное |
| 1 | `GoTo_Angry()` | Злое |
| 2 | `GoTo_Glee()` | Ликующее |
| 3 | `GoTo_Happy()` | Счастливое |
| 4 | `GoTo_Sad()` | Грустное |
| 5 | `GoTo_Worried()` | Обеспокоенное |
| 6 | `GoTo_Focused()` | Сосредоточенное |
| 7 | `GoTo_Annoyed()` | Раздражённое |
| 8 | `GoTo_Surprised()` | Удивлённое |
| 9 | `GoTo_Skeptic()` | Скептическое |
| 10 | `GoTo_Frustrated()` | Разочарованное |
| 11 | `GoTo_Unimpressed()` | Равнодушное |
| 12 | `GoTo_Sleepy()` | Сонное |
| 13 | `GoTo_Suspicious()` | Подозрительное |
| 14 | `GoTo_Squint()` | Прищуренное |
| 15 | `GoTo_Furious()` | В ярости |
| 16 | `GoTo_Scared()` | Испуганное |
| 17 | `GoTo_Awe()` | Восхищённое |

## Структура программных компонентов

- *Face* — центральный объект: левый и правый глаз, поведение, выражение, помощник взгляда и помощник моргания. Выходы этих компонентов выстраиваются в цепочку и формируют изображение каждого кадра.

- *Behaviour* (`FaceBehavior`) хранит массив взвешенных _эмоций_ (перечисление `eEmotions` в `FaceEmotions.hpp`, 18 эмоций). Вес каждой эмоции задаётся отдельно:

```
face->Behavior.SetEmotion(eEmotions::Glee, 1.0);
face->Behavior.SetEmotion(eEmotions::Angry, 0.5);
```

Если `face->RandomBehavior = true;` (значение по умолчанию в библиотеке), Face методом «рулетки» случайно выбирает новую эмоцию пропорционально весам. Периодичность пересчёта текущей эмоции задаётся через `face->Behaviour.Timer.SetIntervalMillis();`, при этом смена происходит не всегда — может выпасть та же эмоция, что активна сейчас. Если ни одной эмоции не назначен вес, лицо переключается в состояние «normal».

Эмоции **не смешиваются** между собой — лицо всегда выражает только одну эмоцию за раз. Вес задаёт не способ композиции, а относительную вероятность выбора данной эмоции.

В этом скетче выставлено `face->RandomBehavior = false;`, поэтому автоматический выбор эмоций отключён — эмоция назначается вручную из `loop()` через `goToEmotion()`.

- *Expression* (`FaceExpression`) отвечает за переход глаз к выбранной эмоции. Для каждой эмоции в `EyePresets.h` задан пресет, определяющий высоту, наклон, радиус и смещение глаза; они применяются соответствующим методом `GoTo_*()`.

- Помощник моргания (`BlinkAssistant`) отвечает за моргание. Если `face->RandomBlink = true;` (значение по умолчанию в библиотеке), глаза моргают автоматически с интервалом, заданным через `face->Blink.Timer.SetIntervalMillis();`. Моргнуть вручную можно вызовом `face->DoBlink();` — именно так и поступает скетч.

- Помощник взгляда (`LookAssistant`) управляет положением глаз на экране. Если `face->RandomLook = true;` (значение по умолчанию в библиотеке), глаза случайно перемещаются по экрану с интервалом из `face->Look.Timer.SetIntervalMillis();`. Направить взгляд в конкретную точку можно вызовом `face->Look.LookAt(x, y);`, где `x` и `y` лежат в диапазоне от -1.0 до 1.0.

  Дополнительно доступны готовые методы: `LookLeft()`, `LookRight()`, `LookFront()`, `LookTop()`, `LookBottom()`.

- Каждый глаз (`Eye`) сам управляет своей отрисовкой. Цепочка преобразований `Transition` → `Transformation` → `Variation1` → `Variation2` → `BlinkTransformation` применяется к конфигурации `EyeConfig`, после чего результат передаётся в `EyeDrawer`, который рисует примитивы через `u8g2`.

- Все три «случайных» флага в скетче выставлены в `false`, поэтому автоматика библиотеки не используется: кадры обновляются вызовом `face->Update()`, а моргание, направление взгляда и эмоция задаются вручную.

## Настройка под себя

 - **Длительность показа эмоции** — константа `FACE_CHANGE_INTERVAL_MILLIS` в скетче.
 - **Размер глаз** — константа `EYE_SIZE` в скетче (сейчас 40).
 - **Своя последовательность эмоций** — вызывайте конкретные методы `face->Expression.GoTo_*()` вместо случайного `goToEmotion(random(0, 18))`.
 - **Автоматический режим** — выставьте `face->RandomBehavior`, `face->RandomBlink` и `face->RandomLook` в `true`, тогда библиотека сама будет менять эмоции, моргать и перемещать взгляд; интервалы задаются через `Timer.SetIntervalMillis()`.
