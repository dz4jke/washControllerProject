#pragma once
#include "TemperatureSensor.h"
#include "EEPROMStorage.h"
#include <Arduino.h>

/*
 * Структура настроек компрессора
 */
struct CoolerSettings {
    float targetTemp = 4.0f;     // Целевая температура (°C)
    float hysteresis = 2.0f;     // Гистерезис (°C)
    uint16_t minInterval = 300; // Минимальный интервал между включениями (сек)
    uint8_t checksum = 0;        // Контрольная сумма
} __attribute__((packed));

/*
 * Класс для управления компрессором охлаждения
 */
class CoolerController {
private:
    TemperatureSensor& sensor;
    const uint8_t compressorPin;
    CoolerSettings settings;
    bool compressorState = false;
    unsigned long lastStopTime = 0; // Время последнего выключения

    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&settings);
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += p[i];
        }
        return ~sum; // Инвертированная сумма
    }

public:
    /*
     * Конструктор
     * sensorRef - ссылка на датчик температуры
     * pin - пин управления компрессором
     */
    CoolerController(TemperatureSensor& sensorRef, uint8_t pin) 
        : sensor(sensorRef), compressorPin(pin) 
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // Компрессор выключен по умолчанию
    }

    /*
     * Основной метод обновления состояния
     * Должен вызываться в главном цикле программы
     */
    void update() {
        // Если датчик неисправен, выключаем компрессор
        if (!sensor.isSensorOK()) {
            stopCompressor();
            return;
        }

        float temp = sensor.getTemp();
        unsigned long now = millis();
        unsigned long minIntervalMs = (unsigned long)settings.minInterval * 1000UL;

        if (compressorState) {
            // Компрессор включен, проверяем условие для выключения
            if (temp < settings.targetTemp - settings.hysteresis) {
                stopCompressor();
            }
        } else {
            // Компрессор выключен, проверяем условие для включения
            if (temp > settings.targetTemp + settings.hysteresis) {
                // Проверяем, прошло ли достаточно времени с последнего выключения
                if (now - lastStopTime > minIntervalMs) {
                    startCompressor();
                }
            }
        }
    }

    /*
     * Включение компрессора
     */
    void startCompressor() {
        if (!compressorState) { // Включаем только если он выключен
            digitalWrite(compressorPin, HIGH);
            compressorState = true;
        }
    }

    /*
     * Выключение компрессора
     */
    void stopCompressor() {
        if (compressorState) { // Выключаем только если он включен
            digitalWrite(compressorPin, LOW);
            compressorState = false;
            lastStopTime = millis(); // Запоминаем время выключения
        }
    }

    /*
     * Установка состояния компрессора (для тестирования)
     * state - true для включения, false для выключения
     */
    void setCompressorState(bool state) {
        if (state) {
            startCompressor();
        } else {
            stopCompressor();
        }
    }

    /*
     * Загрузка настроек из EEPROM
     * Возвращает true, если настройки загружены успешно и контрольная сумма верна
     */
    bool loadSettings() {
        constexpr int address = 0; // Адрес начала настроек компрессора в EEPROM
        EEPROMStorage::read(address, settings);
        // Если контрольная сумма не совпадает, сбрасываем к настройкам по умолчанию
        if(settings.checksum != calculateChecksum()) {
            settings = CoolerSettings(); // Инициализация дефолтными значениями
            return false;
        }
        return true;
    }

    /*
     * Сохранение настроек в EEPROM
     */
    void saveSettings() {
        settings.checksum = calculateChecksum(); // Обновляем контрольную сумму перед сохранением
        constexpr int address = 0;
        EEPROMStorage::write(address, settings);
    }

    /*
     * Проверка состояния компрессора
     * Возвращает true, если компрессор включен
     */
    bool isRunning() const { 
        return compressorState; 
    }
    
    /*
     * Получение ссылки на настройки
     */
    CoolerSettings& getSettings() { 
        return settings; 
    }
};
