// WashingController.h
#pragma once
#include "EEPROMStorage.h"

// Упакованная структура настроек мойки
struct WashingSettings {
    uint16_t stageTimes[5] = {60, 120, 60, 120, 60}; // Времена этапов (сек)
    uint8_t checksum = 0; // Контрольная сумма
} __attribute__((packed));

class WashingController {
private:
    // Пины управления (используем uint8_t для экономии памяти)
    const uint8_t drainValvePin;
    const uint8_t coldWaterValvePin;
    const uint8_t hotWaterValvePin;
    const uint8_t washPumpPin;
    const uint8_t alkaliPumpPin;
    const uint8_t acidPumpPin;
    
    WashingSettings settings;       // Текущие настройки
    volatile bool washingRunning;   // Флаг работы мойки
    uint8_t currentStage;           // Текущий этап
    uint32_t stageStartTime;        // Время начала этапа

    // Названия этапов в PROGMEM
    static const char* const stageNames[6] PROGMEM;

    // Расчет контрольной суммы
    uint8_t calculateChecksum() const {
        uint8_t sum = 0;
        const uint8_t* data = (const uint8_t*)&settings;
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += data[i];
        }
        return ~sum;
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

        switch(stage) {
            case 1: // Холодное ополаскивание
                digitalWrite(drainValvePin, HIGH);
                delay(50);
                digitalWrite(coldWaterValvePin, HIGH);
                break;
                
            case 2: // Щелочная мойка
                digitalWrite(alkaliPumpPin, HIGH);
                digitalWrite(washPumpPin, HIGH);
                break;
                
            case 3: // Промежуточное ополаскивание
                digitalWrite(hotWaterValvePin, HIGH);
                break;
                
            case 4: // Кислотная мойка
                digitalWrite(acidPumpPin, HIGH);
                digitalWrite(washPumpPin, HIGH);
                break;
                
            case 5: // Финальное ополаскивание
                digitalWrite(hotWaterValvePin, HIGH);
                break;
        }
    }

public:
    // Конструктор
    WashingController(
        uint8_t drain, uint8_t cold, uint8_t hot,
        uint8_t wash, uint8_t alkali, uint8_t acid
    ) : 
        drainValvePin(drain), coldWaterValvePin(cold), hotWaterValvePin(hot),
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
        
        // Выключение всех устройств
        digitalWrite(drainValvePin, LOW);
        digitalWrite(coldWaterValvePin, LOW);
        digitalWrite(hotWaterValvePin, LOW);
        digitalWrite(washPumpPin, LOW);
        digitalWrite(alkaliPumpPin, LOW);
        digitalWrite(acidPumpPin, LOW);
    }

    // Основной метод обновления состояния
    void update() {
        if(!washingRunning) return;
        
        uint32_t now = millis();
        // Проверка завершения этапа
        if((now - stageStartTime) > (uint32_t)settings.stageTimes[currentStage-1] * 1000UL) {
            nextStage();
        }
    }

    // Запуск мойки
    void startWashing() {
        if(washingRunning) return;
        
        washingRunning = true;
        currentStage = 1;
        stageStartTime = millis();
        activateStage(currentStage);
    }

    // Переход к следующему этапу
    void nextStage() {
        if(++currentStage > 5) {
            stopWashing();
            return;
        }
        
        stageStartTime = millis();
        activateStage(currentStage);
    }

    // Остановка мойки
    void stopWashing() {
        washingRunning = false;
        currentStage = 0;
        activateStage(0);
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        constexpr int address = sizeof(CoolerSettings) + sizeof(MixerSettings); // Сместить после CoolerSettings
        EEPROMStorage::read(address, settings);
        return settings.checksum == calculateChecksum();
    }

    // Сохранение настроек в EEPROM
    void saveSettings() {
        settings.checksum = calculateChecksum();
        constexpr int address = sizeof(CoolerSettings) + sizeof(MixerSettings);
        EEPROMStorage::write(address, settings);
    }

    // Проверка работы мойки
    bool isRunning() const { return washingRunning; }
    
    // Получение текущего этапа
    uint8_t getCurrentStage() const { return currentStage; }
    
    // Получение названия текущего этапа
    const char* getStageName() const {
        static char buffer[16];
        strcpy_P(buffer, (const char*)pgm_read_word(&(stageNames[currentStage])));
        return buffer;
    }
    
    // Получение оставшегося времени этапа
    int getTimeLeft() const {
        if(!washingRunning) return 0;
        return settings.stageTimes[currentStage-1] - ((millis() - stageStartTime) / 1000);
    }
    
    // Получение ссылки на настройки
    WashingSettings& getSettings() { return settings; }
};

// Инициализация PROGMEM строк
const char* const WashingController::stageNames[6] PROGMEM = {
    "IDLE", "COLD RINSE", "ALKALI WASH", 
    "INTERM. RINSE", "ACID WASH", "FINAL RINSE"
};