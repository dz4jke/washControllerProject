// MixerController.h
#pragma once
#include "EEPROMStorage.h"

// Упакованная структура настроек миксера
struct MixerSettings {
    uint8_t mode = 1;        // Режим работы (0-выкл, 1-авто, 2-таймер)
    uint16_t workTime = 60;  // Время работы (сек)
    uint16_t idleTime = 180; // Время простоя (сек)
    uint8_t checksum = 0;    // Контрольная сумма
} __attribute__((packed));

class MixerController {
private:
    const uint8_t mixerPin;  // Пин управления миксером
    MixerSettings settings;  // Текущие настройки
    bool mixerState;         // Состояние миксера
    unsigned long lastSwitchTime; // Время последнего переключения

    // Расчет контрольной суммы
    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* p = (const uint8_t*)&settings;
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += *p++;
        }
        return ~sum;
    }

public:
    // Конструктор
    MixerController(uint8_t pin) 
        : mixerPin(pin), mixerState(false), lastSwitchTime(0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    // Основной метод обновления состояния
    void update(bool compressorRunning) {
        unsigned long currentMillis = millis();
        unsigned long elapsed = currentMillis - lastSwitchTime;
        
        switch(settings.mode) {
            case 0: // Всегда выключен
                if(mixerState) stop();
                break;
                
            case 1: // Работает с компрессором
                if(compressorRunning != mixerState) {
                    compressorRunning ? start() : stop();
                }
                break;
                
            case 2: // Таймерный режим
                if(mixerState) {
                    if(elapsed > settings.workTime * 1000UL) {
                        stop();
                        lastSwitchTime = currentMillis;
                    }
                } 
                else if(elapsed > settings.idleTime * 1000UL) {
                    start();
                    lastSwitchTime = currentMillis;
                }
                break;
        }
    }

    // Включение миксера
    void start() {
        if(!mixerState) {
            digitalWrite(mixerPin, HIGH);
            mixerState = true;
        }
    }

    // Выключение миксера
    void stop() {
        if(mixerState) {
            digitalWrite(mixerPin, LOW);
            mixerState = false;
        }
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        constexpr int address = sizeof(CoolerSettings); // Сместить после CoolerSettings
        EEPROMStorage::read(address, settings);
        return settings.checksum == calculateChecksum();
    }

    // Сохранение настроек в EEPROM
    void saveSettings() {
        settings.checksum = calculateChecksum();
        constexpr int address = sizeof(CoolerSettings);
        EEPROMStorage::write(address, settings);
    }

    // Проверка состояния миксера
    bool isActive() const { 
        return mixerState; 
    }
    
    // Получение ссылки на настройки
    MixerSettings& getSettings() { 
        return settings; 
    }
};