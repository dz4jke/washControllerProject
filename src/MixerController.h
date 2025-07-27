#pragma once
#include "EEPROMStorage.h"

// Структура настроек миксера
struct MixerSettings {
    uint8_t mode = 1;          // Режим работы (0-выкл, 1-авто, 2-таймер)
    uint16_t workTime = 60;     // Время работы в режиме таймера (сек)
    uint16_t idleTime = 180;    // Время простоя в режиме таймера (сек)
    uint8_t checksum = 0;      // Контрольная сумма
};

/*
  Класс управления перемешивающим устройством
  Реализует:
  - Несколько режимов работы
  - Циклическую работу по таймеру
  - Сохранение настроек в EEPROM
*/
class MixerController {
private:
    uint8_t _mixerPin;          // Пин управления миксером
    MixerSettings _settings;    // Текущие настройки
    bool _mixerState = false;   // Текущее состояние
    unsigned long _lastSwitchTime = 0; // Время последнего переключения

    // Расчет контрольной суммы
    uint8_t _calculateChecksum() {
        uint8_t sum = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&_settings);
        for(size_t i = 0; i < sizeof(_settings) - 1; i++) {
            sum += data[i];
        }
        return 255 - sum;
    }

public:
    // Конструктор: принимает пин миксера
    MixerController(uint8_t pin) : _mixerPin(pin) {
        pinMode(pin, OUTPUT);      // Настраиваем пин как выход
        digitalWrite(pin, LOW);     // Выключаем миксер
    }

    /*
      Обновление состояния (вызывать в loop)
      compressorRunning - состояние компрессора (для режима 1)
    */
    void update(bool compressorRunning) {
        switch(_settings.mode) {
            case 0: // Режим 0: всегда выключен
                stop();
                break;
                
            case 1: // Режим 1: работает только при включенном компрессоре
                compressorRunning ? start() : stop();
                break;
                
            case 2: // Режим 2: работает по таймеру
                if(_mixerState) {
                    // Проверяем, не истекло ли время работы
                    if(millis() - _lastSwitchTime > _settings.workTime * 1000UL) {
                        stop();
                        _lastSwitchTime = millis();  // Сбрасываем таймер
                    }
                } else {
                    // Проверяем, не истекло ли время простоя
                    if(millis() - _lastSwitchTime > _settings.idleTime * 1000UL) {
                        start();
                        _lastSwitchTime = millis();  // Сбрасываем таймер
                    }
                }
                break;
        }
    }

    // Включение миксера
    void start() {
        digitalWrite(_mixerPin, HIGH);  // Включаем
        _mixerState = true;             // Обновляем состояние
    }

    // Выключение миксера
    void stop() {
        digitalWrite(_mixerPin, LOW);   // Выключаем
        _mixerState = false;            // Обновляем состояние
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        // Читаем с учетом размера предыдущей структуры (CoolerSettings)
        if(EEPROMStorage::read(sizeof(CoolerSettings), _settings)) {
            return _settings.checksum == _calculateChecksum();
        }
        return false;
    }

    // Сохранение настроек в EEPROM
    bool saveSettings() {
        _settings.checksum = _calculateChecksum();
        // Записываем с учетом размера предыдущей структуры
        return EEPROMStorage::write(sizeof(CoolerSettings), _settings);
    }

    // Получение текущего состояния миксера
    bool isActive() const { 
        return _mixerState; 
    }
    
    // Получение ссылки на настройки (для меню)
    MixerSettings& getSettings() { 
        return _settings; 
    }
};