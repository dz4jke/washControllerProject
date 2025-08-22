#pragma once
#include "EEPROMStorage.h"
#include <Arduino.h> // Добавлено для pinMode/digitalWrite

/*
 * Структура настроек мойки
 */
struct WashingSettings {
    uint16_t stageTimes[5] = {60, 120, 60, 120, 60}; // Времена этапов (сек)
    uint8_t checksum = 0; // Контрольная сумма
} __attribute__((packed));

/*
 * Класс для управления системой мойки
 * Реализует:
 * - Автоматическую многоэтапную мойку
 * - Управление клапанами и насосами
 * - Сохранение настроек в EEPROM
 * - Ручное управление компонентами
 */
class WashingController {
private:
    // Пины управления
    const uint8_t drainValvePin;
    const uint8_t coldWaterValvePin;
    const uint8_t hotWaterValvePin;
    const uint8_t washPumpPin;
    const uint8_t alkaliPumpPin;
    const uint8_t acidPumpPin;
    
    WashingSettings settings;       // Текущие настройки
    volatile bool washingRunning;   // Флаг работы мойки (volatile для прерываний)
    uint8_t currentStage;           // Текущий этап (0 - не активен)
    unsigned long stageStartTime;   // Время начала этапа

    // Названия этапов в PROGMEM
    static const char* const stageNames[6] PROGMEM;

    /*
     * Расчет контрольной суммы
     */
    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += data[i];
        }
        return ~sum; // Инвертированная сумма
    }

    /*
     * Активация этапа мойки
     * stage - номер этапа (1-5)
     */
    void activateStage(uint8_t stage) {
        // Выключение всех устройств перед активацией нового этапа
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);

        // Включение устройств согласно этапу
        switch(stage) {
            case 1: // Холодное ополаскивание
                digitalWrite(drainValvePin, HIGH);
                digitalWrite(coldWaterValvePin, HIGH);
                break;
                
            case 2: // Щелочная мойка
                digitalWrite(alkaliPumpPin, HIGH);
                digitalWrite(washPumpPin, HIGH);
                break;
                
            case 3: // Промежуточное ополаскивание
                digitalWrite(drainValvePin, HIGH); // Обычно слив открыт на ополаскивании
                digitalWrite(hotWaterValvePin, HIGH);
                break;
                
            case 4: // Кислотная мойка
                digitalWrite(acidPumpPin, HIGH);
                digitalWrite(washPumpPin, HIGH);
                break;
                
            case 5: // Финальное ополаскивание
                digitalWrite(drainValvePin, HIGH); // Обычно слив открыт на ополаскивании
                digitalWrite(hotWaterValvePin, HIGH);
                break;
        }
    }

public:
    /*
     * Конструктор
     * Параметры - пины управления устройствами
     */
    WashingController(uint8_t drain, uint8_t cold, uint8_t hot,
                      uint8_t wash, uint8_t alkali, uint8_t acid)
        : drainValvePin(drain), coldWaterValvePin(cold), hotWaterValvePin(hot),
          washPumpPin(wash), alkaliPumpPin(alkali), acidPumpPin(acid),
          washingRunning(false), currentStage(0), stageStartTime(0)
    {
        // Настройка пинов как выходов
        pinMode(drainValvePin, OUTPUT);
        pinMode(coldWaterValvePin, OUTPUT);
        pinMode(hotWaterValvePin, OUTPUT);
        pinMode(washPumpPin, OUTPUT);
        pinMode(alkaliPumpPin, OUTPUT);
        pinMode(acidPumpPin, OUTPUT);
        
        // Выключение всех устройств по умолчанию
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);
    }

    /*
     * Основной метод обновления состояния
     * Должен вызываться в главном цикле программы
     */
    void update() {
        if(!washingRunning) return;
        
        // Проверка завершения текущего этапа
        if((millis() - stageStartTime) > (uint32_t)settings.stageTimes[currentStage-1] * 1000UL) {
            nextStage();
        }
    }

    /*
     * Запуск мойки
     */
    void startWashing() {
        if(washingRunning) return; // Если мойка уже запущена, ничего не делаем
        
        washingRunning = true;
        currentStage = 1;
        stageStartTime = millis();
        activateStage(currentStage);
    }

    /*
     * Переход к следующему этапу
     */
    void nextStage() {
        if(++currentStage > 5) {
            // Завершение мойки после 5 этапа
            stopWashing();
            return;
        }
        
        stageStartTime = millis();
        activateStage(currentStage);
    }

    /*
     * Остановка мойки
     */
    void stopWashing() {
        washingRunning = false;
        currentStage = 0;
        // Выключение всех устройств
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);
    }

    /*
     * Загрузка настроек из EEPROM
     * Возвращает true, если настройки загружены успешно
     */
    bool loadSettings() {
        // Адрес после настроек компрессора и миксера
        constexpr int address = sizeof(CoolerSettings) + sizeof(MixerSettings); 
        EEPROMStorage::read(address, settings);
        // Проверка контрольной суммы, если не совпадает, используем дефолтные
        if(settings.checksum != calculateChecksum()) {
            settings = WashingSettings(); // Сброс к настройкам по умолчанию
            return false;
        }
        return true;
    }

    /*
     * Сохранение настроек в EEPROM
     */
    void saveSettings() {
        settings.checksum = calculateChecksum();
        constexpr int address = sizeof(CoolerSettings) + sizeof(MixerSettings);
        EEPROMStorage::write(address, settings);
    }

    /*
     * Проверка работы мойки
     * Возвращает true, если мойка активна
     */
    bool isRunning() const { 
        return washingRunning; 
    }
    
    /*
     * Получение текущего этапа
     * Возвращает номер этапа (1-5) или 0 если мойка не активна
     */
    uint8_t getCurrentStage() const { 
        return currentStage; 
    }
    
    /*
     * Получение названия текущего этапа
     * Возвращает строку с названием этапа
     */
    const char* getStageName() const {
        static char buffer[16]; // Статический буфер для возвращаемой строки
        if (currentStage > 0 && currentStage <= 5) {
            strcpy_P(buffer, (const char*)pgm_read_word(&(stageNames[currentStage])));
        } else {
            strcpy_P(buffer, (const char*)pgm_read_word(&(stageNames[0]))); // "IDLE"
        }
        return buffer;
    }
    
    /*
     * Получение оставшегося времени этапа
     * Возвращает время в секундах
     */
    int getTimeLeft() const {
        if(!washingRunning || currentStage == 0) return 0;
        unsigned long elapsedStageTime = (millis() - stageStartTime) / 1000UL;
        int timeLeft = settings.stageTimes[currentStage-1] - elapsedStageTime;
        return (timeLeft > 0) ? timeLeft : 0; // Возвращаем 0, если время уже вышло
    }
    
    /*
     * Получение ссылки на настройки
     */
    WashingSettings& getSettings() { 
        return settings; 
    }
    
    // Методы для ручного управления компонентами (для тестирования)
    void setDrainValve(bool state) { digitalWrite(drainValvePin, state); }
    void setColdWaterValve(bool state) { digitalWrite(coldWaterValvePin, state); }
    void setHotWaterValve(bool state) { digitalWrite(hotWaterValvePin, state); }
    void setWashPump(bool state) { digitalWrite(washPumpPin, state); }
    void setAlkaliPump(bool state) { digitalWrite(alkaliPumpPin, state); }
    void setAcidPump(bool state) { digitalWrite(acidPumpPin, state); }
};

// Инициализация названий этапов в PROGMEM
const char* const WashingController::stageNames[6] PROGMEM = {
    "IDLE",               // 0 - не активно
    "COLD RINSE",         // 1 - холодное ополаскивание
    "ALKALI WASH",        // 2 - щелочная мойка
    "INTERM. RINSE",      // 3 - промежуточное ополаскивание
    "ACID WASH",          // 4 - кислотная мойка
    "FINAL RINSE"         // 5 - финальное ополаскивание
};
