#pragma once
#include <EEPROM.h>  // Подключение стандартной библиотеки EEPROM

class EEPROMStorage {
public:
    // Инициализация, проверка доступности EEPROM
    static bool init() {
        return EEPROM.length() >= 256;  // Проверка минимального размера
    }

    // Шаблонный метод чтения данных из EEPROM
    template<typename T>
    static bool read(int address, T& data) {
        // Проверка, что данные помещаются в EEPROM
        if(address + sizeof(T) > EEPROM.length()) {
            return false;
        }
        EEPROM.get(address, data);  // Чтение данных
        return true;
    }

    // Шаблонный метод записи данных в EEPROM
    template<typename T>
    static bool write(int address, const T& data) {
        // Проверка, что данные помещаются в EEPROM
        if(address + sizeof(T) > EEPROM.length()) {
            return false;
        }
        EEPROM.put(address, data);  // Запись данных
        return true;
    }

    // Очистка EEPROM
    static void clear() {
        for(size_t i = 0; i < EEPROM.length(); i++) {
            EEPROM.write(i, 0);  // Запись нулей во все ячейки
        }
    }
};