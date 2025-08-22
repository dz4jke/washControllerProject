#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <GyverNTC.h>
#include <GyverButton.h>
#include <avr/wdt.h> // Для Watchdog Timer
#include "Display.h"
#include "TemperatureSensor.h"
#include "ButtonMenuHandler.h"
#include "CoolerController.h"
#include "MixerController.h"
#include "WashingController.h"
#include "EEPROMStorage.h"
#include "SafetySystem.h"

// Определение пинов подключения
#define TEMP_SENSOR_PIN A0
#define COMPRESSOR_PIN 8
#define MIXER_PIN 7
#define WASH_BUTTON_PIN 2 // Пин для кнопки запуска мойки
#define UP_BUTTON_PIN 3
#define DOWN_BUTTON_PIN 4
#define SET_BUTTON_PIN 6
#define ESC_BUTTON_PIN 5
#define DRAIN_VALVE_PIN 9
#define COLD_WATER_VALVE_PIN 10
#define HOT_WATER_VALVE_PIN 11
#define WASH_PUMP_PIN 12
#define ALKALI_PUMP_PIN 13
#define ACID_PUMP_PIN A1

// Константы
#define DISPLAY_UPDATE_INTERVAL 500 // Интервал обновления дисплея (мс)

// Глобальные объекты
LiquidCrystal_I2C lcd(0x27, 16, 2); // Адрес 0x27, 16 символов, 2 строки
TemperatureSensor tempSensor(TEMP_SENSOR_PIN);
CoolerController cooler(tempSensor, COMPRESSOR_PIN);
MixerController mixer(MIXER_PIN);
WashingController washer(
    DRAIN_VALVE_PIN, COLD_WATER_VALVE_PIN, HOT_WATER_VALVE_PIN,
    WASH_PUMP_PIN, ALKALI_PUMP_PIN, ACID_PUMP_PIN);
Display display(lcd);
// Передаем все необходимые контроллеры и датчик в ButtonMenuHandler
ButtonMenuHandler buttons(
    UP_BUTTON_PIN, DOWN_BUTTON_PIN, SET_BUTTON_PIN, ESC_BUTTON_PIN,
    display, cooler, mixer, washer, tempSensor);
SafetySystem safety;

// Переменные состояния
unsigned long lastDisplayUpdate = 0;

/*
 * Обработчик прерывания для кнопки мойки
 * Вызывается при низком уровне сигнала на WASH_BUTTON_PIN (FALLING)
 */
void washButtonISR() {
    // Запускаем мойку только если меню не активно и мойка в данный момент не работает
    if (!buttons.isMenuActive() && !washer.isRunning()) {
        washer.startWashing();
    }
}

/*
 * Функция setup - инициализация системы
 */
void setup() {
    //wdt_disable(); // Временно отключаем Watchdog Timer во время инициализации

    // Инициализация последовательного порта для отладки
    Serial.begin(115200);
    Serial.println("System starting...");

    // Инициализация дисплея
    lcd.init();
    lcd.backlight(); // Включаем подсветку
    display.showMessage("Initializing...");
    delay(1000); // Показываем сообщение на короткое время

    // Загрузка настроек из EEPROM для всех контроллеров
    // Если загрузка не удалась (например, из-за неверной контрольной суммы), 
    // контроллеры будут использовать дефолтные значения.
    if (!cooler.loadSettings() || !mixer.loadSettings() || !washer.loadSettings()) {
        display.showMessage("Load Settings Err");
        delay(2000);
    }

    // Настройка прерывания для кнопки мойки
    // Используем FALLING, если кнопка подключена к GND и имеет PULLUP-резистор
    attachInterrupt(digitalPinToInterrupt(WASH_BUTTON_PIN), washButtonISR, FALLING);

    // Показываем, что система готова
    display.showMessage("System Ready");
    delay(2000);

    // --- ВАЖНОЕ ИСПРАВЛЕНИЕ ЗДЕСЬ ---
    // Теперь вызываем showMainScreen() явно после того, как объект `buttons` полностью создан.
    buttons.showMainScreen(); 

   // wdt_enable(WDTO_4S); // Включаем Watchdog Timer с таймаутом 4 секунды
}

/*
 * Главный цикл программы
 */
void loop() {
    //wdt_reset(); // Сбрасываем Watchdog Timer, чтобы предотвратить перезагрузку

    // Обновление состояния всех компонентов
    buttons.update();    // Обработка кнопок и навигации по меню
    tempSensor.update(); // Обновление показаний датчика температуры
    //safety.updateActivity(); // Обновляем активность для SafetySystem (сброс таймера Watchdog)
    //safety.checkActivity(); // Проверяем активность системы

    // Основной режим работы (когда меню не активно)
    if (!buttons.isMenuActive()) {
        cooler.update(); // Обновление состояния контроллера охлаждения
        mixer.update(cooler.isRunning()); // Обновление состояния миксера (зависит от компрессора)

        if (washer.isRunning()) {
            washer.update(); // Обновление состояния контроллера мойки
        }

        // Обновление дисплея с заданным интервалом
        if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
            if (washer.isRunning()) {
                display.showWashingScreen(washer.getStageName(), washer.getTimeLeft());
            } else {
                buttons.showMainScreen(); // Обновляем главный экран
            }
            lastDisplayUpdate = millis();
        }
    }

    // Небольшая задержка для снижения нагрузки, если система не занята
    // Это предотвращает слишком быстрый цикл, что может быть полезно для экономии энергии
    // и стабильности, но избегаем длительных блокирующих задержек.
    // Если мойка или меню активны, задержка не нужна, т.к. система занята.
    if (!washer.isRunning() && !buttons.isMenuActive()) {
        delay(10); // Очень короткая неблокирующая задержка
    }
}