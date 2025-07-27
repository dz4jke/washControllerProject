// CoolerController.h
#pragma once
#include "TemperatureSensor.h"
#include "EEPROMStorage.h"

// Упакованная структура настроек охлаждения
struct CoolerSettings {
         // Контрольная сумма
    float targetTemp = 4.0f;    // Целевая температура (°C)
    float hysteresis = 2.0f;    // Гистерезис (°C)
    uint16_t minInterval = 300; // Минимальный интервал между включениями (сек)
    uint8_t checksum = 0; 
    
} __attribute__((packed));

class CoolerController {
private:
    TemperatureSensor& sensor;  // Ссылка на датчик температуры
    const uint8_t compressorPin; // Пин управления компрессором
    CoolerSettings settings;    // Текущие настройки
    bool compressorState;       // Состояние компрессора
    unsigned long lastStartTime; // Время последнего включения

    // Расчет контрольной суммы
    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* p = (const uint8_t*)&settings;
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += pgm_read_byte(p++);
        }
        return ~sum; // Инвертированная сумма
    }

public:
    // Конструктор
    CoolerController(TemperatureSensor& sensorRef, uint8_t pin) 
        : sensor(sensorRef), compressorPin(pin), 
          compressorState(false), lastStartTime(0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    // Основной метод обновления состояния
    void update() {
        float temp = sensor.getTemp();
        unsigned long now = millis();
        unsigned long minIntervalMs = settings.minInterval * 1000UL;

        // Логика управления компрессором
        if(temp > settings.targetTemp + settings.hysteresis && 
           now - lastStartTime > minIntervalMs) {
            startCompressor();
        }
        else if(temp < settings.targetTemp - settings.hysteresis) {
            stopCompressor();
        }
    }

    // Включение компрессора
    void startCompressor() {
        if(!compressorState) {
            digitalWrite(compressorPin, HIGH);
            compressorState = true;
            lastStartTime = millis();
        }
    }

    // Выключение компрессора
    void stopCompressor() {
        digitalWrite(compressorPin, LOW);
        compressorState = false;
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        constexpr int address = 0; // Сместить после CoolerSettings
        EEPROMStorage::read(address, settings);
        return settings.checksum == calculateChecksum();
    }

    // Сохранение настроек в EEPROM
    void saveSettings() {
        settings.checksum = calculateChecksum();
         constexpr int address = 0;
        EEPROMStorage::write(address, settings);
    }

    // Проверка состояния компрессора
    bool isRunning() const { 
        return compressorState; 
    }
    
    // Получение ссылки на настройки
    CoolerSettings& getSettings() { 
        return settings; 
    }
};