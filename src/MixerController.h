#pragma once
#include "EEPROMStorage.h"

/*
 * Структура настроек миксера
 */
struct MixerSettings {
    uint8_t mode = 1;        // Режим работы (0-выкл, 1-авто, 2-таймер)
    uint16_t workTime = 60;  // Время работы в таймерном режиме (сек)
    uint16_t idleTime = 180; // Время простоя в таймерном режиме (сек)
    uint8_t checksum = 0;    // Контрольная сумма
} __attribute__((packed));

/*
 * Класс для управления перемешивающим устройством
 * Реализует:
 * - Несколько режимов работы
 * - Таймерные функции
 * - Сохранение настроек в EEPROM
 */
class MixerController {
private:
    const uint8_t mixerPin;  // Пин управления миксером
    MixerSettings settings;  // Текущие настройки
    bool mixerState;         // Текущее состояние
    unsigned long lastSwitchTime; // Время последнего переключения

    /*
     * Расчет контрольной суммы
     */
    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&settings);
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += p[i];
        }
        return ~sum;
    }

public:
    /*
     * Конструктор
     * pin - пин управления миксером
     */
    MixerController(uint8_t pin) 
        : mixerPin(pin), mixerState(false), lastSwitchTime(0) 
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    /*
     * Основной метод обновления состояния
     * compressorRunning - состояние компрессора (для режима авто)
     */
    void update(bool compressorRunning) {
        unsigned long currentMillis = millis();
        unsigned long elapsed = currentMillis - lastSwitchTime;
        
        switch(settings.mode) {
            case 0: // Режим 0 - всегда выключен
                if(mixerState) stop();
                break;
                
            case 1: // Режим 1 - работает с компрессором
                if(compressorRunning != mixerState) {
                    compressorRunning ? start() : stop();
                }
                break;
                
            case 2: // Режим 2 - таймерный режим
                if(mixerState) {
                    // Проверка времени работы
                    if(elapsed > settings.workTime * 1000UL) {
                        stop();
                        lastSwitchTime = currentMillis;
                    }
                } else {
                    // Проверка времени простоя
                    if(elapsed > settings.idleTime * 1000UL) {
                        start();
                        lastSwitchTime = currentMillis;
                    }
                }
                break;
        }
    }

    /*
     * Включение миксера
     */
    void start() {
        if(!mixerState) {
            digitalWrite(mixerPin, HIGH);
            mixerState = true;
        }
    }

    /*
     * Выключение миксера
     */
    void stop() {
        if(mixerState) {
            digitalWrite(mixerPin, LOW);
            mixerState = false;
        }
    }

    /*
     * Установка состояния миксера (для тестирования)
     * state - true для включения, false для выключения
     */
    void setMixerState(bool state) {
        if(state) {
            start();
        } else {
            stop();
        }
    }

    /*
     * Загрузка настроек из EEPROM
     * Возвращает true, если настройки загружены успешно
     */
    bool loadSettings() {
        constexpr int address = sizeof(CoolerSettings); // Адрес после настроек компрессора
        EEPROMStorage::read(address, settings);
        return settings.checksum == calculateChecksum();
    }

    /*
     * Сохранение настроек в EEPROM
     */
    void saveSettings() {
        settings.checksum = calculateChecksum();
        constexpr int address = sizeof(CoolerSettings);
        EEPROMStorage::write(address, settings);
    }

    /*
     * Проверка состояния миксера
     * Возвращает true, если миксер включен
     */
    bool isActive() const { 
        return mixerState; 
    }
    
    /*
     * Получение ссылки на настройки
     */
    MixerSettings& getSettings() { 
        return settings; 
    }
};