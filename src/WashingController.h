#pragma once
#include "EEPROMStorage.h"  // Подключение хранилища EEPROM

// Структура настроек мойки
struct WashingSettings {
    uint16_t stageTimes[5] = {60, 120, 60, 120, 60};  // Времена этапов (сек)
    uint8_t checksum = 0;  // Контрольная сумма
};

class WashingController {
private:
    // Пины управления компонентами мойки
    uint8_t drainValvePin;  // Клапан слива
    uint8_t coldWaterValvePin;  // Клапан холодной воды
    uint8_t hotWaterValvePin;  // Клапан горячей воды
    uint8_t washPumpPin;  // Насос мойки
    uint8_t alkaliPumpPin;  // Насос щелочи
    uint8_t acidPumpPin;  // Насос кислоты
    
    WashingSettings settings;  // Текущие настройки
    bool washingRunning = false;  // Флаг работы мойки
    uint8_t currentStage = 0;  // Текущий этап мойки
    unsigned long stageStartTime = 0;  // Время начала этапа

    // Названия этапов для отображения
    const char* stageNames[6] = {
        "IDLE", "COLD RINSE", "ALKALI WASH", 
        "INTERM. RINSE", "ACID WASH", "FINAL RINSE"
    };

    // Расчет контрольной суммы
    uint8_t calculateChecksum() {
        uint8_t sum = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += data[i];
        }
        return 255 - sum;
    }

    // Активация этапа мойки
    void activateStage(uint8_t stage) {
        // Выключение всех устройств
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);

        // Включение нужных устройств для этапа
        switch(stage) {
            case 1: // Холодное ополаскивание
                digitalWrite(drainValvePin, HIGH);  // Открыть слив
                delay(1000);  // Задержка для открытия
                digitalWrite(coldWaterValvePin, HIGH);  // Открыть холодную воду
                break;
            case 2: // Щелочная мойка
                digitalWrite(alkaliPumpPin, HIGH);  // Включить насос щелочи
                digitalWrite(washPumpPin, HIGH);  // Включить насос мойки
                break;
            case 3: // Промежуточное ополаскивание
                digitalWrite(hotWaterValvePin, HIGH);  // Открыть горячую воду
                break;
            case 4: // Кислотная мойка
                digitalWrite(acidPumpPin, HIGH);  // Включить насос кислоты
                digitalWrite(washPumpPin, HIGH);  // Включить насос мойки
                break;
            case 5: // Финальное ополаскивание
                digitalWrite(hotWaterValvePin, HIGH);  // Открыть горячую воду
                break;
        }
    }

public:
    // Конструктор с пинами управления
    WashingController(uint8_t drain, uint8_t cold, uint8_t hot,
                    uint8_t wash, uint8_t alkali, uint8_t acid)
        : drainValvePin(drain), coldWaterValvePin(cold), hotWaterValvePin(hot),
          washPumpPin(wash), alkaliPumpPin(alkali), acidPumpPin(acid) {
        
        // Настройка пинов как выходов
        pinMode(drainValvePin, OUTPUT);
        pinMode(coldWaterValvePin, OUTPUT);
        pinMode(hotWaterValvePin, OUTPUT);
        pinMode(washPumpPin, OUTPUT);
        pinMode(alkaliPumpPin, OUTPUT);
        pinMode(acidPumpPin, OUTPUT);
        
        // Выключение всех устройств
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);
    }

    // Запуск мойки
    void startWashing() {
        if(!washingRunning) {  // Если мойка не запущена
            washingRunning = true;
            currentStage = 1;  // Начальный этап
            stageStartTime = millis();  // Запись времени начала
            activateStage(currentStage);  // Активация этапа
        }
    }

    // Обновление состояния мойки
    void update() {
        if(washingRunning) {
            // Проверка времени текущего этапа
            if(millis() - stageStartTime >= settings.stageTimes[currentStage-1] * 1000UL) {
                if(currentStage < 5) {  // Если есть следующий этап
                    currentStage++;
                    stageStartTime = millis();
                    activateStage(currentStage);  // Активация нового этапа
                } else {  // Последний этап завершен
                    finishWashing();  // Завершение мойки
                }
            }
        }
    }

    // Завершение мойки
    void finishWashing() {
        // Включение слива
        digitalWrite(drainValvePin, HIGH);
        // Выключение всех других устройств
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);
        
        // Задержка для слива
        delay(settings.stageTimes[0] * 1000UL);
        
        // Выключение слива и сброс флагов
        digitalWrite(drainValvePin, LOW);
        washingRunning = false;
        currentStage = 0;
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        // Чтение после настроек холодильника и мешалки
        if(EEPROMStorage::read(sizeof(CoolerSettings) + sizeof(MixerSettings), settings)) {
            return settings.checksum == calculateChecksum();
        }
        return false;
    }

    // Сохранение настроек в EEPROM
    bool saveSettings() {
        settings.checksum = calculateChecksum();
        // Запись после настроек холодильника и мешалки
        return EEPROMStorage::write(sizeof(CoolerSettings) + sizeof(MixerSettings), settings);
    }

    // Геттеры и вспомогательные методы
    bool isRunning() const { return washingRunning; }  // Состояние мойки
    const char* getStageName() const { return stageNames[currentStage]; }  // Название этапа
    uint16_t getTimeLeft() const {  // Оставшееся время этапа
        return washingRunning ? settings.stageTimes[currentStage-1] - (millis()-stageStartTime)/1000 : 0;
    }
    WashingSettings& getSettings() { return settings; }  // Доступ к настройкам
    
    // Получение названия этапа по индексу
    const char* getStageTimeName(uint8_t index) const {
        const char* names[5] = {
            "Cold rinse", "Alkali wash", 
            "Inter. rinse", "Acid wash", "Final rinse"
        };
        return (index < 5) ? names[index] : "Unknown";
    }
};