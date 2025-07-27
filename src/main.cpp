#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>
#include <GyverButton.h>
#include <avr/wdt.h>
#include "Display.h"
#include "TemperatureSensor.h"
#include "ButtonHandler.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"
#include "MenuSystem.h"
#include "SafetySystem.h"

#define WDT_TIMEOUT WDTO_2S // Таймаут watchdog (2 секунды)

// Создание объектов всех компонентов системы
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD с адресом 0x27, 16x2

SafetySystem safety;                                 // Система безопасности
Display display(&lcd);                               // Дисплей
TemperatureSensor tempSensor(A0, 10000, 3950, 25); // Датчик температуры
ButtonHandler buttons(2, 3, 4, 5);                   // Кнопки (пины 2-5)
// CoolerController cooler(&tempSensor, 6);  // Контроллер холодильника
MixerController mixer(7);                       // Контроллер мешалки
WashingController washer(8, 9, 10, 11, 12, 13); // Контроллер мойки
// MenuSystem menu(&display, &buttons, &cooler, &mixer, &washer);  // Меню

// Флаги и таймеры для обновления дисплея
volatile bool updateDisplayFlag = true;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // Интервал обновления (мс)

// Обработчик прерывания для кнопки мойки
void washButtonISR()
{
    // if(!menu.isActive() && !washer.isRunning()) {  // Если не в меню и мойка не запущена
    //    washer.startWashing();  // Запуск мойки
    // }
}

// Функция обновления дисплея
void updateDisplay()
{
    // if(washer.isRunning()) {  // Если мойка активна
    //    display.showWashingScreen(washer.getStageName(), washer.getTimeLeft());
    // } else {  // Основной экран
    //     display.showMainScreen("Cooling", tempSensor.getTemp(), mixer.isActive());
    //  }
}

void setup()
{
    // wdt_disable();  // Отключение watchdog на время инициализации
    Serial.begin(9600); // Инициализация Serial
    lcd.init();         // Инициализация LCD
    lcd.backlight();    // Включение подсветки

    // Проверка EEPROM
    // if(!EEPROM.begin()) {
    //    display.showMessage("EEPROM Error!");
    //     delay(2000);
    // }

    // Загрузка настроек
    // if(!cooler.loadSettings() || !mixer.loadSettings() || !washer.loadSettings()) {
    //  display.showMessage("Load Settings Err");
    //   delay(2000);
    // }

    // Настройка прерывания для кнопки мойки
    // attachInterrupt(digitalPinToInterrupt(5), washButtonISR, FALLING);
    // wdt_enable(WDT_TIMEOUT);  // Включение watchdogдщд
     display.showMainScreen("System Ready", tempSensor.getTemp(), mixer.isActive());
}

void loop()
{
    // wdt_reset();  // Сброс watchdog
    // buttons.update();  // Обновление состояния кнопок
     tempSensor.update();  // Обновление датчика температуры
     display.showMainScreen("System Ready", tempSensor.getTemp(), mixer.isActive());
    // safety.checkActivity();  // Проверка активности системы

    // if(menu.isActive()) {  // Если активно меню
    //   menu.update();  // Обновление меню
    // } else {  // Основной режим работы
    //    cooler.update();  // Управление холодильником
    //     mixer.update(cooler.isRunning());  // Управление мешалкой

    //    if(washer.isRunning()) {  // Если мойка активна
    //       washer.update();  // Обновление состояния мойки
    //   }
    // }

    // Обновление дисплея по таймеру
    // if(millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    //     updateDisplay();
    //     lastDisplayUpdate = millis();
    // }

    // Небольшая задержка, если не активна мойка и не меню
    // if(!washer.isRunning() && !menu.isActive()) {
    //     delay(100);
    //}
}