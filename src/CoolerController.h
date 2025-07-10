#pragma once
#include "TemperatureSensor.h"  // Подключение датчика температуры
#include "EEPROMStorage.h"     // Подключение хранилища EEPROM

// Структура настроек холодильника
struct CoolerSettings {
    float targetTemp = 4.0;    // Целевая температура по умолчанию
    float hysteresis = 0.5;    // Гистерезис по умолчанию
    uint16_t minInterval = 300; // Минимальный интервал между включениями (сек)
    uint8_t checksum = 0;      // Контрольная сумма для проверки целостности
};

class CoolerController {
private:
    TemperatureSensor* sensor;  // Указатель на датчик температуры
    uint8_t compressorPin;     // Пин управления компрессором
    CoolerSettings settings;   // Текущие настройки
    bool compressorState = false; // Состояние компрессора (вкл/выкл)
    unsigned long lastStartTime = 0; // Время последнего включения

    // Метод расчета контрольной суммы
    uint8_t calculateChecksum() {
        uint8_t sum = 0;
        // Преобразуем структуру в массив байт
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&settings);
        // Суммируем все байты, кроме последнего (checksum)
        for(size_t i = 0; i < sizeof(settings) - 1; i++) {
            sum += data[i];
        }
        return 255 - sum;  // Возвращаем дополнение до 255
    }

public:
    // Конструктор, принимающий датчик температуры и пин компрессора
    CoolerController(TemperatureSensor* s, uint8_t pin) 
        : sensor(s), compressorPin(pin) {
        pinMode(pin, OUTPUT);  // Настройка пина как выход
        digitalWrite(pin, LOW); // Выключение компрессора
    }

    // Основной метод обновления состояния
    void update() {
        float temp = sensor->getTemp();  // Получение текущей температуры
        Serial.print(temp);
        unsigned long now = millis();    // Текущее время

        // Условия включения компрессора:
        // Температура выше целевой + гистерезис И
        // Прошло достаточно времени с последнего включения
        if(temp > settings.targetTemp + settings.hysteresis && 
           now - lastStartTime > settings.minInterval * 1000UL) {
            startCompressor();
        }
        // Условие выключения - температура ниже целевой - гистерезис
        else if(temp < settings.targetTemp - settings.hysteresis) {
            stopCompressor();
        }
    }

    // Включение компрессора
    void startCompressor() {
        if(!compressorState) {  // Если еще не включен
            digitalWrite(compressorPin, HIGH);  // Включение
            compressorState = true;             // Установка флага
            lastStartTime = millis();           // Запись времени включения
        }
    }

    // Выключение компрессора
    void stopCompressor() {
        digitalWrite(compressorPin, LOW);  // Выключение
        compressorState = false;           // Сброс флага
    }

    // Загрузка настроек из EEPROM
    bool loadSettings() {
        if(EEPROMStorage::read(0, settings)) {  // Чтение по адресу 0
            return settings.checksum == calculateChecksum(); // Проверка контрольной суммы
        }
        return false;
    }

    // Сохранение настроек в EEPROM
    bool saveSettings() {
        settings.checksum = calculateChecksum();  // Расчет новой контрольной суммы
        return EEPROMStorage::write(0, settings); // Запись по адресу 0
    }

    // Геттеры
    bool isRunning() const { return compressorState; }  // Состояние компрессора
    CoolerSettings& getSettings() { return settings; }   // Доступ к настройкам
};