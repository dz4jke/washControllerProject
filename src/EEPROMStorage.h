// EEPROMStorage.h
#pragma once
#include <EEPROM.h>
#include <Arduino.h>

class EEPROMStorage
{
public:
    // Инициализация (для Arduino Nano не требуется, оставлена для совместимости)

    // Шаблонная функция чтения данных из EEPROM
    template <typename T>
    static void read(int address, T &data)
    {
        EEPROM.get(address, data);
    }

    // Шаблонная функция записи данных в EEPROM (только если отличается)
    template <typename T>
    static void write(int address, const T &data)
    {
        T current;
        EEPROM.get(address, current);
        if (memcmp(&current, &data, sizeof(T)) != 0)
        {
            EEPROM.put(address, data);
        }
    }

    // Очистка EEPROM (только если не нули, чтобы не расходовать ресурс)
    static void clear()
    {
        for (int i = 0; i < EEPROM.length(); ++i)
        {
            if (EEPROM.read(i) != 0)
            {
                EEPROM.write(i, 0);
            }
        }
    }

private:
    EEPROMStorage() = delete; // Запрет создания экземпляров
};
