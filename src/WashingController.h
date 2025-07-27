#pragma once
#include "EEPROMStorage.h"

// Структура настроек мойки
struct WashingSettings {
    uint16_t stageTimes[5] = {60, 120, 60, 120, 60}; // Времена этапов (сек)
    uint8_t checksum = 0;                            // Контрольная сумма
};

/*
  Класс управления системой мойки
  Реализует:
  - Последовательное выполнение этапов мойки
  - Управление клапанами и насосами
  - Сохранение настроек в EEPROM
*/
class WashingController {
private:
    // Пины управления
    uint8_t _drainValvePin;
    uint8_t _coldWaterValvePin;
    uint8_t _hotWaterValvePin;
    uint8_t _washPumpPin;
    uint8_t _alkaliPumpPin;
    uint8_t _acidPumpPin;
    
    WashingSettings _settings;         // Текущие настройки
    bool _washingRunning = false;      // Флаг работы мойки
    uint8_t _currentStage = 0;        // Текущий этап (0-5)
    unsigned long _stageStartTime = 0; // Время начала этапа

    // Названия этапов
    const char* _stageNames[6] = {
        "IDLE", "COLD RINSE", "ALKALI WASH", 
        "INTERM. RINSE", "ACID WASH", "FINAL RINSE"
    };

    // Расчет контрольной суммы
    uint8_t _calculateChecksum() {
        uint8_t sum = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&_settings);
        for(size_t i = 0; i < sizeof(_settings) - 1; i++) {
            sum += data[i];
        }
        return 255 - sum;
    }

    // Активация конкретного этапа мойки
    void _activateStage(uint8_t stage) {
        // Сначала выключаем все устройства
        digitalWrite(_drainValvePin, LOW);
        digitalWrite(_coldWaterValvePin, LOW);
        digitalWrite(_hotWaterValvePin, LOW);
        digitalWrite(_washPumpPin, LOW);
        digitalWrite(_alkaliPumpPin, LOW);
        digitalWrite(_acidPumpPin, LOW);

        // Включаем нужные для текущего этапа
        switch(stage) {
            case 1: // Холодное ополаскивание
                digitalWrite(_drainValvePin, HIGH);
                delay(1000);  // Задержка для открытия клапана
                digitalWrite(_coldWaterValvePin, HIGH);
                break;
                
            case 2: // Щелочная мойка
                digitalWrite(_alkaliPumpPin, HIGH);
                digitalWrite(_washPumpPin, HIGH);
                break;
                
            case 3: // Промежуточное ополаскивание
                digitalWrite(_hotWaterValvePin, HIGH);
                break;
                
            case 4: // Кислотная мойка
                digitalWrite(_acidPumpPin, HIGH);
                digitalWrite(_washPumpPin, HIGH);
                break;
                
            case 5: // Финальное ополаскивание
                digitalWrite(_hotWaterValvePin, HIGH);
                break;
        }
    }

public:
    /*
      Конструктор: принимает все пины управления
      drain - клапан слива
      cold - клапан холодной воды
      hot - клапан горячей воды
      wash - основной насос мойки
      alkali - насос щелочи
      acid - насос кислоты
    */
    WashingController(uint8_t drain, uint8_t cold, uint8_t hot,
                    uint8_t wash, uint8_t alkali, uint8_t acid)
        : _drainValvePin(drain), _coldWaterValvePin(cold), _hotWaterValvePin(hot),
          _washPumpPin(wash), _alkaliPumpPin(alkali), _acidPumpPin(acid) {
        
        // Настройка всех пинов как выходов
        pinMode(_drainValvePin, OUTPUT);
        pinMode(_coldWaterValvePin, OUTPUT);
        pinMode(_hotWaterValvePin, OUTPUT);
        pinMode(_washPumpPin, OUTPUT);
        pinMode(_alkaliPumpPin, OUTPUT);
        pinMode(_acidPumpPin, OUTPUT);
        
        // Выключаем все устройства
        digitalWrite(_drainValvePin, LOW);
        digitalWrite(_coldWaterValvePin, LOW);
        digitalWrite(_hotWaterValvePin, LOW);
        digitalWrite(_washPumpPin, LOW);
        digitalWrite(_alkaliPumpPin, LOW);
        digitalWrite(_acidPumpPin, LOW);
    }

    // Обновление состояния (вызывать в loop)
    void update() {
        if(!_washingRunning) return;  // Если мойка не активна - выходим
        
        unsigned long now = millis();
        // Проверяем, не истекло ли время текущего этапа
        if(now - _stageStartTime > _settings.stageTimes[_currentStage-1] * 1000UL) {
            nextStage();  // Переходим к следующему этапу
        }
    }

    // Запуск мойки
    void startWashing() {
        if(_washingRunning) return;  // Если уже работает - выходим
        
        _washingRunning = true;      // Устанавливаем флаг
        _currentStage = 1;           // Начинаем с первого этапа
        _stageStartTime = millis();   // Запоминаем время начала
        _activateStage(_currentStage); // Активируем первый этап
    }

    // Переход к следующему этапу
    void nextStage() {
        _currentStage++;            // Увеличиваем номер этапа
        
        // Если прошли все этапы - завершаем мойку
        if(_currentStage > 5) {
            stopWashing();
            return;
        }
        
        _stageStartTime = millis();   // Сбрасываем таймер
        _activateStage(_currentStage); // Активируем новый этап
    }

    // Остановка мойки
    void stopWashing() {
        _washingRunning = false;     // Сбрасываем флаг
        _currentStage = 0;           // Сбрасываем этап
        _activateStage(0);           // Выключаем все устройства
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        // Читаем с учетом размеров предыдущих структур
        size_t offset = sizeof(CoolerSettings) + sizeof(MixerSettings);
        if(EEPROMStorage::read(offset, _settings)) {
            return _settings.checksum == _calculateChecksum();
        }
        return false;
    }

    // Сохранение настроек в EEPROM
    bool saveSettings() {
        _settings.checksum = _calculateChecksum();
        size_t offset = sizeof(CoolerSettings) + sizeof(MixerSettings);
        return EEPROMStorage::write(offset, _settings);
    }

    // ===== Геттеры =====
    
    bool isRunning() const { 
        return _washingRunning; 
    }
    
    uint8_t getCurrentStage() const { 
        return _currentStage; 
    }
    
    const char* getStageName() const { 
        return _stageNames[_currentStage]; 
    }
    
    int getTimeLeft() const {
        if(!_washingRunning) return 0;
        unsigned long elapsed = (millis() - _stageStartTime) / 1000;
        return _settings.stageTimes[_currentStage-1] - elapsed;
    }
    
    WashingSettings& getSettings() { 
        return _settings; 
    }
    
    // Получение имени этапа по индексу (для меню)
    const char* getStageTimeName(uint8_t index) const {
        const char* names[5] = {
            "Cold rinse", "Alkali wash", 
            "Interm. rinse", "Acid wash", "Final rinse"
        };
        return (index < 5) ? names[index] : "Unknown";
    }
};