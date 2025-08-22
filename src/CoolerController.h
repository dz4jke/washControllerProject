#pragma once
#include "TemperatureSensor.h"
#include "EEPROMStorage.h"

/*
 * Структура настроек компрессора
 * __attribute__((packed)) гарантирует плотную упаковку в памяти
 */
struct CoolerSettings {
    float targetTemp = 4.0f;    // Целевая температура (°C)
    float hysteresis = 2.0f;    // Гистерезис (°C)
    uint16_t minInterval = 300; // Минимальный интервал между включениями (сек)
    uint8_t checksum = 0;       // Контрольная сумма
} __attribute__((packed));

/*
 * Класс для управления компрессором охлаждения
 * Реализует:
 * - Поддержание заданной температуры
 * - Защиту от частых включений
 * - Сохранение настроек в EEPROM
 */
class CoolerController {
private:
    TemperatureSensor& sensor;  // Ссылка на датчик температуры
    const uint8_t compressorPin; // Пин управления компрессором
    CoolerSettings settings;    // Текущие настройки
    bool compressorState;       // Текущее состояние компрессора
    unsigned long lastStartTime; // Время последнего включения

    /*
     * Расчет контрольной суммы для проверки целостности настроек
     */
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
        : sensor(sensorRef), compressorPin(pin), 
          compressorState(false), lastStartTime(0) 
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    /*
     * Основной метод обновления состояния
     * Должен вызываться в главном цикле программы
     */
    void update() {
        float temp = sensor.getTemp();
        unsigned long now = millis();
        unsigned long minIntervalMs = settings.minInterval * 1000UL;

        // Условия включения компрессора:
        // 1. Температура выше целевой + гистерезис
        // 2. Прошло достаточно времени с последнего включения
        if(temp > settings.targetTemp + settings.hysteresis && 
           now - lastStartTime > minIntervalMs) {
            startCompressor();
        }
        // Условие выключения - температура ниже целевой - гистерезис
        else if(temp < settings.targetTemp - settings.hysteresis) {
            stopCompressor();
        }
    }

    /*
     * Включение компрессора
     */
    void startCompressor() {
        if(!compressorState) {
            digitalWrite(compressorPin, HIGH);
            compressorState = true;
            lastStartTime = millis();
        }
    }

    /*
     * Выключение компрессора
     */
    void stopCompressor() {
        if(compressorState) {
            digitalWrite(compressorPin, LOW);
            compressorState = false;
        }
    }

    /*
     * Установка состояния компрессора (для тестирования)
     * state - true для включения, false для выключения
     */
    void setCompressorState(bool state) {
        if(state) {
            startCompressor();
        } else {
            stopCompressor();
        }
    }

    /*
     * Загрузка настроек из EEPROM
     * Возвращает true, если настройки загружены успешно
     */
    bool loadSettings() {
        constexpr int address = 0; // Адрес в EEPROM
        EEPROMStorage::read(address, settings);
        
        // Проверка контрольной суммы
        if(settings.checksum != calculateChecksum()) {
            // Сброс к настройкам по умолчанию при ошибке
            settings = CoolerSettings();
            return false;
        }
        return true;
    }

    /*
     * Сохранение настроек в EEPROM
     */
    void saveSettings() {
        settings.checksum = calculateChecksum();
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