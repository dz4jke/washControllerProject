#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>
#include <GyverButton.h>
#include <avr/wdt.h>
#include "Display.h"
#include "TemperatureSensor.h"
#include "ButtonMenuHandler.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"
#include "EEPROMStorage.h"

// Пины подключения
#define TEMP_SENSOR_PIN A0 // Датчик температуры
#define COMPRESSOR_PIN 8   // Пин компрессора
#define MIXER_PIN 7        // Пин миксера
#define WASH_BUTTON_PIN 2  // Кнопка мойки (прерывание)
#define UP_BUTTON_PIN 3    // Кнопка "Вверх"
#define DOWN_BUTTON_PIN 4  // Кнопка "Вниз"
#define SET_BUTTON_PIN 5   // Кнопка "Выбор"
#define ESC_BUTTON_PIN 6   // Кнопка "Отмена"
// Пины мойки
#define DRAIN_VALVE_PIN 9
#define COLD_WATER_VALVE_PIN 10
#define HOT_WATER_VALVE_PIN 11
#define WASH_PUMP_PIN 12
#define ALKALI_PUMP_PIN 13
#define ACID_PUMP_PIN A1 // Используем аналоговый пин как цифровой

// Константы
#define DISPLAY_UPDATE_INTERVAL 500 // Интервал обновления дисплея (мс)

// Глобальные объекты
LiquidCrystal_I2C lcd(0x27, 16, 2); // Дисплей I2C адрес 0x27, 16x2
Display display(&lcd);              // Управление дисплеем
TemperatureSensor tempSensor(       // Датчик температуры
    TEMP_SENSOR_PIN, 10000, 3950);
CoolerController cooler(&tempSensor, COMPRESSOR_PIN); // Охлаждение
MixerController mixer(MIXER_PIN);                     // Миксер
WashingController washer(                             // Мойка
    DRAIN_VALVE_PIN, COLD_WATER_VALVE_PIN, HOT_WATER_VALVE_PIN,
    WASH_PUMP_PIN, ALKALI_PUMP_PIN, ACID_PUMP_PIN);
ButtonMenuHandler buttons( // Кнопки и меню
    UP_BUTTON_PIN, DOWN_BUTTON_PIN, SET_BUTTON_PIN, ESC_BUTTON_PIN,
    &display, &cooler, &mixer, &washer);

// Переменные состояния
unsigned long lastDisplayUpdate = 0; // Время последнего обновления дисплея

// Прототип функции прерывания (должен быть объявлен до использования)
void washButtonISR();

void setup()
{
     wdt_disable(); // Временно отключаем watchdog

     // Инициализация последовательного порта
     Serial.begin(115200);
     Serial.println("System starting...");

     // Инициализация дисплея
     lcd.init();
     lcd.backlight();
     display.showMessage("Initializing...");
     delay(2000);

     // Инициализация EEPROM
     if (!EEPROMStorage::init())
     {
          display.showMessage("EEPROM Error!");
          delay(2000);
     }

     // Загрузка настроек
     if (!cooler.loadSettings() || !mixer.loadSettings() || !washer.loadSettings())
     {
          display.showMessage("Load Settings Err");
          delay(2000);
     }

     // Настройка прерывания для кнопки мойки
     pinMode(WASH_BUTTON_PIN, INPUT_PULLUP); // Важно настроить пин перед прерыванием
     attachInterrupt(digitalPinToInterrupt(WASH_BUTTON_PIN), washButtonISR, FALLING);

     // Включаем watchdog с таймаутом 2 секунды
     wdt_enable(WDTO_2S);

     // Отображаем сообщение о готовности
     display.showMainScreen(tempSensor.getTemp(), mixer.isActive(), cooler.isRunning());
}

// Реализация функции прерывания
void washButtonISR()
{
     if (!buttons.isMenuActive() && !washer.isRunning())
     {
          washer.startWashing(); // Запуск мойки, если система готова
     }
}

void loop()
{
     wdt_reset(); // Сбрасываем watchdog таймер

     // Обновление всех компонентов
     buttons.update();    // Опрос кнопок и меню
     tempSensor.update(); // Обновление датчика температуры

     // Если меню не активно - обновляем основные системы
     if (!buttons.isMenuActive())
     {
          cooler.update();                  // Управление охлаждением
          mixer.update(cooler.isRunning()); // Управление миксером

          if (washer.isRunning())
          {
               washer.update(); // Управление мойкой
          }
     }

     // Обновление дисплея с заданным интервалом
     if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)
     {
          if (washer.isRunning())
          {
               // Показываем статус мойки
               display.showWashingScreen(washer.getStageName(), washer.getTimeLeft());
          }
          else
          {
               // Показываем основной экран
               display.showMainScreen(
                   tempSensor.getTemp(),
                   mixer.isActive(),
                   cooler.isRunning());
          }
          lastDisplayUpdate = millis();
     }

     // Небольшая задержка для снижения нагрузки на процессор
     if (!washer.isRunning() && !buttons.isMenuActive())
     {
          delay(100);
     }
}