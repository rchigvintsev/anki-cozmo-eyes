/// @file anki-cozmo-eyes.ino
/// @brief Глаза Anki Cozmo для Arduino.
///
/// Прошивка выводит на монохромный OLED-дисплей 128×64 анимированные глаза робота Cozmo и периодически меняет их
/// выражение. Состав схемы:
///  - Arduino Uno/Nano в качестве контроллера;
///  - OLED-дисплей SSD1306 128×64, подключённый по аппаратному I²C (SDA — A4, SCL — A5).
///
/// Описание работы:
///   Каждые @ref FACE_CHANGE_INTERVAL_MILLIS миллисекунд прошивка моргает (@ref Face::DoBlink), переводит взгляд в
///   случайную точку (@ref Face::Look) и выбирает случайное выражение из @ref EMOTIONS_NUMBER доступных (см.
///   @ref goToEmotion). Между изменениями каждый проход @ref loop отрисовывает текущее состояние лица.
///
/// Автоматическое поведение библиотеки (случайные выражения, моргание и движение взгляда, задаваемые классом
/// @ref FaceBehavior) отключено: анимацией управляет только этот скетч. Начальное зерно генератора случайных чисел
/// берётся с неподключённого вывода A0, поэтому выражение при каждом запуске своё.
///
/// Является форком репозитория https://github.com/playfultechnology/esp32-eyes, автором которого является Alastair
/// Aitchison.
///
/// @author Roman Chigvintsev

#include <Wire.h>
#include "Face.h"

/// Ширина дисплея в пикселях.
const int SCREEN_WIDTH  = 128;
/// Высота дисплея в пикселях.
const int SCREEN_HEIGHT =  64;
/// Размер одного глаза в пикселях: чем меньше значение, тем шире разносится пара глаз.
const int EYE_SIZE = 40;

/// Интервал между сменами выражения лица, мс.
const int FACE_CHANGE_INTERVAL_MILLIS = 3000;

/// Число доступных выражений лица; должно совпадать с числом ветвей @ref goToEmotion.
const int EMOTIONS_NUMBER = 18;

/// Глаза робота: единственный экземпляр лица, которым управляет скетч.
Face *face;

/// Момент времени (millis) последней смены выражения лица.
int lastChangedAt = 0;

/// Точка входа: создаёт лицо, задаёт нейтральное выражение и инициализирует генератор случайных чисел.
void setup() {
  face = new Face(SCREEN_WIDTH, SCREEN_HEIGHT, EYE_SIZE);
  face->Expression.GoTo_Normal();

  // Отключаем автоматическое поведение библиотеки, чтобы анимацией управлял только скетч.
  face->RandomBehavior = false;
  face->RandomBlink = false;
  face->RandomLook = false;

  // Неподключённый вывод A0 даёт случайный шум — используем его как зерно генератора.
  randomSeed(analogRead(A0));
}

/// Главный цикл прошивки: по таймеру меняет выражение лица, а затем отрисовывает каждый кадр.
void loop() {
  int now = millis();
  if (now - lastChangedAt > FACE_CHANGE_INTERVAL_MILLIS) {
    face->DoBlink();
    face->Look.LookAt(randomFloat(-1.0, 1.0), randomFloat(-1.0, 1.0));
    goToEmotion(random(0, EMOTIONS_NUMBER));
    lastChangedAt = now;
  }
  face->Update();
}

/// Переключает лицо на выражение с заданным номером.
///
/// \param emotionNumber Номер выражения; значение вне 0..@ref EMOTIONS_NUMBER-1 оставляет текущее выражение без
///                      изменений. Соответствие номера и вызова @ref FaceExpression приведено в таблице ветвей switch.
void goToEmotion(int emotionNumber) {
  switch (emotionNumber) {
      case 0:
        face->Expression.GoTo_Normal();
        break;
      case 1:
        face->Expression.GoTo_Angry();
        break;
      case 2:
        face->Expression.GoTo_Glee();
        break;
      case 3:
        face->Expression.GoTo_Happy();
        break;
      case 4:
        face->Expression.GoTo_Sad();
        break;
      case 5:
        face->Expression.GoTo_Worried();
        break;
      case 6:
        face->Expression.GoTo_Focused();
        break;
      case 7:
        face->Expression.GoTo_Annoyed();
        break;
      case 8:
        face->Expression.GoTo_Surprised();
        break;
      case 9:
        face->Expression.GoTo_Skeptic();
        break;
      case 10:
        face->Expression.GoTo_Frustrated();
        break;
      case 11:
        face->Expression.GoTo_Unimpressed();
        break;
      case 12:
        face->Expression.GoTo_Sleepy();
        break;
      case 13:
        face->Expression.GoTo_Suspicious();
        break;
      case 14:
        face->Expression.GoTo_Squint();
        break;
      case 15:
        face->Expression.GoTo_Furious();
        break;
      case 16:
        face->Expression.GoTo_Scared();
        break;
      case 17:
        face->Expression.GoTo_Awe();
        break;
    }
}

/// Возвращает случайное число с плавающей точкой в заданном диапазоне.
///
/// \param minVal Нижняя граница диапазона (включительно).
/// \param maxVal Верхняя граница диапазона (включительно).
/// \return Случайное значение из диапазона [\p minVal, \p maxVal].
float randomFloat(float minVal, float maxVal) {
  long randInt = random(0, 1000001);
  float normalized = randInt / 1000000.0;
  return minVal + normalized * (maxVal - minVal);
}
