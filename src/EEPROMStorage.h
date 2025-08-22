#pragma once
#include <EEPROM.h>
#include <Arduino.h>

/*
 * Класс для работы с EEPROM
 * Реализует:
 * - Чтение/запись любых типов данных
 * - Очистку EEPROM
 * - Оптимизированную запись (только при изменении)
 */
class EEPROMStorage
{
public:
    /*
     * Чтение данных из EEPROM
     * address - начальный адрес
     * data - ссылка на объект для чтения
     */
    template <typename T>
    static void read(int address, T &data)
    {
        EEPROM.get(address, data);
    }

    /*
     * Запись данных в EEPROM (только если изменились)
     * address - начальный адрес
     * data - ссылка на объект для записи
     */
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

    /*
     * Очистка EEPROM (запись 0, только в ячейках с данными)
     */
    static void clear()
    {
        for (size_t i = 0; i < EEPROM.length(); ++i)
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