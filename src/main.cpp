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
#include "SafetySystem.h"

// Определение пинов подключения
#define TEMP_SENSOR_PIN A0
#define COMPRESSOR_PIN 8
#define MIXER_PIN 7
#define WASH_BUTTON_PIN 2
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
ButtonMenuHandler buttons(
    UP_BUTTON_PIN, DOWN_BUTTON_PIN, SET_BUTTON_PIN, ESC_BUTTON_PIN,
    display, cooler, mixer, washer);
SafetySystem safety;

// Переменные состояния
unsigned long lastDisplayUpdate = 0;

/*
 * Обработчик прерывания для кнопки мойки
 */
void washButtonISR()
{
    if (!buttons.isMenuActive() && !washer.isRunning())
    {
        washer.startWashing();
    }
}

/*
 * Функция setup - инициализация системы
 */
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

    // Загрузка настроек
    if (!cooler.loadSettings() || !mixer.loadSettings() || !washer.loadSettings())
    {
        display.showMessage("Load Settings Err");
        delay(2000);
    }

    // Настройка прерывания для кнопки мойки
    attachInterrupt(digitalPinToInterrupt(WASH_BUTTON_PIN), washButtonISR, FALLING);

    // Показываем статус системы
    display.showMessage("System Ready");
    delay(2000);
    display.showMainScreen(tempSensor.getTemp(), mixer.isActive(), cooler.isRunning());

    wdt_enable(WDTO_4S); // Включение watchdog с таймаутом 4 секунды
}

/*
 * Главный цикл программы
 */
void loop()
{
    wdt_reset(); // Сбрасываем watchdog timer

    // Обновление компонентов

    buttons.update();
    tempSensor.update();
    safety.checkActivity();

    // Основной режим работы (когда меню не активно)
    if (!buttons.isMenuActive())
    {
        cooler.update();
        mixer.update(cooler.isRunning());

        if (washer.isRunning())
        {
            washer.update();
        }

        // Обновление дисплея с заданным интервалом
        if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)
        {
            if (washer.isRunning())
            {
                display.showWashingScreen(washer.getStageName(), washer.getTimeLeft());
            }
            else
            {
                display.showMainScreen(
                    tempSensor.getTemp(),
                    mixer.isActive(),
                    cooler.isRunning());
            }
            lastDisplayUpdate = millis();
        }
    }

    // Небольшая задержка для снижения нагрузки
    if (!washer.isRunning() && !buttons.isMenuActive())
    {
        delay(100);
    }
}