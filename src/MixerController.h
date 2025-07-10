#pragma once
#include "EEPROMStorage.h"  // Подключение хранилища EEPROM

// Структура настроек мешалки
struct MixerSettings {
    uint8_t mode = 1;  // Режим работы (0-выкл, 1-с компрессором, 2-таймер)
    uint16_t workTime = 60;  // Время работы в режиме таймера (сек)
    uint16_t idleTime = 180;  // Время простоя в режиме таймера (сек)
    uint8_t checksum = 0;  // Контрольная сумма
};

class MixerController {
private:
    uint8_t mixerPin;  // Пин управления мешалкой
    MixerSettings settings;  // Текущие настройки
    bool mixerState = false;  // Состояние мешалки
    unsigned long lastSwitchTime = 0;  // Время последнего переключения

    // Расчет контрольной суммы
    uint8_t calculateChecksum() {
        uint8_t sum = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += data[i];
        }
        return 255 - sum;  // Дополнение до 255
    }

public:
    // Конструктор
    MixerController(uint8_t pin) : mixerPin(pin) {
        pinMode(pin, OUTPUT);  // Настройка пина как выход
        digitalWrite(pin, LOW);  // Выключение мешалки
    }

    // Основной метод обновления
    void update(bool compressorRunning) {
        switch(settings.mode) {
            case 0: stop(); break;  // Режим 0 - всегда выключена
            case 1:  // Режим 1 - работает с компрессором
                compressorRunning ? start() : stop();
                break;
            case 2:  // Режим 2 - работа по таймеру
                if(mixerState) {
                    // Проверка времени работы
                    if(millis() - lastSwitchTime > settings.workTime * 1000UL) {
                        stop();
                        lastSwitchTime = millis();
                    }
                } else {
                    // Проверка времени простоя
                    if(millis() - lastSwitchTime > settings.idleTime * 1000UL) {
                        start();
                        lastSwitchTime = millis();
                    }
                }
                break;
        }
    }

    // Включение мешалки
    void start() {
        digitalWrite(mixerPin, HIGH);
        mixerState = true;
    }

    // Выключение мешалки
    void stop() {
        digitalWrite(mixerPin, LOW);
        mixerState = false;
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        // Чтение с адреса, следующего за настройками холодильника
        if(EEPROMStorage::read(sizeof(CoolerSettings), settings)) {
            return settings.checksum == calculateChecksum();
        }
        return false;
    }

    // Сохранение настроек в EEPROM
    bool saveSettings() {
        settings.checksum = calculateChecksum();
        // Запись с адреса, следующего за настройками холодильника
        return EEPROMStorage::write(sizeof(CoolerSettings), settings);
    }

    // Геттеры
    bool isActive() const { return mixerState; }  // Состояние мешалки
    MixerSettings& getSettings() { return settings; }  // Доступ к настройкам
};