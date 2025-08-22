#pragma once
#include "EEPROMStorage.h"
#include <Arduino.h>

/*
 * Структура настроек миксера
 */
struct MixerSettings {
    uint8_t mode = 1;         // Режим работы (0-выкл, 1-авто (с компрессором), 2-таймер)
    uint16_t workTime = 60;   // Время работы в таймерном режиме (сек)
    uint16_t idleTime = 180;  // Время простоя в таймерном режиме (сек)
    uint8_t checksum = 0;     // Контрольная сумма
} __attribute__((packed));

/*
 * Класс для управления перемешивающим устройством
 */
class MixerController {
private:
    const uint8_t mixerPin;
    MixerSettings settings;
    bool mixerState = false;
    unsigned long lastSwitchTime = 0; // Время последнего изменения состояния миксера

    /*
     * Расчет контрольной суммы
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
     * pin - пин управления миксером
     */
    MixerController(uint8_t pin) 
        : mixerPin(pin)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // Миксер выключен по умолчанию
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
                
            case 1: // Режим 1 - работает синхронно с компрессором
                if(compressorRunning != mixerState) { // Изменяем состояние, только если оно не совпадает
                    compressorRunning ? start() : stop();
                }
                break;
                
            case 2: // Режим 2 - таймерный режим
                if(mixerState) {
                    // Миксер включен, проверяем, не пора ли его выключить
                    if(elapsed > (unsigned long)settings.workTime * 1000UL) {
                        stop();
                    }
                } else {
                    // Миксер выключен, проверяем, не пора ли его включить
                    if(elapsed > (unsigned long)settings.idleTime * 1000UL) {
                        start();
                    }
                }
                break;
            default:
                if(mixerState) stop(); // Неизвестный режим - выключаем
                break;
        }
    }

    /*
     * Включение миксера
     */
    void start() {
        if (!mixerState) { // Включаем только если он выключен
            digitalWrite(mixerPin, HIGH);
            mixerState = true;
            lastSwitchTime = millis(); // Запоминаем время включения
        }
    }

    /*
     * Выключение миксера
     */
    void stop() {
        if (mixerState) { // Выключаем только если он включен
            digitalWrite(mixerPin, LOW);
            mixerState = false;
            lastSwitchTime = millis(); // Запоминаем время выключения
        }
    }

    /*
     * Установка состояния миксера (для тестирования)
     * state - true для включения, false для выключения
     */
    void setMixerState(bool state) {
        if (state) {
            start();
        } else {
            stop();
        }
    }

    /*
     * Загрузка настроек из EEPROM
     * Возвращает true, если настройки загружены успешно и контрольная сумма верна
     */
    bool loadSettings() {
        // Адрес начала настроек миксера (после настроек компрессора)
        constexpr int address = sizeof(CoolerSettings); 
        EEPROMStorage::read(address, settings);
        // Если контрольная сумма не совпадает, сбрасываем к настройкам по умолчанию
        if(settings.checksum != calculateChecksum()) {
            settings = MixerSettings(); // Инициализация дефолтными значениями
            return false;
        }
        return true;
    }

    /*
     * Сохранение настроек в EEPROM
     */
    void saveSettings() {
        settings.checksum = calculateChecksum(); // Обновляем контрольную сумму перед сохранением
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
